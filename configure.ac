AC_INIT([dfshow], [0.9.0-dev], [https://github.com/roberthawdon/dfshow/issues])
AC_GNU_SOURCE
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CC
AC_PROG_CC_STDC
AC_CONFIG_HEADERS([config.h:config.hin])
AC_SUBST([AM_LDFLAGS])

# Check for ACL_TYPE_NFS4
AC_CHECK_DECL([ACL_TYPE_NFS4],
    [AC_DEFINE(HAVE_ACL_TYPE_NFS4, 1, [True for FreeBSD with NFSv4 ACL support])],
    [],
    [#include <sys/acl.h>])
# MacOS has an acl.h that isn't POSIX.  It can be detected by
# checking for ACL_USER
AC_CHECK_DECL([ACL_USER],
    [AC_DEFINE(HAVE_ACL_USER, 1, [True for systems with POSIX ACL support])],
    [],
    [#include <sys/acl.h>])
# MacOS has ACL_TYPE_EXTENDED instead
AC_CHECK_DECL([ACL_TYPE_EXTENDED],
    [AC_DEFINE(HAVE_ACL_TYPE_EXTENDED, 1, [True for MacOS ACL support])],
    [],
    [#include <sys/types.h>
    #include <sys/acl.h>])
# Solaris and derivates ACLs
AC_CHECK_LIB([sec], [acl_get])
AC_CHECK_TYPES([aclent_t], [], [], [[#include <sys/acl.h>]])
AC_CHECK_TYPES([ace_t], [], [], [[#include <sys/acl.h>]])
AC_CHECK_FUNCS(acl_get facl_get acl_set facl_set)


AC_CHECK_MEMBERS([struct stat.st_author])
AC_CHECK_HEADERS([stdio.h limits.h signal.h ctype.h wctype.h getopt.h sys/types.h sys/stat.h dirent.h fcntl.h pwd.h string.h stdlib.h unistd.h time.h sys/statvfs.h libgen.h errno.h wchar.h hurd.h math.h sys/sysmacros.h regex.h utime.h sys/xattr.h sys/acl.h])
AC_CHECK_HEADERS(ncurses.h, , AC_MSG_ERROR(ncurses header (ncurses.h) not found. You may need to install an ncurses development package.))
AC_CHECK_HEADERS(libconfig.h, , AC_MSG_ERROR(libconfig header (libconfig.h) not found. You may need to install a libconfig development package.))

AC_ARG_WITH([selinux], AS_HELP_STRING([--with-selinux], [Build with selinux library (default: disabled)]))

AS_IF([test "x$with_selinux" = "xyes"], [
  AC_CHECK_HEADERS(selinux/selinux.h, , AC_MSG_ERROR(selinux header (selinux/selinux.h) not found. You may need to install an selinux development package.))
])

AM_CONDITIONAL([DARWIN], [test $(uname) = "Darwin"])
AM_CONDITIONAL([LINUX], [test $(uname) = "Linux"])
AM_CONDITIONAL([SELINUX], [test "x$with_selinux" = "xyes"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
