SUBDIRS = deps/ae src doc test

all-am:
	cd deps/ae && $(MAKE)

install:
	cd src && $(MAKE) install

deb: all-am
	cd debian && ./make_deb.sh

test: check
