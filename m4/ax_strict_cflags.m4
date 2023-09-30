dnl SYNOPSIS
dnl
dnl AX_STRICT_CFLAGS - Set strict compiler flags
dnl
dnl DESCRIPTION
dnl
dnl This macro checks a number of warning options for GCC and
dnl Clang, and adds supoprted options to CFLAGS. -Werror doesn't
dnl get added here, simply because having -Werror and -Wstrict-prototypes
dnl will cause compilations from autoconf to fail.
dnl
dnl COVERAGE
dnl
dnl This macro exports ax_cc_gcov_command as the proper gcov command
dnl to invoke (i.e. llvm-cov gcov on clang.)
dnl

AC_DEFUN([AX_STRICT_CFLAGS],[
	AX_REQUIRE_DEFINED([AX_APPEND_COMPILE_FLAGS])
	AX_REQUIRE_DEFINED([AX_CHECK_COMPILE_FLAG])
	AC_LANG_PUSH([C])

	dnl Clang uses -Q, GCC doesn't support it.
	AX_CHECK_COMPILE_FLAG([-Werror -Qunused-arguments],dnl
	                      [ax_cc_clang="yes"],dnl
	                      [ax_cc_clang="no"])

	AS_IF([test "$ax_cc_clang" == "yes"],[
		ax_cc_gcov_command="llvm-cov gcov"
		AX_APPEND_COMPILE_FLAGS([ dnl
			-pedantic dnl
			-Weverything dnl
			-Wno-padded dnl
			-Wno-switch-enum dnl
			-Wno-missing-variable-declarations dnl
			-Wno-disabled-macro-expansion dnl
			-Wno-format-nonliteral dnl
			-Wno-long-long dnl
			-Wno-format dnl
			-Wno-extra-semi-stmt dnl
			-Wno-documentation dnl
			-Qunused-arguments dnl
		])
	],[	dnl Assume GCC
		ax_cc_gcov_command="gcov"
		AX_APPEND_COMPILE_FLAGS([ dnl
			-pedantic dnl
			-Wall dnl
			-W dnl
			-Wconversion dnl
			-Wstrict-prototypes dnl
			-Wmissing-prototypes dnl
			-Wmissing-declarations dnl
			-Wnested-externs dnl
			-Wshadow dnl
			-Wcast-align dnl
			-Wwrite-strings dnl
			-Wcomment dnl
			-Wcast-qual dnl
			-Wredundant-decls dnl
			-Wbad-function-cast dnl
			-Wno-variadic-macros dnl
		])

		dnl -Wformat-security was included in gcc 3.0.4
		AX_APPEND_COMPILE_FLAGS([-Wformat-security])

		dnl -Wc90-c99-compat was introduced in gcc 5
		AX_APPEND_COMPILE_FLAGS([-Wc90-c99-compat])
	])

	AC_SUBST([ax_cc_gcov_command])
	AC_LANG_POP([C])
]) dnl AX_STRICT_CFLAGS
