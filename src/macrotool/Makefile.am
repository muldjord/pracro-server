
bin_PROGRAMS = macrotool

macrotool_LDADD = $(LD_EFENCE) $(PQXX_LIBS) $(CONFIG_LIBS) \
	$(LUA_LIBS) $(PTHREAD_LIBS) $(EXPAT_LIBS)

macrotool_CXXFLAGS = -I.. $(PQXX_CXXFLAGS) $(CONFIG_CXXFLAGS) \
	$(LUA_CFLAGS) $(PTHREAD_CFLAGS) $(EXPAT_CFLAGS)

macrotool_SOURCES = \
	macrotool.cc \
	dump.cc \
	export.cc \
	fieldnames.cc \
	filehandler.cc \
	sunlock.cc \
	../debug.cc \
	../configuration.cc \
	../configurationparser.cc \
	../database.cc \
	../entitylist.cc \
	../exception.cc \
	../fieldnamescanner.cc \
	../inotify.cc \
	../log.cc \
	../macroheaderparser.cc \
	../macrolist.cc \
	../macroparser.cc \
	../mutex.cc \
	../pracrodao.cc \
	../pracrodaopgsql.cc \
	../pracrodaotest.cc \
	../saxparser.cc \
	../templateparser.cc \
	../util.cc \
	../versionstr.cc

EXTRA_DIST = \
	dump.h \
	export.h \
	fieldnames.h \
	filehandler.h \
	sunlock.h

################
# Test Section #
################

TEST_SOURCE_DEPS = ${macrotool_SOURCES} ${EXTRA_DIST}
TEST_SCRIPT_DIR = $(top_srcdir)/../tools

include ${TEST_SCRIPT_DIR}/Makefile.am.test

include Makefile.am.test