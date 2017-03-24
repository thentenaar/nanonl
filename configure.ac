dnl nanonl: Netlink helper library
dnl Copyright (C) 2015 - 2017 Tim Hentenaar.
dnl
dnl This code is Licensed under the Simplified BSD License.
dnl See the LICENSE file for details.
dnl
AC_PREREQ([2.63])
AC_INIT([nanonl], [1.0], [http://github.com/thentenaar/nanonl])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules])
AM_SILENT_RULES([yes])
LT_INIT
AC_LANG([C])

dnl Check for the C compiler, etc.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h linux/netlink.h])

dnl Check the host system type, and compiler characteristics
AC_CANONICAL_HOST
AC_C_CONST
AC_TYPE_SIZE_T

dnl Enable netlink_generic support
AC_ARG_ENABLE([generic],
	[AS_HELP_STRING(
		[--enable-generic],
		[enable netlink generic support])
	]
)
AM_CONDITIONAL([NL_GENERIC], [test "x$enable_generic" == "xyes"])

dnl Enable nfnetlink support
AC_ARG_ENABLE([netfilter],
	[AS_HELP_STRING(
		[--enable-netfilter],
		[enable nfnetlink support])
	]
)
AM_CONDITIONAL([NL_NETFILTER], [test "x$enable_netfilter" == "xyes"])

dnl Enable nfqueue support (implies netfilter)
AC_ARG_ENABLE([nfqueue],
	[AS_HELP_STRING(
		[--enable-nfqueue],
		[enable nfqueue support (implies netfilter)])
	]
)
AM_CONDITIONAL([NL_NFQUEUE], [test "x$enable_nfqueue" == "xyes"])
AS_IF([test "x$enable_nfqueue" == "xyes"],[
	AM_CONDITIONAL([NL_NETFILTER], [true])
])

dnl Set-up CFLAGS
CFLAGS="$CFLAGS -ansi"
AX_STRICT_CFLAGS

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

