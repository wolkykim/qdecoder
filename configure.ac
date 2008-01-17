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
		opt_defs="$opt_defs $3"
	fi
])

AC_DEFUN([Q_ARG_DISABLE], [
	AC_ARG_ENABLE([$1],[AS_HELP_STRING([--disable-$1], [$2])],,[enableval=yes])
	if test "$enableval" = no; then
		opt_defs="$opt_defs $3"
	fi
])

## Initialize
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

## Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
AC_CHECK_HEADERS([mysql.h hello.h])

## Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_OFF_T

## Checks for libraries.

## Checks for library functions.
AC_CHECK_FUNCS([socket sendfile])

## Set additional arguments
opt_defs=""

Q_ARG_ENABLE([64bit], [enable 64 bit supports], [-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64])
Q_ARG_ENABLE([mysql], [enable MySQL database supports], [-DWITH_MYSQL])
#todo MYSQL_INCLUDE

Q_ARG_DISABLE([cgisupport], [exclude CGI related APIs], [-DWITHOUT_CGISUPPORT])
Q_ARG_DISABLE([configparser], [exclude configuration file parser & string parser APIs], [-DWITHOUT_CONFIGPARSER])
Q_ARG_DISABLE([arg], [exclude pattern matching & string search APIs], [-DWITHOUT_ARG])
Q_ARG_DISABLE([awk], [exclude file & string tokenizing APIs], [-DWITHOUT_AWK])
Q_ARG_DISABLE([sed], [exclude server side include and variable replacement APIs], [-DWITHOUT_SED])
Q_ARG_DISABLE([ipc], [exclude ipc(semaphore, shared-memory) APIs], [-DWITHOUT_IPC])
Q_ARG_DISABLE([socket], [exclude socket APIs], [-DWITHOUT_SOCKET])
Q_ARG_DISABLE([hashtbl], [exclude hash-table data structure APIs], [-DWITHOUT_HASHTBL])
Q_ARG_DISABLE([obstack], [exclude obstack APIs], [-DWITHOUT_OBSTACK])
Q_ARG_DISABLE([log], [exclude file logging APIs], [-DWITHOUT_LOG])

AC_MSG_NOTICE([optional flags $opt_defs])
AC_SUBST([OPT_DEFS], [$opt_defs] )

## Create Makefile
AC_OUTPUT
