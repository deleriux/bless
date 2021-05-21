AC_INIT([bless], [1.0], [matthew@ife.onl])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Wno-extra-portability -Werror subdir-objects])

LT_INIT
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])

AC_SEARCH_LIBS([cap_init], [cap])
AC_SUBST(CAP_LIBS, "-lcap")

AC_SUBST([AM_CPPFLAGS], [-D_GNU_SOURCE])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
