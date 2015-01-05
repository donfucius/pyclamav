# Get version number
VERSION=`python setup.py --version`
FILES=CHANGELOG Makefile pyclamav.c setup.py README.txt gpl.txt example.py
ARCHIVE=`python setup.py --fullname`

#LIBS=`clamav-config --libs`
#CFLAGS=`clamav-config --cflags`


all:
	@python setup.py build

clean:
	@python setup.py clean

build:
	@python setup.py build

install:
	@python setup.py install

archive:
	@rm -rf $(ARCHIVE)/
	@mkdir $(ARCHIVE)
	@cp $(FILES) $(ARCHIVE)/
	@tar cvzf $(ARCHIVE).tar.gz $(ARCHIVE)/
	@rm -rf $(ARCHIVE)/
	@echo Archive is create and named $(ARCHIVE).tar.gz
	@echo -n md5sum is :
	@md5sum $(ARCHIVE).tar.gz

license:
	@python setup.py --license

register:
	@python setup.py register
