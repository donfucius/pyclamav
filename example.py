#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
example.py

An example for pyclamav python module

Version 0.3.1

Author : Alexandre Norman - norman()xael.org - 2005
License : GPL

Usage :

example.py [file|directory]

Scan the given file or directory, if none is given,
scan current directory
"""

import sys, dircache, os
import pyclamav

############################################################################

def temporary_filename(prefix='rand_',suffix='.an', create=True):
    """
    Return a temporary unique filename in the
    form /tmp/rand_2004-05-11.92194015.an

    Filename is created (if create is equal to true) in order
    to be sure that it is unique. You have to destroy it after use.

    prefix : string
    suffix : string
    create : boolean
    
    """
    import random, datetime

    ddate=str(datetime.date.today())
    filename='/'
    while os.access(filename, os.F_OK):
        filename=os.path.join('/tmp',
                              prefix+'.'+ddate+'.' \
                              +str(random.randint(10000000,99999999))+'.'+suffix)
    if create==True:
        file=open(filename,'w')
        file.close()
    
    return filename


############################################################################

def scanfile(file):
    """ Scan a given file
    """
    # Call libclamav thought pyclamav
    try:
        ret=pyclamav.scanfile(file)
    except ValueError, e:
        print '** A problem as occured :', e, '("'+file+'")'
        return None
    except TypeError, e:
        print '** A problem as occured :', e, '("'+file+'")'
        return None
    else:
        # Check return tupple
        if ret[0]==0:
            print file, 'is not infected'
            return True
        elif ret[0]==1:
            print file, 'is infected with', ret[1]
            return False

############################################################################

def scanthis(buffer):
    """ Scan a given buffer
    """
    try:
        ret=pyclamav.scanthis(buffer)
    except ValueError, e:
        print '** A problem as occured :', e
        return None
    except TypeError, e:
        print '** A problem as occured :', e
        return None
    else:
        # Check return tupple
        if ret[0]==0:
            print 'buffer is not infected'
            return True
        elif ret[0]==1:
            print 'buffer is infected with', ret[1]
            return False


############################################################################

def scanthis_secure(buffer):
    """ A more secure way to scan a given buffer,
    because scanning a buffer on the fly is less efficient
    with libclamav...
    """
    tempfilename=temporary_filename()
    fd=open(tempfilename,'w')
    fd.write(buffer)
    fd.close()

    # Call libclamav thought pyclamav
    try:
        ret=pyclamav.scanfile(tempfilename)
    except ValueError, e:
        print '** A problem as occured :', e
        os.remove(tempfilename)
        return None
    except TypeError, e:
        print '** A problem as occured :', e
        os.remove(tempfilename)
        return None
    else:
        os.remove(tempfilename)
        # Check return tupple
        if ret[0]==0:
            print 'buffer is not infected'
            return True
        elif ret[0]==1:
            print 'buffer is infected with', ret[1]
            return False


############################################################################

# MAIN -------------------
if __name__ == '__main__':
    
    # Do we have an argument on command line ?
    if len(sys.argv)>1:
        # is it a directory to scan ?
        dirlisting=dircache.listdir(sys.argv[1])
        if dirlisting!=[]:
            for file in dirlisting:
                scanfile(file)
        # Nope, it may be a file
        else:
            scanfile(sys.argv[1])

    # No argument : scan current dir
    else:
        dirlisting=dircache.listdir('./')
        if dirlisting!=[]:
            for file in dirlisting:
                scanfile(file)
            
        # Scan the given buffer
        scanthis("this one is OK")

        # Just for AV software... not to raise an alert
        a="7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*"
        b="X5O!P%@AP[4\PZX54(P^)7CC)"
        scanthis_secure(b+a)

#<EOF>######################################################################

