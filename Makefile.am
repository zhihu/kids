SUBDIRS = deps/ae src doc test

all-am:
	cd deps/ae && $(MAKE)

install:
	cd src && $(MAKE) install

deb: all-am
	cd debian && ./make_deb.sh

kids := kids_@VERSION@

bin: all-am
	mkdir $(kids) && cp src/kids $(kids) && cp README.md $(kids) && cp samples/{agent.conf,server.conf} $(kids) \
		      && cp LICENSE $(kids) && tar czf $(kids)_amd64.tar.gz $(kids) && rm -rf $(kids)
test: check
