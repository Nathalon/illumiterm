# Copyright 2023 Elijah Gordon (SLcK) <braindisassemblue@gmail.com>

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

AC_PREREQ([2.64])
AC_CONFIG_MACRO_DIRS([m4])
AC_INIT([illumiterm], [braindisassemblue@gmail.com])
AC_CONFIG_SRCDIR([src/illumiterm.c])

AC_CONFIG_FILES([Makefile src/Makefile])

AM_INIT_AUTOMAKE([foreign -Wall -Werror tar-ustar])
AM_SILENT_RULES
AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_EGREP

LT_INIT

PKG_CHECK_MODULES([GTK], [gtk+-3.0 gdk-3.0])
PKG_CHECK_MODULES([VTE], [vte-2.91])

AC_DEFUN([AX_LDFLAGS_OPTION], [
  AC_MSG_CHECKING([for linker flag $1])
  case " $LDFLAGS " in
    *" $1 "*) AC_MSG_RESULT(yes) ;;
    *) LDFLAGS="$LDFLAGS $1"; AC_MSG_RESULT(no) ;;
  esac
  AC_SUBST(MORE_LDFLAGS, [$2])
])

AX_LDFLAGS_OPTION([-Wl,-z,relro -Wl,-z,now], [MORE_LDFLAGS])

AC_ARG_VAR([MORE_CFLAGS], [Additional compiler flags])

AC_CACHE_SAVE

AC_SUBST([MORE_CFLAGS])
AC_SUBST([MORE_CPPFLAGS])
AC_SUBST([MORE_LDFLAGS])

AC_OUTPUT
cat <<EOF

------------------ Summary ------------------
  Prefix.........: $prefix
  C Compiler.....: $CC $MORE_CFLAGS $MORE_CPPFLAGS $CFLAGS $CPPFLAGS
  Linker.........: $LD $MORE_LDFLAGS $LDFLAGS $LIBS
---------------------------------------------

Check the above options and compile with: ${MAKE-make}

EOF
