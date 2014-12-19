DEPS_DIR = $(top_srcdir)/deps

KIDS_DIR = $(top_srcdir)/src
GTEST_DIR = gtest

AE_SOURCE = @top_srcdir@/deps/ae/ae.h \
						@top_srcdir@/deps/ae/ae.c \
						@top_srcdir@/deps/ae/zmalloc.h \
						@top_srcdir@/deps/ae/zmalloc.c \
						@top_srcdir@/deps/ae/anet.h \
						@top_srcdir@/deps/ae/anet.c \
						@top_srcdir@/deps/ae/fmacros.h \
						@top_srcdir@/deps/ae/libae.h

KIDS_SOURCE = @top_srcdir@/src/store/store.cc \
							@top_srcdir@/src/store/bufferstore.cc\
							@top_srcdir@/src/store/filestore.cc\
							@top_srcdir@/src/store/multiplestore.cc\
							@top_srcdir@/src/store/networkstore.cc\
							@top_srcdir@/src/store/prioritystore.cc\
							@top_srcdir@/src/buffer.cc \
							@top_srcdir@/src/client.cc \
							@top_srcdir@/src/conf.cc \
							@top_srcdir@/src/filesystem.cc \
							@top_srcdir@/src/logger.cc \
							@top_srcdir@/src/msgqueue.cc \
							@top_srcdir@/src/pubsub.cc \
							@top_srcdir@/src/master.cc \
							@top_srcdir@/src/worker.cc \
							@top_srcdir@/src/storer.cc \
							@top_srcdir@/src/sds.c \
							@top_srcdir@/src/util.cc \
							@top_srcdir@/src/kids.h

GTEST_SOURCE = gtest/gtest-all.cc \
							 gtest/gtest.h

TEST_SOURCE = test_message.cc \
							test_conf.cc \
							test_filesystem.cc \
							test_buffer.cc \
							test_pattern.cc \
							test_util.cc \
							test_main.cc

check_PROGRAMS = runtest

runtest_SOURCES = $(TEST_SOURCE)
runtest_SOURCES += $(KIDS_SOURCE)
runtest_SOURCES += $(AE_SOURCE)
runtest_SOURCES += $(GTEST_SOURCE)

CXXFLAGS := $(filter-out -Werror,$(CXXFLAGS))
runtest_CXXFLAGS = -std=c++11 -I$(KIDS_DIR) -I$(DEPS_DIR) -DTEST_DIR=\"$(top_srcdir)/test/data\"

LIBS = -lpthread

TESTS = $(check_PROGRAMS)

CLEANFILES = runtest

EXTRA_DIST = data
