#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([primes], [0.01], [gustav.behm@gmail.com])

AM_INIT_AUTOMAKE

m4_include([ac_header_traces.m4])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

# Parameters

AC_ARG_VAR(THREADS, Number of worker threads)
if test -z "$THREADS"; then
    THREADS=`echo \`nproc\` - 2 | bc`
fi
AC_DEFINE_UNQUOTED(THREADS, $THREADS, Number of worker threads)


AC_ARG_VAR(TOTAL_PRIMES, The number of primes to find)
if test -z "$TOTAL_PRIMES"; then
    TOTAL_PRIMES=99
fi
AC_DEFINE_UNQUOTED(TOTAL_PRIMES, $TOTAL_PRIMES, The number of primes to find)

AC_ARG_VAR(BUFFER_SIZE, The buffer size)
if test -z "$BUFFER_SIZE"; then
    BUFFER_SIZE=120
fi
AC_DEFINE_UNQUOTED(BUFFER_SIZE, $BUFFER_SIZE, The buffer size)

AC_ARG_VAR(CHECK_PRIMES, The number of prime factors needed for checks)
if test -z "$CHECK_PRIMES"; then
    CHECK_PRIMES=10
fi
AC_DEFINE_UNQUOTED(CHECK_PRIMES, $CHECK_PRIMES,
                   The number of prime factors needed for checks)

AC_ARG_VAR(BORED_SORTER, The amount of milliseconds the sorter will sleep)
if test -z "$BORED_SORTER"; then
    BORED_SORTER=2
fi
AC_DEFINE_UNQUOTED(BORED_SORTER, $BORED_SORTER,
                   The amount of milliseconds the sorter will sleep)

AC_ARG_VAR(ASSIGNMENT_MAX, The maximum assignment lengt)
if test -z "$ASSIGNMENT_MAX"; then
    ASSIGNMENT_MAX=10
fi
AC_DEFINE_UNQUOTED(ASSIGNMENT_MAX, $ASSIGNMENT_MAX,
                   The maximum assignment length)

AC_ARG_VAR(THREADED_SORT_LENGTH, The shortest part to sort in own thread)
if test -z "$THREADED_SORT_LENGTH"; then
    THREADED_SORT_LENGTH=10
fi
AC_DEFINE_UNQUOTED(THREADED_SORT_LENGTH, $THREADED_SORT_LENGTH,
                   The shortest part to sort in own thread)

AC_ARG_VAR(ASSIGNMENT_END, The largest number to test)
if test -z "$ASSIGNMENT_END"; then
    ASSIGNMENT_END=615
fi
AC_DEFINE_UNQUOTED(ASSIGNMENT_END, $ASSIGNMENT_END,
                   The largest number to test)

# The debug option
AC_HEADER_ASSERT
AC_HEADER_TRACES





# Checks for programs.
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11

# Checks for libraries
AX_PTHREAD

LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

# Checks for header files.


# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([pow sqrt])

AC_OUTPUT
