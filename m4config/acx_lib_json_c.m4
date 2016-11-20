#
# lldp_CHECK_JSONC
#

AC_DEFUN([ACX_LIB_JSON_C], [
    PKG_CHECK_MODULES([JSONC], [json-c], [
        AC_SUBST([JSONC_LIBS])
        AC_SUBST([JSONC_CFLAGS])
        AC_DEFINE_UNQUOTED([USE_JSON], 1, [Define to indicate to enable JSON support])
        AC_DEFINE_UNQUOTED([USE_JSONC], 1, [Define to indicate to enable JSON via json-c support])
        with_json=json-c
        LIBS="$JSONC_LIBS $LIBS"
        CFALGS="$JSONC_CFLAGS $CFLAGS"
        AC_MSG_RESULT([JSON-C Library is found])
    ],[
        PKG_CHECK_MODULES([JSONC], [json >= 0.10], [
            AC_SUBST([JSONC_LIBS])
            AC_SUBST([JSONC_CFLAGS])
            AC_DEFINE_UNQUOTED([USE_JSON], 1, [Define to indicate to enable JSON support])
            AC_DEFINE_UNQUOTED([USE_JSONC], 1, [Define to indicate to enable JSON via json-c support])
            with_json=json-c
            LIBS="$JSONC_LIBS $LIBS"
            CFALGS="$JSONC_CFLAGS $CFLAGS"
            AC_MSG_ERROR([MQTT Paho C Client Library is not installed!])
        ],[
            if test x"$with_json" = x"json-c"; then
                AC_MSG_ERROR([*** unable to find json-c])
            fi
            with_json=no
        ])
    ])
])
