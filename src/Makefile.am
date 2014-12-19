check:

DEPS_DIR = @top_srcdir@/deps

KIDS_SOURCE = \
	buffer.h conf.h kids.h msgqueue.h storer.h \
	client.h constants.h logger.h parser.h util.h \
	common.h filesystem.h master.h sds.h worker.h \
	kids.cc \
	sds.c util.cc conf.cc logger.cc buffer.cc \
	master.cc storer.cc worker.cc \
	msgqueue.cc filesystem.cc \
	client.cc pubsub.cc \
	store/bufferstore.h store/multiplestore.h store/prioritystore.h \
	store/filestore.h store/networkstore.h store/store.h \
	store/store.cc store/filestore.cc store/multiplestore.cc \
	store/bufferstore.cc store/networkstore.cc store/prioritystore.cc

EXTRA_DIST = \
	parser.c parser.y lexer.c lexer.rl

bin_PROGRAMS = kids

kids_SOURCES = $(KIDS_SOURCE)
kids_SOURCES += $(AE_SOURCE)

kids_CPPFLAGS = -I$(DEPS_DIR) $(AM_CPPFLAGS)
kids_CXXFLAGS = $(AM_CXXFLAGS)
kids_CFLAGS = $(AM_CFLAGS)

kids_LDADD = $(DEPS_DIR)/ae/libae.a

kids_LIBS = pthread
