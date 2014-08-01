# AC_HEADER_TRACES
# ----------------
# Check whether to enable traces.
AC_DEFUN_ONCE([AC_HEADER_TRACES],
[
  AC_MSG_CHECKING([whether to enable traces])
  AC_ARG_ENABLE([traces],
    [AS_HELP_STRING([--disable-traces], [turn off traces])],
    [ac_enable_traces=$enableval
     AS_IF(dnl
      [test "x$enableval" = xno],
        [AC_DEFINE([NTRACES], [1],
          [Define to 1 if traces should be disabled.])],
      [test "x$enableval" != xyes],
        [AC_MSG_WARN([invalid argument supplied to --enable-traces])
        ac_enable_traces=yes])],
    [ac_enable_traces=yes])
  AC_MSG_RESULT([$ac_enable_traces])
])

# AC_HEADER_TIMING
# ----------------
# Check whether to enable timing.
AC_DEFUN_ONCE([AC_HEADER_TIMING],
[
  AC_MSG_CHECKING([whether to enable timing])
  AC_ARG_ENABLE([timing],
    [AS_HELP_STRING([--disable-timing], [turn off timing])],
    [ac_enable_timing=$enableval
     AS_IF(dnl
      [test "x$enableval" = xno],
        [AC_DEFINE([NTIMING], [1],
          [Define to 1 if timing should be disabled.])],
      [test "x$enableval" != xyes],
        [AC_MSG_WARN([invalid argument supplied to --enable-timing])
        ac_enable_timing=yes])],
    [ac_enable_timing=yes])
  AC_MSG_RESULT([$ac_enable_timing])
])

