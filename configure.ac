dnl Copyright (C) 2014 William Breathitt Gray
dnl
dnl This file is part of Cassandra.
dnl
dnl Cassandra is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published
dnl by the Free Software Foundation, either version 3 of the License,
dnl or (at your option) any later version.
dnl
dnl Cassandra is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with Cassandra.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.69])
AC_INIT([Cassandra], [1.0], [vilhelm.gray@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/cassandra.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.
AC_SEARCH_LIBS([sqrt], [m],
             [AC_CHECK_HEADERS([math.h], [],
                               [AC_MSG_ERROR([*** The math header file was not found.])]
              )],
             [AC_MSG_ERROR([*** The math library was not found.])]
)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([floor sqrt])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
