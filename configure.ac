#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([primes], [0.01], [gustav.behm@gmail.com])

AM_INIT_AUTOMAKE

AC_PROG_LIBTOOL

m4_include([ac_header_traces.m4])
m4_include([boost.m4])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile])


# The debug option
AC_HEADER_ASSERT
AC_HEADER_TRACES
AC_HEADER_TIMING



# Checks for programs.
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11

# Checks for libraries
AX_PTHREAD

BOOST_REQUIRE
BOOST_PROGRAM_OPTIONS

LIBS="$PTHREAD_LIBS $LIBS $BOOST_LDFLAGS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS $BOOST_CPPFLAGS"


# Include the cppformat library
CPPFORMAT_INCL='-I$(top_srcdir)/cppformat'
CPPFORMAT_LIBS='-L$(top_srcdir)/cppformat -lformat'
AC_SUBST(CPPFORMAT_INCL)
AC_SUBST(CPPFORMAT_LIBS)


# Checks for header files.


# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([pow sqrt])

AC_OUTPUT
