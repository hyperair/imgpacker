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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
