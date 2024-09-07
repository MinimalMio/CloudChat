AC_INIT([cloudchat], [1.0])
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_SUBST([CFLAGS], ["-Wall"])
AC_SUBST([LIBS], ["-lpthread"])

AC_ARG_ENABLE([client],
  [AS_HELP_STRING([--enable-client], [Build the client])],
  [enable_client=yes],
  [enable_client=no])

AC_ARG_ENABLE([server],
  [AS_HELP_STRING([--enable-server], [Build the server])],
  [enable_server=yes],
  [enable_server=no])

if test "x$enable_client" = "xyes"; then
  AC_CONFIG_FILES([client/Makefile])
fi

if test "x$enable_server" = "xyes"; then
  AC_CONFIG_FILES([server/Makefile])
fi

AC_OUTPUT

