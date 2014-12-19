SUBDIRS = deps/ae src doc test

all-am:
	cd deps/ae && $(MAKE)

install:
	cd src && $(MAKE) install

deb:
	cd epm && epm -a amd64 -f deb kids

test: check
