AC_PREREQ(2.61)
m4_include([version.m4])
AC_INIT(kids, KIDS_VERSION_NUMBER, opensource@zhihu.com)
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/kids.cc])
AC_CONFIG_HEADER([config.h])

AC_SUBST(VERSION, KIDS_VERSION_NUMBER)
# Checks for programs.
CXXFLAGS="$CXXFLAGS -std=c++11"
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_CPP
AC_PROG_MKDIR_P
AC_CANONICAL_HOST

# Checks for libraries.
AC_SEARCH_LIBS([pthread_create], [pthread])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h termios.h unistd.h wchar.h wctype.h pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([atexit dup2 getcwd gethostbyname getpagesize gettimeofday inet_ntoa isascii localtime_r memmove memset mkdir munmap regcomp rmdir select socket strcasecmp strchr strdup strerror strncasecmp strrchr strstr strtol strtoull kqueue pthread_create])

AC_ARG_ENABLE([tcmalloc],
              [AS_HELP_STRING([--enable-tcmalloc],
                              [enable linking with tcmalloc @<:@default: no@:>@])],
              [enable_tcmalloc=${enableval}], [enable_tcmalloc=no])

AC_ARG_ENABLE([heap-profile],
              [AS_HELP_STRING([--enable-heap-profile],
                              [enable heap profile using tcmalloc @<:@default: no@:>@])],
              [enable_heap_profile=${enableval}], [enable_heap_profile=no])

AC_ARG_ENABLE([log-debug],
              [AS_HELP_STRING([--enable-log-debug],
                              [enable debug log @<:@default: no@:>@])],
              CPPFLAGS+="-DLOGDEBUG",)

if test "x${enable_heap_profile}" = xyes; then
  enable_tcmalloc=yes
  AC_DEFINE([HEAP_PROFILE], 1, [profile heap using tcmalloc])
  # Check system type
  case "$host_os" in
  *darwin* | *rhapsody* | *macosx*)
      CXXFLAGS="-Wl,-no_pie $CFLAGS"
      ;;
  *)
  ;;
  esac
fi

if test "x${enable_tcmalloc}" = xyes; then
  AC_SEARCH_LIBS([HeapProfilerStart], [tcmalloc])
  AC_DEFINE([TCMALLOC], 1, [linking with tcmalloc])
  CXXFLAGS="$CXXFLAGS -ltcmalloc"
fi

CFLAGS="$CFLAGS -g -Wall"
CXXFLAGS="$CXXFLAGS -g -Wall -Wno-sign-compare -Werror -Wno-deprecated"

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 doc/Makefile
                 test/Makefile])

AC_CONFIG_SUBDIRS([deps/ae])
AC_OUTPUT
AC_MSG_NOTICE([Configuration Summary:
 _  _______ _____   _____
| |/ /_   _|  __ \\ / ____|
| ' /  | | | |  | | (___
|  <   | | | |  | |\\___ \\
| . \\ _| |_| |__| |____) |
|_|\\_\\_____|_____/|_____/

  Operating System:  ${host_os}
  Target directory:  ${prefix}
  TCMalloc:          ${enable_tcmalloc}
  Heap profile:      ${enable_heap_profile}

Type 'make' to build. remember to run 'make test' after build.
send complains and bugs to opensource@zhihu.com
])
