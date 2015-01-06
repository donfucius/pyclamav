#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
	#include <unistd.h>
#else
	#include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <clamav.h>

/* Exception object for python */
static PyObject *PyclamavError;

unsigned int signumber = 0;

struct cl_stat dbstat = { 0 };

struct cl_engine *engine = NULL;
enum {
		False = 0,
		True
};


unsigned char database_needs_reload()
{
	if ((cl_statchkdir(&dbstat) == 1) || (engine == NULL)) {
		signumber = 0;
		if (engine != NULL) {
			cl_engine_free(engine);
			engine = NULL;
		}
		return True;
	}
	
	return False;
}

int load_database(void)
{
	int ret = -1;

	/* Test if the database have changed */
	/* If yes : reload DB                */
	if (database_needs_reload()) {
		if(!(engine = cl_engine_new())) {
		PyErr_SetString(PyclamavError,  "Creating database failed.");
		return ret;
		}
	
		/* load all available databases from default directory */
		if((ret = cl_load(dbstat.dir, engine, &signumber, CL_DB_STDOPT)) != CL_SUCCESS) {
			PyErr_SetString(PyclamavError,  cl_strerror(ret));
			cl_engine_free(engine);
			engine = NULL;
			return ret;
		}

		/* build engine */
		if((ret = cl_engine_compile(engine)) != CL_SUCCESS) {
			PyErr_SetString(PyclamavError,  cl_strerror(ret));
			cl_engine_free(engine);
			engine = NULL;
			return ret;
		}
	}
	
	ret = CL_SUCCESS;
	return ret;
}

/*
 * Checks if the given filename is a directory or not 
 */
unsigned char filename_is_dir(char *file)
{
#ifdef __linux__
        struct stat64 buf;
        if(stat64(file, &buf) < 0) return(0);
#else  /* For FreeBSD */
        struct stat buf;
        if(stat(file, &buf) < 0) return(0);	
#endif
        return(S_ISDIR(buf.st_mode));
}

static PyObject *pyclamav_get_numsig(PyObject *self, PyObject *args)
{
  
	if (database_needs_reload()) {
		if (load_database() != CL_SUCCESS) {
			return NULL;
		}
	}

	return Py_BuildValue("i", signumber);
}

static PyObject *pyclamav_get_version(PyObject *self, PyObject *args)
{
    return Py_BuildValue("s", cl_retver());
}

static PyObject *pyclamav_check_dbfile(PyObject *self, PyObject *args)
{
	int ret = -1;
	char *filename = NULL;
	struct cl_engine *_engine = NULL;

	if (!PyArg_ParseTuple(args, "s", &filename)) {
		/* Raise exception with error message */
		PyErr_SetString(PyclamavError, "Pass filename to check (string) as argument");
		return Py_BuildValue("i", ret);
	}
	
	if (filename_is_dir(filename)) {
		PyErr_SetString(PyclamavError, "Argument is not a filename");
		return Py_BuildValue("i", ret);
	}
	if(!(_engine = cl_engine_new())) {
		PyErr_SetString(PyclamavError,  "Creating database failed.");
		return Py_BuildValue("i", ret);
	}
	if((ret = cl_load(filename, _engine, &signumber, CL_DB_STDOPT)) != CL_SUCCESS) {
		PyErr_SetString(PyclamavError,  cl_strerror(ret));
		cl_engine_free(_engine);
		return Py_BuildValue("i", ret);
	}
	if((ret = cl_engine_compile(_engine)) != CL_SUCCESS) {
		PyErr_SetString(PyclamavError,  cl_strerror(ret));
		cl_engine_free(_engine);
		return Py_BuildValue("i", ret);
	}
		
	return Py_BuildValue("i", CL_SUCCESS);
}

static PyObject *pyclamav_scanfile(PyObject *self, PyObject *args)
{
	char *file_to_scan;
	unsigned long int size = 0;
	const char *virname;
	int ret = 0;
	
	if (database_needs_reload()) {
		if (load_database() != CL_SUCCESS) {
			return NULL;
		}
	}

	if (!PyArg_ParseTuple(args, "s", &file_to_scan)) {
		/* Raise exception with error message */
		PyErr_SetString(PyExc_TypeError,  "Pass filename to scan (string) as argument");
		return NULL; 
	}
	
	/* scan file */
	ret = cl_scanfile(file_to_scan, &virname, &size, engine, CL_SCAN_STDOPT);

	/* Test return code */
	switch (ret) {
	case CL_VIRUS : /* File contains a virus */
		return Py_BuildValue("(i,s)", 1, virname);
		break;
	case CL_CLEAN : /* File is clean */
		return Py_BuildValue("(i,s)", 0, "");
		break;
	default: /* Error : raise exception with message */
		PyErr_SetString(PyExc_ValueError,  cl_strerror(ret));
		return NULL;
	}
}

static PyObject *pyclamav_set_dbpath(PyObject *self, PyObject *args)
{
	int ret = -1;
	char *dirname = NULL;

	cl_statfree(&dbstat);
	memset(&dbstat, 0, sizeof(struct cl_stat));

	if (engine != NULL) {
		cl_engine_free(engine);
		engine = NULL;
	}
    
	if (!PyArg_ParseTuple(args, "s", &dirname))
		/* Raise exception with error message */
		PyErr_SetString(PyExc_TypeError,  "Pass filename to scan (string) as argument");
	else {
		cl_statinidir(dirname, &dbstat);
		ret = CL_SUCCESS;
	}

	return Py_BuildValue("i", ret);
}

static PyMethodDef ClamavMethods[] = {
	{"set_database_path", pyclamav_set_dbpath, METH_VARARGS, "set_database_path(path) : Set path of database.\nArguments : path (string)\n"},
	{"check_database_file",  pyclamav_check_dbfile, METH_VARARGS, "check_database_file(filename) : Check if database file can be correctly compiled.\nArguments : filename (string)\n Return CL_SUCCESS if it can be compiled successfully.\n"},
	{"scanfile",  pyclamav_scanfile, METH_VARARGS, "scanfile(filename) : Scan a file for virus.\nArguments : filename (string)\n Return a tupple (status, virusname) where status=0 when no virus found\n or status=1 if a virus was found\n May raise a ValueError exception if an error occurs\n May raise a TypeError exception if wrong arguments are passed.\n"},
	{"get_numsig",  pyclamav_get_numsig, METH_NOARGS, "get_numsig() : Get the number of known virii signatures\nArguments : None\n Return the number of known signatures.\n"},
	{"get_version",  pyclamav_get_version, METH_NOARGS, "get_version() : Get Clamav version.\nArguments : None\n Return the version of Clamav.\n"},
	{NULL, NULL, 0, NULL}    
};

PyMODINIT_FUNC initpyclamav(void)
{
	int ret = -1;
	PyObject *module, *dict;

	module = Py_InitModule("pyclamav", ClamavMethods);
	dict = PyModule_GetDict(module);

	PyclamavError = PyErr_NewException("pyclamav.error", NULL, NULL);
	PyDict_SetItemString(dict, "error", PyclamavError);

	if((ret = cl_init(CL_INIT_DEFAULT)) != CL_SUCCESS) {
		PyErr_SetString(PyclamavError,  cl_strerror(ret));
		return;
    }

	memset(&dbstat, 0, sizeof(struct cl_stat));
	cl_statinidir("", &dbstat);

	return;
}

int main(int argc, char* argv[])
{
	/* Pass argv[0] to the Python interpreter */
	Py_SetProgramName(argv[0]);

	/* Initialize the Python interpreter.  Required. */
	Py_Initialize();
  
	/* Add a static module */
	initpyclamav();

	return 0;
}
