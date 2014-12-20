SUBDIRS = deps/ae src doc test

all-am:
	cd deps/ae && $(MAKE)

install:
	cd src && $(MAKE) install

test: check
