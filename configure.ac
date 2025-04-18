dnl nanonl: Netlink helper library
dnl Copyright (C) 2015 - 2025 Tim Hentenaar.
dnl
dnl This code is Licensed under the Simplified BSD License.
dnl See the LICENSE file for details.
dnl
AC_PREREQ([2.72])
AC_INIT([nanonl], [1.0], [http://github.com/thentenaar/nanonl])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules])
AM_SILENT_RULES([yes])
LT_INIT
AC_LANG([C])

dnl Check for the C compiler, etc.
AC_PROG_CC
AC_CHECK_HEADERS([errno.h linux/netlink.h netinet/in.h])

dnl Check the host system type, and compiler characteristics
AC_CANONICAL_HOST
AC_C_CONST
AC_TYPE_SIZE_T

dnl Check for check
AX_CHECK_DEP([check], [check.h], [check], [srunner_run_all], [nonfatal])
AS_IF([test "x$have_check" != "xyes"],[
	AX_CHECK_DEP([check], [check.h], [check], [suite_create], [nonfatal], [-lsubunit -lm])
])
AM_CONDITIONAL([HAVE_CHECK], [test "x$have_check" == "xyes"])

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

dnl Enable conntrack support (implies netfilter)
AC_ARG_ENABLE([conntrack],
	[AS_HELP_STRING(
		[--enable-conntrack],
		[enable conntrack support (implies netfilter)])
	]
)
AM_CONDITIONAL([NL_CONNTRACK], [test "x$enable_conntrack" == "xyes"])
AS_IF([test "x$enable_conntrack" == "xyes"],[
	AM_CONDITIONAL([NL_NETFILTER], [true])
])

dnl Enable ifinfo support
AC_ARG_ENABLE([ifinfo],
	[AS_HELP_STRING(
		[--enable-ifinfo],
		[enable interface info support])
	]
)
AM_CONDITIONAL([NL_IFINFO], [test "x$enable_ifinfo" == "xyes"])

dnl Enable ifaddr support
AC_ARG_ENABLE([ifaddr],
	[AS_HELP_STRING(
		[--enable-ifaddr],
		[enable interface address support])
	]
)
AM_CONDITIONAL([NL_IFADDR], [test "x$enable_ifaddr" == "xyes"])

dnl Enable nd support
AC_ARG_ENABLE([nd],
	[AS_HELP_STRING(
		[--enable-nd],
		[enable neighbor discovery support])
	]
)
AM_CONDITIONAL([NL_ND], [test "x$enable_nd" == "xyes"])

dnl Enable support for everything
AC_ARG_ENABLE([all],
	[AS_HELP_STRING(
		[--enable-all],
		[enable support for everything])
	]
)
AS_IF([test "x$enable_all" == "xyes"],[
	AM_CONDITIONAL([NL_ND],        [true])
	AM_CONDITIONAL([NL_IFINFO],    [true])
	AM_CONDITIONAL([NL_IFADDR],    [true])
	AM_CONDITIONAL([NL_CONNTRACK], [true])
	AM_CONDITIONAL([NL_NFQUEUE],   [true])
	AM_CONDITIONAL([NL_NETFILTER], [true])
	AM_CONDITIONAL([NL_GENERIC],   [true])
])

dnl Set-up CFLAGS
LIBS=
AX_STRICT_CFLAGS

AC_CONFIG_FILES([Makefile example/Makefile])
AC_OUTPUT

