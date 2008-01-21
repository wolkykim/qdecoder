###########################################################################
## qDecoder - Web Application Interface for C/C++   http://www.qDecoder.org
##
## Copyright (C) 2007 Seung-young Kim.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published
## by the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################

AC_COPYRIGHT([========================================================================
qDecoder - Web Application Interface for C/C++   http://www.qDecoder.org

Copyright (C) 2007 Seung-young Kim.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
========================================================================])

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
#PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin:/usr/lib:/usr/libexec:/usr/ccs/bin:/usr/local/bin:/usr/local/sbin:/usr/local/lib:/usr/local/libexec:/usr/local/mysql/include
PATH=$PATH:/usr/local/mysql/include

AC_INIT([qDecoder], [8-CURRENT], [http://www.qdecoder.org/])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile examples/Makefile])

## Set autoconf setting
#AC_CANONICAL_TARGET
AC_PREFIX_DEFAULT([/usr/local])
#AC_PRESERVE_HELP_ORDER

## Checks for programs.
AC_PROG_CC
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
AC_CHECK_HEADER([sys/sendfile.h])

## Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_OFF_T

## Checks for libraries.

## Checks for library functions.
#AC_CHECK_FUNCS([socket sendfile, dkdkdk])

## Set additional arguments
opt_defs=""

Q_ARG_ENABLE([debug], [enable debugging print out], [-DBUILD_DEBUG])
Q_ARG_ENABLE([64bit], [enable 64 bit supports], [-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64])

AC_ARG_ENABLE([mysql],[AS_HELP_STRING([--enable-mysql=/MYSQL_INCLUDE_DIR_PATH/], [enable MySQL database supports])],[enableval=yes],[enableval=no])
if test "$enableval" = yes; then
	AC_CHECK_FILE([$enable_mysql/mysql.h],[enableval=yes],[enableval=no])
	if test "$enableval" = yes; then
		AC_MSG_NOTICE(['mysql' feature is enabled])
		opt_defs="$opt_defs -DENABLE_MYSQL -I$enable_mysql"
	else
		AC_MSG_FAILURE([can't find 'mysql.h' header in '$enable_mysql' directory])
	fi
fi

Q_ARG_DISABLE([cgisupport], [exclude CGI related APIs], [-DDISABLE_CGISUPPORT])
Q_ARG_DISABLE([configparser], [exclude configuration file parser & string parser APIs], [-DDISABLE_CONFIGPARSER])
Q_ARG_DISABLE([arg], [exclude pattern matching & string search APIs], [-DDISABLE_ARG])
Q_ARG_DISABLE([awk], [exclude file & string tokenizing APIs], [-DDISABLE_AWK])
Q_ARG_DISABLE([sed], [exclude server side include and variable replacement APIs], [-DDISABLE_SED])
Q_ARG_DISABLE([ipc], [exclude ipc(semaphore, shared-memory) APIs], [-DDISABLE_IPC])
Q_ARG_DISABLE([socket], [exclude socket APIs], [-DDISABLE_SOCKET])
Q_ARG_DISABLE([hashtbl], [exclude hash-table data structure APIs], [-DDISABLE_HASHTBL])
Q_ARG_DISABLE([obstack], [exclude obstack APIs], [-DDISABLE_OBSTACK])
Q_ARG_DISABLE([log], [exclude file logging APIs], [-DDISABLE_LOG])

AC_MSG_NOTICE([optional flags $opt_defs])
AC_SUBST([OPT_DEFS], [$opt_defs] )

## Create Makefile
AC_OUTPUT
