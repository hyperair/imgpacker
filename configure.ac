#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([imgpacker], [0.1], [hyperair@ubuntu.com])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AM_SILENT_RULES([yes])
PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])

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

AC_CONFIG_FILES([
    Makefile
    po/Makefile.in
])
AC_OUTPUT
