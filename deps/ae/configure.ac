AC_PREREQ([2.68])
AC_INIT([libae], [1.0], [opensource@zhihu.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([ae.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_MODE_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostbyname gettimeofday inet_ntoa memset select socket strchr strerror kqueue])

haveepoll=no
AC_CHECK_FUNCS(epoll_ctl, [haveepoll=yes], )
if test "x$haveepoll" = "xyes" ; then
  AC_DEFINE(HAVE_EPOLL, 1, [Define if your system supports the epoll system calls])
fi

haveprocfs=no
AC_CHECK_FILE(/proc, [haveprocfs=yes], )
if test "x$haveprocfs" = "xyes" ; then
  AC_DEFINE(HAVE_PROCFS, 1, [Define if your system supports the /proc file system])
fi

havetaskinfo=no
AC_CHECK_FUNCS(task_info, [havetaskinfo=yes], )
if test "x$havetaskinfo" = "xyes" ; then
  AC_DEFINE(HAVE_TASKINFO, 1, [Define if your system supports the task_info function])
fi

haveprocstat=no
AC_CHECK_FILE(/proc/stat, [haveprocstat=yes], )
if test "x$haveprocstat" = "xyes" ; then
  AC_DEFINE(HAVE_PROC_STAT,1,[Define to 1 if file '/proc/stat' should be used.])
fi

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
