#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define(pkg_major_version, 0)
m4_define(pkg_minor_version, 1)
m4_define(pkg_micro_version, 0)
m4_define(pkg_version, pkg_major_version.pkg_minor_version.pkg_micro_version)

AC_PREREQ([2.69])
AC_INIT([gjs-hunspell], pkg_version, [https://github.com/kris7topher/gjs-hunspell/issues/new])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([gjs-hunspell/hunspell.cxx])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE(1.12)
AC_LANG([C++])

# Do not build static libraries.
AM_DISABLE_STATIC
AC_PROG_LIBTOOL

# Use C++11.
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext])

# Checks for libraries.
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([gjs], [gjs-1.0 >= 1.34.0])
PKG_CHECK_MODULES([mozjs], [mozjs185])
PKG_CHECK_MODULES([hunspell], [hunspell >= 1.3.2])

# Checks for header files.
AC_CHECK_HEADERS([algorithm string vector stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Directory to install native library to.
gjsnativedir="\${libdir}/gjs-1.0"
AC_SUBST([gjsnativedir])

# Option to enable experimental Hunspell features.
AC_ARG_ENABLE([experimental],
	AS_HELP_STRING([--enable-experimental],
		[Experimental Hunspell features.]))
AS_IF([test "x$enable_experimental" = "xyes"], [
	AC_DEFINE([HUNSPELL_EXPERIMENTAL], [1],
		[Define to enable bindings for experimental Hunspell features.])
])

AC_OUTPUT
