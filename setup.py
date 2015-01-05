
from distutils.core import setup, Extension

pyclamav = Extension('pyclamav',
                    sources = ['pyclamav.c'], 
		    libraries = ['clamav'],
		    library_dirs=['/usr/local/lib'])

# Build : python setup.py build
# Install : python setup.py install
# Register : python setup.py register

#  platform = 'Unix',
#  download_url = 'http://xael.org/norman/python/pyclamav/',


setup (name = 'pyclamav',
       version = '0.4.1',
       author = 'Alexandre Norman',
       author_email = 'norman()xael.org',
       license ='GPL',
       keywords="python, clamav, antivirus, scanner, virus, libclamav",
       url = 'http://xael.org/norman/python/pyclamav/',
       include_dirs = ['/usr/local/include'],
       description = 'This is a python binding to the C libclamav library (from the Clamav project - http://www.clamav.net). It can be used to easily allow a Python script to scan a file or a buffer against known viruses.',
       long_description = 'This is a python binding to the C libclamav library (from the Clamav project - http://www.clamav.net). It can be used to easily allow a Python script to scan a file or a buffer against known viruses.',
       ext_modules = [pyclamav])
