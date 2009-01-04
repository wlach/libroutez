# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(libroutez)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AX_PYTHON
AX_WITH_RUBY
AX_RUBY_DEVEL

AC_CHECK_PROG(SWIG, swig, swig)

if test x"$SWIG" = "x"; then
    AC_MSG_ERROR("swig not found")
fi

if test x"$PYTHON_LIB" = "x"; then
    AC_MSG_ERROR("python not found")
fi

AC_CONFIG_FILES(config.mk)

AC_OUTPUT
