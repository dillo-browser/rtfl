dnl Process this file with aclocal, autoconf and automake.

AC_INIT([rtfl], [0.1.1])

dnl Detect the canonical target build environment
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([objects/rtfl_objview.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl Options

JAVA_HOME=""
WARN_GRAPH=no

AC_ARG_ENABLE(efence, [  --enable-efence         Try to compile and run with Electric Fence],
                    , enable_efence=no)
AC_ARG_ENABLE(gprof,  [  --enable-gprof          Try to compile and run with profiling enabled],
                    , enable_gprof=no)
AC_ARG_ENABLE(insure, [  --enable-insure         Try to compile and run with Insure++],
                    , enable_insure=no)
AC_ARG_ENABLE(rtfl,   [  --enable-rtfl           Build with RTFL messages (for debugging rendering)])
AC_ARG_ENABLE(graph2, [  --disable-graph2        Use simple Graph widget instead of Graph2])
AC_ARG_ENABLE(java,   [  --disable-java          Build RTFL java agent])
AC_ARG_WITH(java-home,[  --with-java-home=DIR    Specify where to find the JDK], JAVA_HOME=$withval)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

dnl ----------------------------
dnl Check our char and int types
dnl ----------------------------
dnl
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(void *)

AC_TYPE_INT16_T
AC_TYPE_UINT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T

dnl -----------------------------------------------------------------
dnl Check for absolute path of working directory.
dnl This is needed for RTFL, to get full the full paths of the source
dnl file names
dnl -----------------------------------------------------------------
dnl
BASE_CUR_WORKING_DIR=`pwd`

dnl --------------------------------------
dnl Check whether to add /usr/local or not
dnl (this is somewhat a religious problem)
dnl --------------------------------------
dnl
if test "`$CPP -v < /dev/null 2>&1 | grep '/usr/local/include' 2>&1`" = ""; then
  CPPFLAGS="$CPPFLAGS -I/usr/local/include"
  LDFLAGS="$LDFLAGS -L/usr/local/lib"
fi

dnl ----------------------
dnl Test for FLTK 1.3 library
dnl ----------------------
dnl
dnl For debugging and to be user friendly
AC_MSG_CHECKING([FLTK 1.3])
fltk_version="`fltk-config --version 2>/dev/null`"
case $fltk_version in
  1.3.*) AC_MSG_RESULT(yes)
         LIBFLTK_CXXFLAGS=`fltk-config --cxxflags`
         LIBFLTK_LIBS=`fltk-config --ldflags`;;
  ?*)    AC_MSG_RESULT(no)
         AC_MSG_ERROR(FLTK 1.3 required; version found: $fltk_version);;
  *)     AC_MSG_RESULT(no)
         AC_MSG_ERROR(FLTK 1.3 required; fltk-config not found)
esac

dnl -------------------------
dnl Test for Graphviz library
dnl -------------------------
AC_MSG_CHECKING([Graphviz library >= 2.38.0])
AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <graphviz/graphviz_version.h>
#include <ctype.h>
#include <stdlib.h>

static int version_cmp (const char *v1, const char *v2)
{
   const char *s1 = v1, *s2 = v2;
   while (*s1 && *s2) {
      if (isdigit (*s1) && isdigit (*s2)) {
         char buf1[10], buf2[10];
         int n1 = 0, n2 = 0;

         while (isdigit (*s1)) {
            if (n1 < 9) buf1[n1++] = *s1;
            s1++;
         }

         while (isdigit (*s2)) {
            if (n2 < 9) buf2[n2++] = *s2;
            s2++;
         }

         buf1[n1] = buf2[n2] = 0;
         int c = atoi (buf1) - atoi (buf2);
         if (c != 0)
            return c;
      } else {
         if (*s1 != *s2)
            return *s1 - *s2;
         s1++;
         s2++;
      }
   }

   return *s1 - *s2;
}
]],[[
if(version_cmp (PACKAGE_VERSION, "2.38.0") >= 0)
   return 0;
else
   return 1;
]])],
[AC_MSG_RESULT(yes)
 HAS_GRAPHVIZ=yes
 AM_CONDITIONAL([HAS_GRAPHVIZ], [true])
 GRAPHVIZ_LIBS="-lcgraph -lgvc"],
[AC_MSG_RESULT(no)
 HAS_GRAPHVIZ=no
 AM_CONDITIONAL([HAS_GRAPHVIZ], [false])]
[AC_MSG_RESULT(no)
 HAS_GRAPHVIZ=no
 AM_CONDITIONAL([HAS_GRAPHVIZ], [false])
 AC_MSG_WARN([Testing Graphviz library not possible when cross-compiling.])])

dnl ------------
dnl Test for JDK
dnl ------------
AC_MSG_CHECKING([JDK])
if test "$enable_java" = no ; then
   AC_MSG_RESULT(disabled)
   AM_CONDITIONAL([HAS_JAVA], [false])
   JAVA_HOME=""
else
    # Follow symbolic links of javac in $PATH.
    if test -z "$JAVA_HOME"; then
        if which javac >/dev/null; then
           javac=$(which javac)
           while test -n "$javac"; do
               nextjavac=$(readlink $javac)
               if test -z "$nextjavac"; then
                   JAVA_HOME=${javac%/bin/javac}
                   if test $JAVA_HOME = $javac; then
                      AC_MSG_RESULT(no)
                      AC_MSG_WARN([Javac found at $javac, which is strange.])
                      AM_CONDITIONAL([HAS_JAVA], [false])
                      JAVA_HOME=""
                   fi
               fi
               javac=$nextjavac
           done
        else
            AC_MSG_RESULT(no)
            AM_CONDITIONAL([HAS_JAVA], [false])
            JAVA_HOME=""
        fi
    fi
fi

if test -n "$JAVA_HOME"; then
   if test '!' -e $JAVA_HOME/include/jvmti.h; then
      AC_MSG_RESULT(no)
      AC_MSG_WARN([$JAVA_HOME/include/jvmti.h not found: JDK not usable.])
      AM_CONDITIONAL([HAS_JAVA], [false])
      JAVA_HOME=""
   else
      AC_MSG_RESULT(yes)
      AM_CONDITIONAL([HAS_JAVA], [true])
      # Hopefully only simple directory names ...
      JAVA_CFLAGS="-I$JAVA_HOME/include"
      if test '!' -e $JAVA_HOME/include/jni_md.h; then
         # For some JDKs, "jni_md.h" is only in a OS specific directory.
         JAVA_CFLAGS=$JAVA_CFLAGS" -I"`find $JAVA_HOME/include/ -name jni_md.h \
                                       | head -1 | xargs dirname`
      fi
   fi
fi

dnl --------------------
dnl Command line options
dnl --------------------
dnl
if test "x$enable_efence" = "xyes" ; then
  LIBS="-lefence $LIBS"
fi
if test "x$enable_gprof" = "xyes" ; then
  CXXFLAGS="$CXXFLAGS -pg"
fi
if test "x$enable_insure" = "xyes" ; then
  CC="insure -Zoi \"compiler $CC\""
  LIBS="$LIBS -lstdc++-2-libc6.1-1-2.9.0"
fi
if test "x$enable_rtfl" = "xyes" ; then
  CXXFLAGS="$CXXFLAGS -DDBG_RTFL"
fi
if test "$enable_graph2" = no ; then
  AM_CONDITIONAL([USE_GRAPH2], [false])
else
  if test $HAS_GRAPHVIZ = yes ; then
    AM_CONDITIONAL([USE_GRAPH2], [true])
    CXXFLAGS="$CXXFLAGS -DUSE_GRAPH2"
  else
    AM_CONDITIONAL([USE_GRAPH2], [false])
    AC_MSG_WARN([Graphviz library >= 2.38.0 not found. Graph2 is not enabled.])
    WARN_GRAPH=yes
  fi
fi

dnl -----------------------
dnl Checks for header files
dnl -----------------------
dnl
AC_CHECK_HEADERS(fcntl.h unistd.h sys/uio.h)

dnl --------------------------
dnl Check for compiler options
dnl --------------------------
dnl
if eval "test x$GCC = xyes"; then
  if test "`echo $CFLAGS | grep '\-D_REENTRANT' 2> /dev/null`" = ""; then
    CFLAGS="$CFLAGS -D_REENTRANT"
  fi
  if test "`echo $CFLAGS | grep '\-D_THREAD_SAFE' 2> /dev/null`" = ""; then
    CFLAGS="$CFLAGS -D_THREAD_SAFE"
  fi
  if test "`echo $CFLAGS | grep '\-Wall' 2> /dev/null`" = ""; then
    CFLAGS="$CFLAGS -Wall"
  fi
  if test "`echo $CFLAGS | grep -e '-W ' -e '-W$' 2> /dev/null`" = ""; then
    CFLAGS="$CFLAGS -W"
  fi
  if test "`echo $CFLAGS | grep '\-Wno-unused-parameter' 2> /dev/null`" = ""; then
    CFLAGS="$CFLAGS -Wno-unused-parameter"
  fi
  if test "`echo $CFLAGS | grep '\-Waggregate-return' 2> /dev/null`" = ""; then
    CFLAGS="$CFLAGS -Waggregate-return"
  fi
fi

dnl -----------
dnl CXX options
dnl -----------
dnl
if eval "test x$GCC = xyes"; then
  CXXFLAGS="$CXXFLAGS -Wall -W -Wno-unused-parameter -fno-rtti -fno-exceptions"
fi

AC_SUBST(BASE_CUR_WORKING_DIR)
AC_SUBST(LIBFLTK_CXXFLAGS)
AC_SUBST(GRAPHVIZ_LIBS)
AC_SUBST(LIBFLTK_LIBS)
AC_SUBST(JAVA_HOME)
AC_SUBST(JAVA_CFLAGS)
AC_SUBST(datadir)

AC_CONFIG_FILES([
    Makefile
    lout/Makefile
    common/Makefile
    dw/Makefile
    dwr/Makefile
    objects/Makefile
    tests/Makefile
    scripts/Makefile
    doc/Makefile
    java/Makefile
])

AC_OUTPUT

dnl --------------------------
dnl Some more notable messages
dnl --------------------------

if test "$WARN_GRAPH" = yes; then
   echo '----------------------------------------------------------------------'
   echo 'NOTE: No Graphviz library >= 2.38.0 was found, and you did not'
   echo 'explicitly disable the Graph2 widget, which depends on Graphviz. It is'
   echo 'recommend to install the Graphviz library and thus enable Graph2 which'
   echo 'greatly improves rendering of rtfl-objview. See README for details.'
   echo '----------------------------------------------------------------------'
fi   	
