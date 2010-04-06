AC_PREREQ([2.0])
AC_INIT( [Hybris Scripting Language], [1.0 beta 2], [evilsocket@gmail.com], [hybris] )
AM_INIT_AUTOMAKE( hybris, 1.0 beta 2 )
AC_CONFIG_HEADERS([include/config.h])

AC_DEFINE( AUTHOR,   "The Hybris Dev Team [http://www.hybris-lang.org/]", [] )
AC_DEFINE( VERSION,  "1.0 beta 2", [] )
AC_DEFINE( INC_PATH, "/usr/lib/hybris/include/", [system path for hybris global scripts] )
AC_DEFINE( LIB_PATH, "/usr/lib/hybris/library/", [system path for libraries (.so dynamic libs)] )

LEX=flex
YACC=bison
# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for feature disable
AC_ARG_ENABLE( gc, 
    AS_HELP_STRING([--disable-gc], [Disable garbage collector support]),
    [ ac_disable_gc=yes ], [] )
 
AC_ARG_ENABLE( boundaries, 
    AS_HELP_STRING([--disable-boundaries], [Disable boundaries check]),
    [ ac_disable_bc=yes ], [] )

AC_ARG_ENABLE( memdebug, 
    AS_HELP_STRING([--enable-memdebug], [Enable memory debugging messages]),
    [ ac_enble_mdbg=yes ], [] )

if test "x${ac_disable_gc}" != xyes; then
	AC_DEFINE( GC_SUPPORT, 1, [basic garbage collection] )
fi
if test "x${ac_disable_bc}" != xyes; then
	AC_DEFINE( BOUNDS_CHECK, 1, [security check on bounds for array/map/matrix access] )
fi
if test "x${ac_enble_mdbg}" == xyes; then
	AC_DEFINE( MEM_DEBUG, 1, [memory allocation debug messages] )
fi

# Checks for libraries.
AC_CHECK_LIB( [dl],  [dlsym],        [], [AC_MSG_ERROR([libdl is missing])] ) 
AC_DEFINE( PCRE_SUPPORT, 1, [support for regular expressions] )
AC_CHECK_LIB( [pcrecpp], [pcre_compile], [], [AC_MSG_ERROR([libpcrecpp is missing])] ) 
AC_DEFINE( MT_SUPPORT, 1, [multi threading support] )
AC_CHECK_LIB( [pthread], [pthread_create], [], [AC_MSG_ERROR([libpthread is missing])] )
AC_CHECK_HEADERS( [pthread.h], [], [AC_MSG_ERROR([pthread.h header missing])] )
# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_RESOLV
AC_CHECK_HEADERS( [arpa/inet.h], [], [AC_MSG_ERROR([arpa/inet.h header missing])] )
AC_CHECK_HEADERS( [assert.h], [], [AC_MSG_ERROR([assert.h header missing])] )
AC_CHECK_HEADERS( [dirent.h], [], [AC_MSG_ERROR([dirent.h header missing])] )
AC_CHECK_HEADERS( [dlfcn.h], [], [AC_MSG_ERROR([dlfcn.h header missing])] )
AC_CHECK_HEADERS( [errno.h], [], [AC_MSG_ERROR([errno.h header missing])] )
AC_CHECK_HEADERS( [math.h], [], [AC_MSG_ERROR([math.h header missing])] )
AC_CHECK_HEADERS( [netdb.h], [], [AC_MSG_ERROR([netdb.h header missing])] )
AC_CHECK_HEADERS( [netinet/in.h], [], [AC_MSG_ERROR([netinet/in.h header missing])] )
AC_CHECK_HEADERS( [resolv.h], [], [AC_MSG_ERROR([resolv.h header missing])] )
AC_CHECK_HEADERS( [setjmp.h], [], [AC_MSG_ERROR([setjmp.h header missing])] )
AC_CHECK_HEADERS( [signal.h], [], [AC_MSG_ERROR([signal.h header missing])] )
AC_CHECK_HEADERS( [stdarg.h], [], [AC_MSG_ERROR([stdarg.h header missing])] )
AC_CHECK_HEADERS( [stdio.h], [], [AC_MSG_ERROR([stdio.h header missing])] )
AC_CHECK_HEADERS( [stdlib.h], [], [AC_MSG_ERROR([stdlib.h header missing])] )
AC_CHECK_HEADERS( [string.h], [], [AC_MSG_ERROR([string.h header missing])] )
AC_CHECK_HEADERS( [sys/socket.h], [], [AC_MSG_ERROR([sys/socket.h header missing])] )
AC_CHECK_HEADERS( [sys/time.h], [], [AC_MSG_ERROR([sys/time.h header missing])] )
AC_CHECK_HEADERS( [sys/types.h], [], [AC_MSG_ERROR([sys/types.h header missing])] )
AC_CHECK_HEADERS( [sys/wait.h], [], [AC_MSG_ERROR([sys/wait.h header missing])] )
AC_CHECK_HEADERS( [time.h], [], [AC_MSG_ERROR([time.h header missing])] )
AC_CHECK_HEADERS( [unistd.h], [], [AC_MSG_ERROR([unistd.h header missing])] )
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

AC_CONFIG_FILES( [Makefile stdlib/Makefile])

AC_OUTPUT
