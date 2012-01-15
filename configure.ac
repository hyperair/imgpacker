#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([imgpacker], [0.1], [hyperair@ubuntu.com])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_COMPILE_STDCXX_0X
if test "$ac_cv_cxx_compile_cxx0x_native" = yes; then
    CXX0X_FLAG=""
elif test "$ac_cv_cxx_compile_cxx0x_gxx" = yes; then
    CXX0X_FLAG="-std=gnu++0x"
elif test "$ac_cv_cxx_compile_cxx0x_cxx" = yes; then
    CXX0X_FLAG="-std=c++0x"
else
    AC_MSG_ERROR([C++0x support needed for $PACKAGE])
fi
AC_SUBST([CXX0X_FLAG])

AC_HEADER_STDCXX_0X
if test "$ac_cv_cxx_stdcxx_0x" = no; then
    AC_MSG_ERROR([C++0x headers needed to build $PACKAGE])
fi
AM_SILENT_RULES([yes])
PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])
PKG_CHECK_MODULES([NIHPP], [nihpp])

AC_LANG([C++])
AC_CHECK_HEADER([autosprintf.h])

dnl i18n bits
IT_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=imgpacker
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(
    [GETTEXT_PACKAGE],
    ["$GETTEXT_PACKAGE"],
    [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT
PROGRAMNAME_LOCALEDIR=[${datadir}/locale]
AC_SUBST(PROGRAMNAME_LOCALEDIR)
dnl end i18n bits

AC_ARG_ENABLE([warnings],
              [AS_HELP_STRING([--enable-warnings=yes], [Enable extra warning flags])],,
              [enable_warnings=yes])

AM_CONDITIONAL([ENABLE_WARNINGS], [test "$enable_warnings" = "yes"])

AC_CONFIG_FILES([
    Makefile
    po/Makefile.in
])
AC_OUTPUT
