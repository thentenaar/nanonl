dnl SYNOPSIS
dnl
dnl AX_CHECK_DEP - Check for a dependant library
dnl
dnl USAGE
dnl
dnl AX_CHECK_DEP(dep, header, library, function, nonfatal, otherlibs)
dnl
dnl PARAMETERS
dnl
dnl dep       - Name of the dependency
dnl header    - Header file to look for
dnl library   - Library to look for
dnl function  - Function to look for within the library
dnl nonfatal  - If set, don't abort if this dep is missing
dnl otherlibs - Other needed libraries
dnl
dnl DESCRIPTION
dnl
dnl This macro sets `have_DEP` to either "yes" or "no" to indicate
dnl the presence of the lib, and appends the necessary options
dnl to CPPFLAGS, LDFLAGS, and LIBS.
dnl
dnl `have_DEP` is also substituted in the output.
dnl
dnl Example
dnl
dnl AX_CHECK_DEP([libx], [x.h], [x], [x_func])
dnl
AC_DEFUN([AX_CHECK_DEP],[
	pushdef([DEP], translit([$1], [A-Z], [a-z]))dnl

	have_[]DEP=no
	AC_ARG_WITH(DEP[],
		[AS_HELP_STRING(
			[--with-]DEP[=PATH],
			[set the path for ]DEP[])],
		[with_]DEP[=$withval],
		[with_]DEP[=yes]
	)

	AS_IF([test "$with_[]DEP" != "no"],[
		have_[]DEP=yes
		save_cppflags=$CPPFLAGS
		save_ldflags=$LDFLAGS
		save_libs=$LIBS

		dnl Check for the given header ($2)
		AS_IF([test "$with_[]DEP" == "yes"],[with_]DEP[=$prefix])
		AS_IF([test "$with_[]DEP" != "$prefix"],[
			CPPFLAGS="$CPPFLAGS -I$with_[]DEP[]/include"
		])
		AC_CHECK_HEADER([$2], [], [have_]DEP[=no])

		dnl Check for the given lib ($3 / $4)
		AS_IF([test "$have_[]DEP" != "no"],[
			AS_IF([test "$with_[]DEP" != "$prefix"],[
				LDFLAGS="$LDFLAGS -L$with_[]DEP[]/lib"
			])
			AC_CHECK_LIB([$3], [$4], [], [have_]DEP[=no], [$6])
		])

		AS_IF([test "$have_[]DEP" == "no"],[
			CPPFLAGS=$save_cppflags
			LDFLAGS=$save_ldflags
			LIBS=$save_libs
			AS_IF([test "x$5" != "x"],[],
				  [AC_MSG_FAILURE([DEP not found.])
			])
		],[
			LIBS_[]DEP[]="$LIBS$6"
			AS_IF([test "x$with_[]DEP" != "x$prefix" ],[
				AS_IF([test "x$RPATHS" != "x" ], [
					RPATH="$RPATH:$with_[]DEP[/lib]"
				],[
					RPATH="$with_[]DEP[/lib]"
				])
			])
		])
	])

	AC_SUBST([LIBS_]DEP[])
	AC_SUBST([have_]DEP[])
	popdef([DEP])
]) dnl AX_CHECK_DEP
