AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS =

bin_PROGRAMS = pracrod

pracrod_LDADD = $(LD_EFENCE) $(PQ_LIBS) $(CONFIG_LIBS) \
	$(LUA_LIBS) $(HTTPD_LIBS) $(PTHREAD_LIBS) \
	$(EXPAT_LIBS) $(ATF_LIBS) $(CURL_LIBS)

pracrod_CXXFLAGS = $(PQ_CFLAGS) $(CONFIG_CXXFLAGS) \
	$(LUA_CFLAGS) $(HTTPD_CFLAGS) $(EXPAT_CFLAGS) \
	$(PTHREAD_CFLAGS) $(ATF_CFLAGS) $(CURL_CFLAGS) \
	-I$(top_srcdir)/../hugin -DWITH_HUG_FILTER

pracrod_CFLAGS = -DWITH_HUG_FILTER

pracrod_SOURCES = \
	pracrod.cc \
	admin_connection.cc \
	admin_export.cc \
	admin_rc.cc \
	artefact.cc \
	client_connection.cc \
	configuration.cc \
	configurationparser.cc \
	connection.cc \
	connectionpool.cc \
	courseparser.cc \
	courselist.cc \
	daemon.cc \
	database.cc \
	entitylist.cc \
	environment.cc \
	exception.cc \
	fieldnamescanner.cc \
	httpd.cc \
	inotify.cc \
	journal_commit.cc \
	journal.cc \
	journal_uploadserver.cc \
	log.cc \
	luascript.cc \
	luaoncommit.cc \
	luaquerymapper.cc \
	luaresume.cc \
	luautil.cc \
	macroheaderparser.cc \
	macrolist.cc \
	macroparser.cc \
	mutex.cc \
	pgwork.cc \
	pracrodao.cc \
	pracrodaopgsql.cc \
	pracrodaotest.cc \
	queryhandlerpentominos.cc \
	queryhandlerpracro.cc \
	queryparser.cc \
	saxparser.cc \
	semaphore.cc \
	server.cc \
	session.cc \
	sessionheaderparser.cc \
	sessionparser.cc \
	sessionserialiser.cc \
	templatelist.cc \
	templateheaderparser.cc \
	templateparser.cc \
	transactionhandler.cc \
	transactionparser.cc \
	tcpsocket.cc \
	utf8.cc \
	util.cc \
	versionstr.cc \
	widgetvalue.cc \
	widgetgenerator.cc \
	xml_encode_decode.cc \
	$(top_srcdir)/../hugin/hugin.c \
	$(top_srcdir)/../hugin/hugin_filter.c

EXTRA_DIST = \
	artefact.h \
	admin_connection.h \
	admin_export.h \
	admin_rc.h \
	client_connection.h \
	configuration.h \
	configurationparser.h \
	connection.h \
	connectionpool.h \
	courseparser.h \
	courselist.h \
	daemon.h \
	database.h \
	dbtypes.h \
	environment.h \
	entitylist.h \
	exception.h \
	fieldnamescanner.h \
	httpd.h \
	inotify.h \
	journal_commit.h \
	journal.h \
	journal_uploadserver.h \
	log.h \
	luascript.h \
	luaoncommit.h \
	luaquerymapper.h \
	luaresume.h \
	luautil.h \
	macroheaderparser.h \
	macrolist.h \
	macroparser.h \
	mutex.h \
	pgwork.h \
	pracrodao.h \
	pracrodaopgsql.h \
	pracrodaotest.h \
	queryhandler.h \
	queryhandlerpentominos.h \
	queryhandlerpracro.h \
	queryparser.h \
	queryresult.h \
	saxparser.h \
	semaphore.h \
	server.h \
	session.h \
	sessionheaderparser.h \
	sessionparser.h \
	sessionserialiser.h \
	template.h \
	templatelist.h \
	templateheaderparser.h \
	templateparser.h \
	transaction.h \
	transactionhandler.h \
	transactionparser.h \
	tcpsocket.h \
	utf8.h \
	util.h \
	versionstr.h \
	widgetvalue.h \
	widgetgenerator.h \
	xml_encode_decode.h \
	$(top_srcdir)/../hugin/hugin.h

################
# Test Section #
################

TEST_SOURCE_DEPS = ${pracrod_SOURCES} ${EXTRA_DIST}
TEST_SCRIPT_DIR = $(top_srcdir)/tools

include ${TEST_SCRIPT_DIR}/Makefile.am.test

#include Makefile.am.test
