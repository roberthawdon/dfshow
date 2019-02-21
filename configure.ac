AC_INIT([dfshow], [0.6.2-dev], [https://github.com/roberthawdon/dfshow/issues])
AC_GNU_SOURCE
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CC
AC_PROG_CC_STDC
AC_CONFIG_HEADERS([config.h:config.hin])
AC_SUBST([AM_LDFLAGS])

AC_CHECK_MEMBERS([struct stat.st_author])
AC_CHECK_HEADERS([stdio.h limits.h signal.h ctype.h wctype.h getopt.h sys/types.h sys/stat.h dirent.h fcntl.h pwd.h string.h stdlib.h unistd.h time.h sys/statvfs.h libgen.h errno.h wchar.h hurd.h math.h sys/sysmacros.h regex.h utime.h])
AC_CHECK_HEADERS(ncurses.h, , AC_MSG_ERROR(ncurses header (ncurses.h) not found. You may need to install an ncurses development package.))
AC_CHECK_HEADERS(libconfig.h, , AC_MSG_ERROR(libconfig header (libconfig.h) not found. You may need to install a libconfig development package.))

AM_CONDITIONAL([DARWIN], [test $(uname) = "Darwin"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
