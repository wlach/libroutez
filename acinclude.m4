# ===========================================================================
#              http://autoconf-archive.cryp.to/ax_with_prog.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_WITH_PROG([VARIABLE],[program],[VALUE-IF-NOT-FOUND],[PATH])
#
# DESCRIPTION
#
#   Locates an installed program binary, placing the result in the precious
#   variable VARIABLE. Accepts a present VARIABLE, then --with-program, and
#   failing that searches for program in the given path (which defaults to
#   the system path). If program is found, VARIABLE is set to the full path
#   of the binary; if it is not found VARIABLE is set to VALUE-IF-NOT-FOUND
#   if provided, unchanged otherwise.
#
#   A typical example could be the following one:
#
#         AX_WITH_PROG(PERL,perl)
#
#   NOTE: This macro is based upon the original AX_WITH_PYTHON macro from
#   Dustin J. Mitchell <dustin@cs.uchicago.edu>.
#
# LAST MODIFICATION
#
#   2008-05-05
#
# COPYLEFT
#
#   Copyright (c) 2008 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#   Copyright (c) 2008 Dustin J. Mitchell <dustin@cs.uchicago.edu>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_WITH_PROG],[
    AC_PREREQ([2.61])

    pushdef([VARIABLE],$1)
    pushdef([EXECUTABLE],$2)
    pushdef([VALUE_IF_NOT_FOUND],$3)
    pushdef([PATH_PROG],$4)

    AC_ARG_VAR(VARIABLE,Absolute path to EXECUTABLE executable)

    AS_IF(test -z "$VARIABLE",[
    	AC_MSG_CHECKING(whether EXECUTABLE executable path has been provided)
        AC_ARG_WITH(EXECUTABLE,AS_HELP_STRING([--with-EXECUTABLE=[[[[PATH]]]]],absolute path to EXECUTABLE executable), [
	    AS_IF([test "$withval" != "yes"],[
	        VARIABLE="$withval"
		AC_MSG_RESULT($VARIABLE)
	    ],[
		VARIABLE=""
	        AC_MSG_RESULT([no])
	    ])
	],[
	    AC_MSG_RESULT([no])
	])

        AS_IF(test -z "$VARIABLE",[
	    AC_PATH_PROG([]VARIABLE[],[]EXECUTABLE[],[]VALUE_IF_NOT_FOUND[],[]PATH_PROG[])
        ])
    ])

    popdef([PATH_PROG])
    popdef([VALUE_IF_NOT_FOUND])
    popdef([EXECUTABLE])
    popdef([VARIABLE])
])

# ===========================================================================
#               http://autoconf-archive.cryp.to/ax_python.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_PYTHON
#
# DESCRIPTION
#
#   This macro does a complete Python development environment check.
#
#   It recurses through several python versions (from 2.1 to 2.5 in this
#   version), looking for an executable. When it finds an executable, it
#   looks to find the header files and library.
#
#   It sets PYTHON_BIN to the name of the python executable,
#   PYTHON_INCLUDE_DIR to the directory holding the header files, and
#   PYTHON_LIB to the name of the Python library.
#
#   This macro calls AC_SUBST on PYTHON_BIN (via AC_CHECK_PROG),
#   PYTHON_INCLUDE_DIR and PYTHON_LIB.
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Michael Tindal
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Macro Archive. When you make and
#   distribute a modified version of the Autoconf Macro, you may extend this
#   special exception to the GPL to apply to your modified version as well.

AC_DEFUN([AX_PYTHON],
[AC_MSG_CHECKING(for python build information)
AC_MSG_RESULT([])
for python in python2.6 python2.5 python2.4 python2.3 python2.2 python2.1 python; do
AC_CHECK_PROGS(PYTHON_BIN, [$python])
ax_python_bin=$PYTHON_BIN
if test x$ax_python_bin != x; then
   AC_CHECK_LIB($ax_python_bin, main, ax_python_lib=$ax_python_bin, ax_python_lib=no)
   AC_CHECK_HEADER([$ax_python_bin/Python.h],
   [[ax_python_header=`locate $ax_python_bin/Python.h | sed -e s,/Python.h,, | tail -1`]],
   ax_python_header=no)
   if test "$ax_python_lib" != no; then
     if test "$ax_python_header" != no; then
       break;
     fi
   fi
fi
done
if test "x$ax_python_bin" = x; then
   ax_python_bin=no
fi
if test "x$ax_python_header" = x; then
   ax_python_header=no
fi
if test "x$ax_python_lib" = x; then
   ax_python_lib=no
fi

AC_MSG_RESULT([  results of the Python check:])
AC_MSG_RESULT([    Binary:      $ax_python_bin])
AC_MSG_RESULT([    Library:     $ax_python_lib])
AC_MSG_RESULT([    Include Dir: $ax_python_header])

if test "x$ax_python_header" != xno; then
  PYTHON_INCLUDE_DIR=$ax_python_header
  AC_SUBST(PYTHON_INCLUDE_DIR)
fi
if test "x$ax_python_lib" != xno; then
  PYTHON_LIB=$ax_python_lib
  AC_SUBST(PYTHON_LIB)
fi
])dnl

# ===========================================================================
#              http://autoconf-archive.cryp.to/ax_with_ruby.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_WITH_RUBY([VALUE-IF-NOT-FOUND],[PATH])
#
# DESCRIPTION
#
#   Locates an installed Ruby binary, placing the result in the precious
#   variable $RUBY. Accepts a present $RUBY, then --with-ruby, and failing
#   that searches for ruby in the given path (which defaults to the system
#   path). If ruby is found, $RUBY is set to the full path of the binary; if
#   it is not found $RUBY is set to VALUE-IF-NOT-FOUND if provided,
#   unchanged otherwise.
#
#   A typical use could be the following one:
#
#         AX_WITH_RUBY
#
# LAST MODIFICATION
#
#   2008-05-05
#
# COPYLEFT
#
#   Copyright (c) 2008 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_WITH_RUBY],[
    AX_WITH_PROG(RUBY,ruby,$1,$2)
])

# ===========================================================================
#           http://autoconf-archive.cryp.to/ax_compare_version.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_COMPARE_VERSION(VERSION_A, OP, VERSION_B, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   This macro compares two version strings. Due to the various number of
#   minor-version numbers that can exist, and the fact that string
#   comparisons are not compatible with numeric comparisons, this is not
#   necessarily trivial to do in a autoconf script. This macro makes doing
#   these comparisons easy.
#
#   The six basic comparisons are available, as well as checking equality
#   limited to a certain number of minor-version levels.
#
#   The operator OP determines what type of comparison to do, and can be one
#   of:
#
#    eq  - equal (test A == B)
#    ne  - not equal (test A != B)
#    le  - less than or equal (test A <= B)
#    ge  - greater than or equal (test A >= B)
#    lt  - less than (test A < B)
#    gt  - greater than (test A > B)
#
#   Additionally, the eq and ne operator can have a number after it to limit
#   the test to that number of minor versions.
#
#    eq0 - equal up to the length of the shorter version
#    ne0 - not equal up to the length of the shorter version
#    eqN - equal up to N sub-version levels
#    neN - not equal up to N sub-version levels
#
#   When the condition is true, shell commands ACTION-IF-TRUE are run,
#   otherwise shell commands ACTION-IF-FALSE are run. The environment
#   variable 'ax_compare_version' is always set to either 'true' or 'false'
#   as well.
#
#   Examples:
#
#     AX_COMPARE_VERSION([3.15.7],[lt],[3.15.8])
#     AX_COMPARE_VERSION([3.15],[lt],[3.15.8])
#
#   would both be true.
#
#     AX_COMPARE_VERSION([3.15.7],[eq],[3.15.8])
#     AX_COMPARE_VERSION([3.15],[gt],[3.15.8])
#
#   would both be false.
#
#     AX_COMPARE_VERSION([3.15.7],[eq2],[3.15.8])
#
#   would be true because it is only comparing two minor versions.
#
#     AX_COMPARE_VERSION([3.15.7],[eq0],[3.15])
#
#   would be true because it is only comparing the lesser number of minor
#   versions of the two values.
#
#   Note: The characters that separate the version numbers do not matter. An
#   empty string is the same as version 0. OP is evaluated by autoconf, not
#   configure, so must be a string, not a variable.
#
#   The author would like to acknowledge Guido Draheim whose advice about
#   the m4_case and m4_ifvaln functions make this macro only include the
#   portions necessary to perform the specific comparison specified by the
#   OP argument in the final configure script.
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Tim Toolan <toolan@ele.uri.edu>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

dnl #########################################################################
AC_DEFUN([AX_COMPARE_VERSION], [
  AC_PROG_AWK

  # Used to indicate true or false condition
  ax_compare_version=false

  # Convert the two version strings to be compared into a format that
  # allows a simple string comparison.  The end result is that a version
  # string of the form 1.12.5-r617 will be converted to the form
  # 0001001200050617.  In other words, each number is zero padded to four
  # digits, and non digits are removed.
  AS_VAR_PUSHDEF([A],[ax_compare_version_A])
  A=`echo "$1" | sed -e 's/\([[0-9]]*\)/Z\1Z/g' \
                     -e 's/Z\([[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/[[^0-9]]//g'`

  AS_VAR_PUSHDEF([B],[ax_compare_version_B])
  B=`echo "$3" | sed -e 's/\([[0-9]]*\)/Z\1Z/g' \
                     -e 's/Z\([[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/[[^0-9]]//g'`

  dnl # In the case of le, ge, lt, and gt, the strings are sorted as necessary
  dnl # then the first line is used to determine if the condition is true.
  dnl # The sed right after the echo is to remove any indented white space.
  m4_case(m4_tolower($2),
  [lt],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort -r | sed "s/x${A}/false/;s/x${B}/true/;1q"`
  ],
  [gt],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort | sed "s/x${A}/false/;s/x${B}/true/;1q"`
  ],
  [le],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort | sed "s/x${A}/true/;s/x${B}/false/;1q"`
  ],
  [ge],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort -r | sed "s/x${A}/true/;s/x${B}/false/;1q"`
  ],[
    dnl Split the operator from the subversion count if present.
    m4_bmatch(m4_substr($2,2),
    [0],[
      # A count of zero means use the length of the shorter version.
      # Determine the number of characters in A and B.
      ax_compare_version_len_A=`echo "$A" | $AWK '{print(length)}'`
      ax_compare_version_len_B=`echo "$B" | $AWK '{print(length)}'`

      # Set A to no more than B's length and B to no more than A's length.
      A=`echo "$A" | sed "s/\(.\{$ax_compare_version_len_B\}\).*/\1/"`
      B=`echo "$B" | sed "s/\(.\{$ax_compare_version_len_A\}\).*/\1/"`
    ],
    [[0-9]+],[
      # A count greater than zero means use only that many subversions
      A=`echo "$A" | sed "s/\(\([[0-9]]\{4\}\)\{m4_substr($2,2)\}\).*/\1/"`
      B=`echo "$B" | sed "s/\(\([[0-9]]\{4\}\)\{m4_substr($2,2)\}\).*/\1/"`
    ],
    [.+],[
      AC_WARNING(
        [illegal OP numeric parameter: $2])
    ],[])

    # Pad zeros at end of numbers to make same length.
    ax_compare_version_tmp_A="$A`echo $B | sed 's/./0/g'`"
    B="$B`echo $A | sed 's/./0/g'`"
    A="$ax_compare_version_tmp_A"

    # Check for equality or inequality as necessary.
    m4_case(m4_tolower(m4_substr($2,0,2)),
    [eq],[
      test "x$A" = "x$B" && ax_compare_version=true
    ],
    [ne],[
      test "x$A" != "x$B" && ax_compare_version=true
    ],[
      AC_WARNING([illegal OP parameter: $2])
    ])
  ])

  AS_VAR_POPDEF([A])dnl
  AS_VAR_POPDEF([B])dnl

  dnl # Execute ACTION-IF-TRUE / ACTION-IF-FALSE.
  if test "$ax_compare_version" = "true" ; then
    m4_ifvaln([$4],[$4],[:])dnl
    m4_ifvaln([$5],[else $5])dnl
  fi
]) dnl AX_COMPARE_VERSION


# ===========================================================================
#          http://autoconf-archive.cryp.to/ax_prog_ruby_version.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_PROG_RUBY_VERSION([VERSION],[ACTION-IF-TRUE],[ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   Makes sure that ruby supports the version indicated. If true the shell
#   commands in ACTION-IF-TRUE are executed. If not the shell commands in
#   ACTION-IF-FALSE are run. Note if $RUBY is not set (for example by
#   running AC_CHECK_PROG or AC_PATH_PROG),
#
#   Example:
#
#     AC_PATH_PROG([RUBY],[ruby])
#     AC_PROG_RUBY_VERSION([1.8.0],[ ... ],[ ... ])
#
#   This will check to make sure that the ruby you have supports at least
#   version 1.6.0.
#
#   NOTE: This macro uses the $RUBY variable to perform the check.
#   AX_WITH_RUBY can be used to set that variable prior to running this
#   macro. The $RUBY_VERSION variable will be valorized with the detected
#   version.
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_PROG_RUBY_VERSION],[
    AC_REQUIRE([AC_PROG_SED])
    AC_REQUIRE([AC_PROG_GREP])

    AS_IF([test -n "$RUBY"],[
        ax_ruby_version="$1"

        AC_MSG_CHECKING([for ruby version])
        changequote(<<,>>)
        ruby_version=`$RUBY --version 2>&1 | $GREP "^ruby " | $SED -e 's/^.* \([0-9]*\.[0-9]*\.[0-9]*\) .*/\1/'`
        changequote([,])
        AC_MSG_RESULT($ruby_version)

	AC_SUBST([RUBY_VERSION],[$ruby_version])

        AX_COMPARE_VERSION([$ax_ruby_version],[le],[$ruby_version],[
	    :
            $2
        ],[
	    :
            $3
        ])
    ],[
        AC_MSG_WARN([could not find the ruby interpreter])
        $3
    ])
])

# ===========================================================================
#             http://autoconf-archive.cryp.to/ax_ruby_devel.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_RUBY_DEVEL([version])
#
# DESCRIPTION
#
#   This macro checks for Ruby and tries to get the include path to
#   'ruby.h'. It provides the $(RUBY_CPPFLAGS) and $(RUBY_LDFLAGS) output
#   variables. It also exports $(RUBY_EXTRA_LIBS) for embedding Ruby in your
#   code.
#
#   You can search for some particular version of Ruby by passing a
#   parameter to this macro, for example "1.8.6".
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Rafal Rzepecki <divided.mind@gmail.com>
#   Copyright (c) 2008 Sebastian Huber <sebastian-huber@web.de>
#   Copyright (c) 2008 Alan W. Irwin <irwin@beluga.phys.uvic.ca>
#   Copyright (c) 2008 Rafael Laboissiere <rafael@laboissiere.net>
#   Copyright (c) 2008 Andrew Collier <colliera@ukzn.ac.za>
#   Copyright (c) 2008 Matteo Settenvini <matteo@member.fsf.org>
#   Copyright (c) 2008 Horst Knorr <hk_classes@knoda.org>
#
#   This program is free software: you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation, either version 3 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Macro Archive. When you make and
#   distribute a modified version of the Autoconf Macro, you may extend this
#   special exception to the GPL to apply to your modified version as well.

AC_DEFUN([AX_RUBY_DEVEL],[
    AC_REQUIRE([AX_WITH_RUBY])
    AS_IF([test -n "$1"], [AX_PROG_RUBY_VERSION([$1])])

    #
    # Check if you have mkmf, else fail
    #
    AC_MSG_CHECKING([for the mkmf Ruby package])
    ac_mkmf_result=`$RUBY -rmkmf -e ";" 2>&1`
    if test -z "$ac_mkmf_result"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([cannot import Ruby module "mkmf".
Please check your Ruby installation. The error was:
$ac_distutils_result])
    fi

    #
    # Check for Ruby include path
    #
    AC_MSG_CHECKING([for Ruby include path])
    if test -z "$RUBY_CPPFLAGS"; then
        ruby_path=`$RUBY -rmkmf -e 'print Config::CONFIG[["archdir"]]'`
        if test -n "${ruby_path}"; then
                ruby_path="-I$ruby_path"
        fi
        RUBY_CPPFLAGS=$ruby_path
    fi
    AC_MSG_RESULT([$RUBY_CPPFLAGS])
    AC_SUBST([RUBY_CPPFLAGS])

    #
    # Check for Ruby library path
    #
    AC_MSG_CHECKING([for Ruby library path])
    if test -z "$RUBY_LDFLAGS"; then
        RUBY_LDFLAGS=`$RUBY -rmkmf -e 'print Config::CONFIG[["LIBRUBYARG_SHARED"]]'`
    fi
    AC_MSG_RESULT([$RUBY_LDFLAGS])
    AC_SUBST([RUBY_LDFLAGS])

    #
    # Check for site packages
    #
    AC_MSG_CHECKING([for Ruby site-packages path])
    if test -z "$RUBY_SITE_PKG"; then
        RUBY_SITE_PKG=`$RUBY -rmkmf -e 'print Config::CONFIG[["sitearchdir"]]'`
    fi
    AC_MSG_RESULT([$RUBY_SITE_PKG])
    AC_SUBST([RUBY_SITE_PKG])

    #
    # libraries which must be linked in when embedding
    #
    AC_MSG_CHECKING(ruby extra libraries)
    if test -z "$RUBY_EXTRA_LIBS"; then
       RUBY_EXTRA_LIBS=`$RUBY -rmkmf -e 'print Config::CONFIG[["SOLIBS"]]'`
    fi
    AC_MSG_RESULT([$RUBY_EXTRA_LIBS])
    AC_SUBST(RUBY_EXTRA_LIBS)

    #
    # linking flags needed when embedding
    # (is it even needed for Ruby?)
    #
    # AC_MSG_CHECKING(ruby extra linking flags)
    # if test -z "$RUBY_EXTRA_LDFLAGS"; then
    # RUBY_EXTRA_LDFLAGS=`$RUBY -rmkmf -e 'print Config::CONFIG[["LINKFORSHARED"]]'`
    # fi
    # AC_MSG_RESULT([$RUBY_EXTRA_LDFLAGS])
    # AC_SUBST(RUBY_EXTRA_LDFLAGS)

    # this flags breaks ruby.h, and is sometimes defined by KDE m4 macros
    CFLAGS="`echo "$CFLAGS" | sed -e 's/-std=iso9899:1990//g;'`"
    #
    # final check to see if everything compiles alright
    #
    AC_MSG_CHECKING([consistency of all components of ruby development environment])
    AC_LANG_PUSH([C])
    # save current global flags
    ac_save_LIBS="$LIBS"
    LIBS="$ac_save_LIBS $RUBY_LDFLAGS"
    ac_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$ac_save_CPPFLAGS $RUBY_CPPFLAGS"
    AC_TRY_LINK([
        #include <ruby.h>
    ],[
        ruby_init();
    ],[rubyexists=yes],[rubyexists=no])

    AC_MSG_RESULT([$rubyexists])

    if test ! "$rubyexists" = "yes"; then
       AC_MSG_ERROR([
  Could not link test program to Ruby. Maybe the main Ruby library has been
  installed in some non-standard library path. If so, pass it to configure,
  via the LDFLAGS environment variable.
  Example: ./configure LDFLAGS="-L/usr/non-standard-path/ruby/lib"
  ============================================================================
   ERROR!
   You probably have to install the development version of the Ruby package
   for your distribution.  The exact name of this package varies among them.
  ============================================================================
       ])
      RUBY_VERSION=""
    fi
    AC_LANG_POP
    # turn back to default flags
    CPPFLAGS="$ac_save_CPPFLAGS"
    LIBS="$ac_save_LIBS"

    #
    # all done!
    #
])
