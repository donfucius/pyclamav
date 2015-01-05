--------------------------------------------------------------------------
pyclamav - a quick hack to use libclamav with python
author : Alexandre Norman <norman@xael.org> - 2005
--------------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
--------------------------------------------------------------------------

Intallation
===========

   These are generic installation instructions.

   You need to have clamav and libclamav v0.80 installed 
   with clamav.h header file
   You also need to have python installed
   
The simpliest way to compile this package is:

  1. Run 'python setup.py build'
  2. As root run 'python setup.py install'
  3. It should be done.


Usage
=====

From python :

Python 2.3.3 (#1, Mar 21 2004, 00:29:15) 
[GCC 2.95.4 20011002 (Debian prerelease)] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> import pyclamav
>>> ret=pyclamav.scanfile('/tmp/virus')
>>> print ret
(1, 'Worm.Sober.G')
>>> ret=pyclamav.scanthis("Buffer to test blalblabla...")
>>> print ret
(0, '')
>>> print pyclamav.get_numsig()
25474


Return value is a tupple (integer, string)
where integer 
= 1 : if a virus was found, string then contains the name of the virus
= 0 : no virus found, string is empty

If anything went wrong, an exception ValueError is raised. 
String contains the error string (see clamav.h )

-<EOF>--------------------------------------------------------------------
