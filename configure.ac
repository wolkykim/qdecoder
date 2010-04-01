## Copyright 2000-2010 The qDecoder Project. All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE QDECODER PROJECT ``AS IS'' AND ANY
## EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
## WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL THE QDECODER PROJECT BE LIABLE FOR ANY
## DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
## LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
## ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
## THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## $Id$

AC_COPYRIGHT([
=============================================================================
Copyright 2000-2010 The qDecoder Project. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE QDECODER PROJECT ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE QDECODER PROJECT BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=============================================================================
])

## Internal functions
AC_DEFUN([Q_ARG_ENABLE], [
	AC_ARG_ENABLE([$1],[AS_HELP_STRING([--enable-$1], [$2])],,[enableval=no])
	if test "$enableval" = yes; then
		AC_MSG_NOTICE(['$1' feature is enabled])
		opt_defs="$opt_defs $3"
	fi
])

AC_DEFUN([Q_ARG_DISABLE], [
	AC_ARG_ENABLE([$1],[AS_HELP_STRING([--disable-$1], [$2])],,[enableval=yes])
	if test "$enableval" = no; then
		AC_MSG_NOTICE(['$1' feature is disabled])
		opt_defs="$opt_defs $3"
	fi
])

## Initialize
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin:/usr/lib:/usr/libexec:/usr/local/bin:/usr/local/sbin:/usr/local/lib:/usr/local/libexec

AC_INIT([qDecoder], [10 RELEASE], [http://www.qdecoder.org/])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile examples/Makefile])

## Set autoconf setting
#AC_CANONICAL_TARGET
AC_PREFIX_DEFAULT([/usr/local])
#AC_PRESERVE_HELP_ORDER

## Checks for programs.
AC_PROG_CC
if test $ac_compiler_gnu = yes; then
	CFLAGS="$CFLAGS -Wall -fPIC"
#else
#	AC_MSG_FAILURE([GCC is required to compile qDecoder.])
fi

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_PATH_PROG(AR, ar)
AC_PATH_PROG(CHMOD, chmod)
AC_PATH_PROG(LD, ld)
AC_PATH_PROG(RM, rm)

## Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
#AC_CHECK_HEADER([sys/sendfile.h])

## Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_OFF_T

## Checks for libraries.

## Checks for library functions.
#AC_CHECK_FUNCS([socket sendfile])

## Set additional arguments
opt_defs="-D_GNU_SOURCE"
opt_incs=""
opt_libs=""

Q_ARG_ENABLE([debug], [enable debugging output], [-DBUILD_DEBUG])
Q_ARG_ENABLE([threadsafe], [enable thread-safe feature], [-DENABLE_THREADSAFE])
if test "$enableval" = yes; then
	AC_MSG_NOTICE(['threadsafe' feature is enabled])
	opt_libs="-lpthread"
fi

Q_ARG_ENABLE([lfs], [enable large file supports], [-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64])
#Q_ARG_ENABLE([sendfile], [use sendfile() if possible], [-DENABLE_SENDFILE])

AC_ARG_ENABLE([fastcgi],[AS_HELP_STRING([--enable-fastcgi=/FASTCGI_INCLUDE_DIR_PATH/], [enable FastCGI supports])],[enableval=yes],[enableval=no])
if test "$enableval" = yes; then
	AC_CHECK_FILE([$enable_fastcgi/fcgi_stdio.h],[enableval=yes],[enableval=no])
	if test "$enableval" = yes; then
		AC_MSG_NOTICE(['fastcgi' feature is enabled])
		opt_defs="$opt_defs -DENABLE_FASTCGI"
		opt_incs="$opt_incs -I$enable_fastcgi"
	else
		AC_MSG_FAILURE([can't find 'fcgi_stdio.h' header file in '$enable_fastcgi' directory. can't enable fastcgi support.])
	fi
fi

AC_ARG_ENABLE([mysql],[AS_HELP_STRING([--enable-mysql=/MYSQL_INCLUDE_DIR_PATH/], [enable MySQL database supports])],[enableval=yes],[enableval=no])
if test "$enableval" = yes; then
	AC_CHECK_FILE([$enable_mysql/mysql.h],[enableval=yes],[enableval=no])
	if test "$enableval" = yes; then
		AC_MSG_NOTICE(['mysql' feature is enabled])
		opt_defs="$opt_defs -DENABLE_MYSQL"
		opt_incs="$opt_incs -I$enable_mysql"
	else
		AC_MSG_FAILURE([can't find 'mysql.h' header file in '$enable_mysql' directory. can't enable mysql support.])
	fi
fi

Q_ARG_DISABLE([cgi], [disable CGI APIs], [-DDISABLE_CGI])
Q_ARG_DISABLE([socket], [disable socket APIs], [-DDISABLE_SOCKET])
Q_ARG_DISABLE([ipc], [disable IPC APIs], [-DDISABLE_IPC])
Q_ARG_DISABLE([datastructure], [disable data-structure APIs], [-DDISABLE_DATASTRUCTURE])

AC_MSG_NOTICE([optional flags: $opt_defs $opt_incs $opt_libs])
AC_SUBST([OPT_DEFS], [$opt_defs] )
AC_SUBST([OPT_INCS], [$opt_incs] )
AC_SUBST([OPT_LIBS], [$opt_libs] )

## Create Makefile
AC_OUTPUT
