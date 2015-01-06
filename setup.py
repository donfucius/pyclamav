
from distutils.core import setup, Extension

pyclamav = Extension('pyclamav',
                    sources = ['pyclamav.c'], 
		    libraries = ['libclamav', 'python27'],
		    library_dirs=['.'])

# Build : python setup.py build
# Install : python setup.py install
# Register : python setup.py register

#  platform = 'Unix',
#  download_url = 'https://github.com/donfucius/pyclamav/tree/mypyclamav',


setup (name = 'pyclamav',
       version = '0.0.1',
       author = 'Rocky Dong',
       author_email = '',
       license ='GPL',
       keywords="python, clamav, antivirus, scanner, virus, libclamav",
       url = 'https://github.com/donfucius/pyclamav/tree/mypyclamav',
       include_dirs = ['.'],
       description = 'This is a python binding to the C libclamav library (from the Clamav project - http://www.clamav.net) based on http://xael.org/norman/python/pyclamav/. It can be used to easily allow a Python script to scan a file against known viruses.',
       long_description = 'This is a python binding to the C libclamav library (from the Clamav project - http://www.clamav.net) based on http://xael.org/norman/python/pyclamav/. It can be used to easily allow a Python script to scan a file against known viruses.',
       ext_modules = [pyclamav])
