#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT(src/hybris.cpp,0.7.1)
AM_INIT_AUTOMAKE(hybris,0.7.1)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

LEX=flex
YACC=bison
# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_global_init])
AC_CHECK_LIB([dl], [dlsym])
AC_CHECK_LIB([pcrecpp], [CASELESS])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([xml2], [xmlReadFile])

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h inttypes.h libintl.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([bzero floor getcwd gethostbyname gettimeofday memset pow socket sqrt strchr strdup strrchr strtol])

AC_CONFIG_FILES([Makefile
                 src/builtins/Makefile])
CPPFLAGS=''
CXXFLAGS=''
CFLAGS=''
AC_OUTPUT
