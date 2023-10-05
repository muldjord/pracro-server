# Filename: configure.in

AC_INIT([pracrod],[2.3.1])
AC_CONFIG_SRCDIR([src/pracrod.cc])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_CC_C_O

dnl ======================
dnl Compile with debug options
dnl ======================
AC_ARG_WITH(debug,
	[  --with-debug            build with debug support (default=no)],
	[],
	[with_debug=no])
if test x$with_debug == xyes; then
    AC_MSG_WARN([*** Building with debug support!])
    AC_DEFINE_UNQUOTED(WITH_DEBUG, , [The project is configured to use debug output])
    CXXFLAGS="$CXXFLAGS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -fstack-protector -Wall -Werror -g -O0"
fi

dnl ======================
dnl Check for inotify (old and new)
dnl ======================
inotify_support=no
AC_CHECK_HEADERS([sys/inotify.h],
[
  AC_CHECK_FUNCS(inotify_init1, inotify_support=yes
)
])
if test "$inotify_support" = "no"
then
  AC_MSG_ERROR([*** Inotify not found (needs at least 2.6.21 kernel)!])
fi
#AM_CONDITIONAL(HAVE_INOTIFY, [test "$inotify_support" = "yes"])

dnl ======================
dnl Init pkg-config
dnl ======================
PKG_PROG_PKG_CONFIG(0.23)

dnl ======================
dnl Compile with artefact support
dnl ======================
AC_ARG_WITH(artefact,
	[  --with-artefact         build with artefact support (default=yes)],
	[],
	[with_artefact=yes])
if test x$with_artefact == xyes; then
    PKG_CHECK_MODULES(ATF, libartefact >= 0.0.2)
else
    AC_MSG_WARN([*** Building without artefact support!])
    AC_DEFINE_UNQUOTED(WITHOUT_ARTEFACT, , [The project is configured not to use artefact])
fi 

dnl ======================
dnl Compile without uploadserver support
dnl ======================
AC_ARG_WITH(uploadserver,
	[  --with-uploadserver     build with uploadserver support (default=yes)],
	[],
	[with_uploadserver=yes])
if test x$with_uploadserver == xno; then
    AC_MSG_WARN([*** Building without uploadserver support!])
    AC_DEFINE_UNQUOTED(WITHOUT_UPLOADSERVER, , [The project is configured not to use the upload server])
fi 

dnl ======================
dnl Compile without db support
dnl ======================
AC_ARG_WITH(db,
	[  --with-db               build with db support (default=yes)],
	[],
	[with_db=yes])
if test x$with_db == xno; then
    AC_MSG_WARN([*** Building without db support!])
    AC_DEFINE_UNQUOTED(WITHOUT_DB, , [The project is configured not to use the db])
else
    dnl ======================
    dnl Check for libpg
    dnl ======================
    PQ_CFLAGS="-I$(pg_config --includedir)"
    AC_SUBST(PQ_CFLAGS)
    PQ_LIBS="-L$(pg_config --libdir) -lpq"
    AC_SUBST(PQ_LIBS)
fi 

dnl ======================
dnl Compile with ssl support?
dnl ======================
AC_ARG_WITH(ssl,
	[  --with-ssl              build with ssl support, requires microhttpd to be build with ssl support (default=yes)],
	[],
	[with_ssl=yes])
if test x$with_ssl == xno; then
    AC_MSG_WARN([*** Building without ssl support!])
    AC_DEFINE_UNQUOTED(WITHOUT_SSL, , [The project is configured not to use ssl])
else
    AC_MSG_WARN([*** Building with ssl support!])
    dnl ======================
    dnl TODO: Check for ssl support in microhttpd
    dnl ======================
fi 

AC_PROG_CXX

LT_INIT
LT_INIT

AC_CONFIG_HEADERS(config.h)

dnl ======================
dnl Create the ETC var i config.h
dnl ======================
if echo "$prefix" | grep "NONE" > /dev/null
then
	MYPREFIX="/usr/local"
else
	MYPREFIX="${prefix}"
fi
AC_SUBST(MYPREFIX)
AC_DEFINE_UNQUOTED(ETC, "$MYPREFIX/etc", [The path to the config files])

dnl ======================
dnl Use efence in linking and includes
dnl ======================
AC_ARG_ENABLE(efence,
	[  --enable-efence         enable efence - for debugging only (no)],
	[], [ enable_efence=no])             
if test "x$enable_efence" = xno; then
	enable_efence=no
else
	LD_EFENCE="-lefence"
	AC_SUBST(LD_EFENCE)
	AC_DEFINE_UNQUOTED(USE_EFENCE, , [Use the efence includes])
fi

dnl ======================
dnl Use duma in linking and includes
dnl ======================
AC_ARG_ENABLE(duma,
	[  --enable-duma           enable duma - for debugging only (no)],
	[], [ enable_duma=no])             
if test "x$enable_duma" = xno; then
	enable_duma=no
else
	LD_EFENCE="-lduma"
	AC_SUBST(LD_DUMA)
	AC_DEFINE_UNQUOTED(USE_DUMA, , [Use the duma includes])
fi

dnl ======================
dnl Check for lua
dnl ======================
PKG_CHECK_MODULES(LUA, lua-5.1 >= 5.1)

dnl ======================
dnl Create the XML var i config.h
dnl ======================
AC_DEFINE_UNQUOTED(XML, "$MYPREFIX/share/xml", [The path to the xml files])

dnl ======================
dnl Check for getopt
dnl ======================
AC_CHECK_HEADERS([getopt.h])

dnl ======================
dnl Check for libconfig++
dnl ======================
PKG_CHECK_MODULES(CONFIG, libconfig++ >= 1.0.1)

dnl ======================
dnl Check for libmicrohttpd
dnl ======================
PKG_CHECK_MODULES(HTTPD, libmicrohttpd >= 0.4.4)

dnl ======================
dnl Compile with openssl
dnl ======================
PKG_CHECK_MODULES(OPENSSL, openssl >= 0.9.7)

dnl ======================
dnl Check for eXpat library
dnl ======================
tmp_CXXFLAGS="$CXXFLAGS"
tmp_CPPFLAGS="$CPPFLAGS"
tmp_CFLAGS="$CFLAGS"
tmp_LDFLAGS="$LDFLAGS"
tmp_LIBS="$LIBS"
CXXFLAGS=""
CPPFLAGS=""
CFLAGS=""
LDFLAGS=""
LIBS=""
AC_CHECK_HEADER(expat.h, , AC_MSG_ERROR([*** eXpat header file not found!]))
AC_CHECK_LIB(expat, XML_ParserCreate, , AC_MSG_ERROR([*** eXpat library not found!]))
EXPAT_CFLAGS="$CXXFLAGS $CPPFLAGS $CFLAGS"
EXPAT_LIBS="$LDFLAGS $LIBS"
CXXFLAGS="$tmp_CXXFLAGS"
CPPFLAGS="$tmp_CPPFLAGS"
CFLAGS="$tmp_CFLAGS"
LDFLAGS="$tmp_LDFLAGS"
LIBS="$tmp_LIBS"
AC_SUBST(EXPAT_CFLAGS)
AC_SUBST(EXPAT_LIBS)

dnl ======================
dnl Check for the pthread library
dnl ======================
tmp_CXXFLAGS="$CXXFLAGS"
tmp_CPPFLAGS="$CPPFLAGS"
tmp_CFLAGS="$CFLAGS"
tmp_LDFLAGS="$LDFLAGS"
tmp_LIBS="$LIBS"
CXXFLAGS=""
CPPFLAGS=""
CFLAGS=""
LDFLAGS=""
LIBS=""
AC_CHECK_HEADER(pthread.h, , AC_MSG_ERROR([*** pthread header file not found!]))
AC_CHECK_LIB(pthread,  pthread_mutex_init, , AC_MSG_ERROR([*** pthread library not found!]))
PTHREAD_CFLAGS="$CXXFLAGS $CPPFLAGS $CFLAGS"
PTHREAD_LIBS="$LDFLAGS $LIBS"
CXXFLAGS="$tmp_CXXFLAGS"
CPPFLAGS="$tmp_CPPFLAGS"
CFLAGS="$tmp_CFLAGS"
LDFLAGS="$tmp_LDFLAGS"
LIBS="$tmp_LIBS"
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

AC_CONFIG_FILES([Makefile
	src/Makefile
	etc/Makefile
	man/Makefile
	tools/Makefile
	xml/templates/Makefile
	xml/macros/Makefile
	xml/courses/Makefile
	xml/Makefile])
AC_OUTPUT

