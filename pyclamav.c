/*
 *  Copyright (C) 2005 Alexandre Norman <norman@xael.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/* Tested with python 2.4 and clamav 0.86.1
 * Should work with any version of python from 2.1
 */

#include <Python.h>
#include <clamav.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#define PYCLAMAV_VERSION "0.4.1"


/* ********************************************************* */

/* To be able to compile with 
   releases 0.75 of libclamav 

   Where cl_free was cl_freetrie
   and cl_build was cl_buildtrie
   CL_SCAN_STDOPT did not exist
*/
#ifndef CL_SCAN_STDOPT
#define CL_SCAN_STDOPT CL_RAW | CL_ARCHIVE | CL_MAIL | CL_DISABLERAR | CL_OLE2 | CL_ENCRYPTED
void cl_free(struct cl_node *rootnode) {
  cl_freetrie(rootnode);  
  return;    
} 

int cl_build(struct cl_node *rootnode) {  
  return cl_buildtrie(rootnode);    
} 
#endif

/* For python prior to 2.3 */
#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

/* ********************************************************* */


/* Exception object for python */
static PyObject *PyclamavError;


/* Signature number */
unsigned int signumber = 0;
 
/* Structures for clamav */
struct cl_node *root = NULL;
struct cl_limits limits;
struct cl_stat dbstat;


/*
 * If the virus database has been changed, then
 * free the current tree and reload the new one
 */
int if_database_have_changed_then_reload(void)
{
  int ret = 0;
 
  /* Test if the database have changed */
  /* If yes : reload DB                */
  if (cl_statchkdir(&dbstat) == 1)
    {
      /* free the tree */
      cl_free(root); 
      signumber=0;
      root=NULL;

      /* Load DB */
      if((ret = cl_load(cl_retdbdir(), &root, &signumber, CL_DB_STDOPT))) {
	/* Raise exception with error message */
	PyErr_SetString(PyclamavError,  cl_strerror(ret));
	return -2;
      }

      /* build the final tree */
      if((ret = cl_build(root))) {
	/* free the partial tree */
	cl_free(root); 
	/* Raise exception with error message */
	PyErr_SetString(PyclamavError, cl_strerror(ret));
	return -2;
      }

      /* Reinit db status update check */
      cl_statfree(&dbstat);
      cl_statinidir(cl_retdbdir(), &dbstat);

      return -1;
    }
  else
    {
      return 0;
    }

  return 0;
}




/*
 * Checks if the given filename is a directory or not 
 */
int filename_is_dir(char *file)
{
#ifdef __linux__
        struct stat64 buf;
        if(stat64(file, &buf) < 0) return(0);
#else  /* For FreeBSD */
#warning Not a Linux system... 
        struct stat buf;
        if(stat(file, &buf) < 0) return(0);	
#endif
        return(S_ISDIR(buf.st_mode));
}



/*
 * Return pyclamav version
 */

static PyObject *pyclamav_version(PyObject *self, PyObject *args)
{
    return Py_BuildValue("s", PYCLAMAV_VERSION);
}



/*
 * Return clamav version
 */

static PyObject *pyclamav_get_version(PyObject *self, PyObject *args)
{
    const char *version;
    int daily_version = 0;
    int daily_date = 0;

    const char *dbdir;
    char *path = NULL;
    struct cl_cvd *daily;

    //Clamav version
    version = cl_retver();

    //Database version
    dbdir = cl_retdbdir();
    if((path = malloc(strlen(dbdir) + 11))){
        
        sprintf(path, "%s/daily.cvd", dbdir);

        if((daily = cl_cvdhead(path))){

            daily_date    = daily->stime;
            daily_version = daily->version;

            cl_cvdfree(daily);
        }
    }

    return Py_BuildValue("(s,i,i)", version, daily_version, daily_date);
}
    

/*
 * Return number of signature in the database as integer
 */
static PyObject *pyclamav_get_numsig(PyObject *self, PyObject *args)
{
  /* Raise exception if database error */
  if (if_database_have_changed_then_reload() == -2) {
    return NULL;
  }

  return Py_BuildValue("i", signumber);
}




/*
 * Scan a file given as a parameter
 */
static PyObject *pyclamav_scanfile(PyObject *self, PyObject *args)
{
  char *file_to_scan;
  unsigned long int size = 0;
  const char *virname;
  int ret = 0;

  /* Raise exception if database error */
  if (if_database_have_changed_then_reload() == -2) {
    return NULL;
  }

  if (!PyArg_ParseTuple(args, "s", &file_to_scan)) {
    /* Raise exception with error message */
    PyErr_SetString(PyExc_TypeError,  "Pass filename to scan (string) as argument");
    return NULL; 
  }
  
  /* Test if the filename is really an existing file and not a directory for example */
  if (filename_is_dir(file_to_scan)) {
    PyErr_SetString(PyExc_ValueError,  "Argument is not a filename");
    return NULL;     
  }

  ret = cl_scanfile(file_to_scan, &virname, &size, root, &limits, CL_SCAN_STDOPT);

  /* Test return code */
  switch (ret) {
  case CL_VIRUS : /* File contains a virus */
    return Py_BuildValue("(i,s)",1, virname);
    break;
  case CL_CLEAN : /* File is clean */
    return Py_BuildValue("(i,s)",0, "");
    break;
  default: /* Error : raise exception with message */
    PyErr_SetString(PyExc_ValueError,  cl_strerror(ret));
    return NULL;
  }

}





static PyMethodDef ClamavMethods[] = {
    {"scanfile",  pyclamav_scanfile, METH_VARARGS, "scanfile(filename) : Scan a file for virus.\nArguments : filename (string)\n Return a tupple (status, virusname) where status=0 when no virus found\n or status=1 if a virus was found\n May raise a ValueError exception if an error occurs\n May raise a TypeError exception if wrong arguments are passed\n"},
    {"get_numsig",  pyclamav_get_numsig, METH_VARARGS, "get_numsig() : Get the number of know virii signatures\nArguments : None\n Return the number of known signatures.\n"},
    {"get_version",  pyclamav_get_version, METH_VARARGS, "get_version() : Get Clamav version.\nArguments : None\n Return the version of Clamav as a tupple (version, daily_version, daily_date).\n"},
    {"version",  pyclamav_version, METH_VARARGS, "version() : Get pyclamav version.\nArguments : None\n Return the version of pyclamav.\n"},
    {NULL, NULL, 0, NULL}    
};




PyMODINIT_FUNC initpyclamav(void)
{
  int ret= 0;

  PyObject *module, *dict;
  module=Py_InitModule("pyclamav", ClamavMethods);
  dict = PyModule_GetDict(module);

  PyclamavError = PyErr_NewException("pyclamav.error", NULL, NULL);
  PyDict_SetItemString(dict, "error", PyclamavError);


  /* Set documentation string for the module */
  PyDict_SetItemString(dict, "__doc__", PyString_FromString("pyclamav :\n\n  This is a python binding to the C libclamav library\n  (from the Clamav project - http://www.clamav.net).\n  It can be used to easily allow a Python script to scan\n  a file or a buffer against known viruses.\n\nAuthor : Alexandre Norman [norman@xael.org]\n\nFunctions :\n  - scanfile(string filename) : Scan a file for virus.\n  - get_numsig() : Return the number of known signatures.\n  - get_version() : Return the version of Clamav.\n  - version() : Return the version of pyclamav.\n"));


  if((ret = cl_load(cl_retdbdir(), &root, &signumber, CL_DB_STDOPT))) {
    /* Raise exception with error message */
    PyErr_SetString(PyclamavError,  cl_strerror(ret));
    return;
  }

  /* build the final tree */
  if((ret = cl_build(root))) {
    /* free the partial tree */
    cl_free(root); 
    /* Raise exception with error message */
    PyErr_SetString(PyclamavError, cl_strerror(ret));
    return;
  }


  /* Set dbstat to get notification of database changes */
  memset(&dbstat, 0, sizeof(struct cl_stat));
  cl_statinidir(cl_retdbdir(), &dbstat);



  /* set up archive limits */
  memset(&limits, 0, sizeof(struct cl_limits));
  limits.maxfiles = 1000; /* max files */
  limits.maxfilesize = 10 * 1048576; /* maximal archived file size == 10 Mb */
  limits.maxreclevel = 5; /* maximal recursion level */
  limits.archivememlim = 0; /* disable memory limit for bzip2 scanner */

  return ;
}



int main(int argc, char *argv[])
{
  /* Pass argv[0] to the Python interpreter */
  Py_SetProgramName(argv[0]);

  /* Initialize the Python interpreter.  Required. */
  Py_Initialize();
  
  /* Add a static module */
  initpyclamav();
  
  return 0;
}
