# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(libroutez, 0.1.0, libroutez@googlegroups.com, libroutez)
AC_CONFIG_SRCDIR(lib/tripgraph.cc)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL

# Detect target build environment
AC_CANONICAL_TARGET
case "$target" in
    *-linux*)
        OS="LINUX"
	;;
    *-sunos*|*-solaris*)
	OS="SOLARIS"
	;;
    *-win*)
    	OS="WIN32"
    	;;
    *-apple*)
    	OS="MACOS"
    	;;
    *)
    	OS="OTHER"
	;;
esac

AC_SUBST(OS)

AX_PYTHON_DEVEL
AX_WITH_RUBY
AX_RUBY_DEVEL

AC_CHECK_PROG(SWIG, swig, swig)

if test x"$SWIG" = "x"; then
    AC_MSG_ERROR("swig not found")
fi

if test x"$PYTHON_CPPFLAGS" = "x"; then
    AC_MSG_ERROR("python not found")
fi

if test x"$RUBY_CPPFLAGS" = "x"; then
    AC_MSG_ERROR("ruby not found")
fi

AC_CONFIG_FILES(config.mk)

AC_OUTPUT

