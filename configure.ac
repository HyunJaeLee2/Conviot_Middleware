#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(conviot_middleware, 0.1, hyunjae.lee410@gmail.com)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4config])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()
CFLAGS="$CFLAGS -Wall -g"

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AX_LIB_MYSQL
AX_PTHREAD
ACX_LIB_PAHO_MQTT
ACX_LIB_JSON_C

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lconfig':
AC_CHECK_LIB([config], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h limits.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([bzero clock_gettime gettimeofday localtime_r memmove memset realpath select socket strdup strerror strtol])

AC_CONFIG_FILES([Makefile
                 src/common/Makefile
                 src/common/capcommon/Makefile
                 src/middleware/Makefile])
AC_OUTPUT
