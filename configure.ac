AC_INIT([dfshow], 
        m4_esyscmd_s([
            if test -d .git; then
                git describe --tags --dirty --always 2>/dev/null || echo "1.0.0-unknown"
            elif test -f .tarball-version; then
                cat .tarball-version
            else
                echo "1.0.0-unknown"
            fi
        ]), 
        [https://github.com/roberthawdon/dfshow/issues])
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h:config.hin])
AC_SUBST([AM_LDFLAGS])

# Adjust CFLAGS depending on the compiler
case "$CC" in
  *gcc*)
    CFLAGS="$CFLAGS -flto=auto -Werror=lto-type-mismatch"
    LDFLAGS="$LDFLAGS -flto=auto"
    ;;
  *clang*)
    CFLAGS="$CFLAGS"
    ;;
  *)
    AC_MSG_WARN([Unknown compiler, using default CFLAGS])
    ;;
esac

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.17.18)
AM_GNU_GETTEXT_REQUIRE_VERSION(0.17.18)

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

PKG_CHECK_MODULES([NCURSES], [ncursesw])

AC_CHECK_MEMBERS([struct stat.st_author])
AC_CHECK_HEADERS([stdio.h limits.h signal.h ctype.h wctype.h getopt.h sys/types.h sys/stat.h dirent.h fcntl.h pwd.h string.h stdlib.h unistd.h time.h sys/statvfs.h libgen.h errno.h wchar.h hurd.h math.h sys/sysmacros.h regex.h utime.h sys/xattr.h acl/libacl.h stdint.h])
AC_CHECK_HEADERS(libconfig.h, , AC_MSG_ERROR(libconfig header (libconfig.h) not found. You may need to install a libconfig development package.))
AC_CHECK_HEADERS(sys/acl.h, , AC_MSG_ERROR(libacl header (sys/acl.h) not found. You may need to install a libacl development package.))

AC_CHECK_PROG(VIM_CHECK,vim,yes)
AS_IF([test "x$VIM_CHECK" = "xyes"], [
  AC_DEFINE(HAVE_VIM, 1, [True for systems with Vim])
])

AC_CHECK_PROG(NANO_CHECK,nano,yes)
AS_IF([test "x$NANO_CHECK" = "xyes"], [
  AC_DEFINE(HAVE_NANO, 1, [True for systems with Nano])
])

AC_CHECK_PROG(VI_CHECK,vi,yes)
AS_IF([test "x$VI_CHECK" = "xyes"], [
  AC_DEFINE(HAVE_VI, 1, [True for systems with Vi])
])

AC_CHECK_PROG(EMACS_CHECK,emacs,yes)
AS_IF([test "x$EMACS_CHECK" = "xyes"], [
  AC_DEFINE(HAVE_EMACS, 1, [True for systems with Emacs])
])

AC_CHECK_PROG(JED_CHECK,jed,yes)
AS_IF([test "x$JED_CHECK" = "xyes"], [
  AC_DEFINE(HAVE_JED, 1, [True for systems with Jed])
])


AC_CHECK_PROG(LESS_CHECK,less,yes)
AS_IF([test "x$LESS_CHECK" = "xyes"], [
  AC_DEFINE(HAVE_LESS, 1, [True for systems with Less])
])

AC_CHECK_PROG(MORE_CHECK,more,yes)
AS_IF([test "x$MORE_CHECK" = "xyes"], [
  AC_DEFINE(HAVE_MORE, 1, [True for systems with More])
])


AC_ARG_WITH([selinux], AS_HELP_STRING([--with-selinux], [Build with selinux library (default: disabled)]))
AC_ARG_ENABLE([move-between-devices], AS_HELP_STRING([--enable-move-between-devices], [Enable moving objects between mounted devices (default: disabled)]))

AS_IF([test "x$with_selinux" = "xyes"], [
  AC_CHECK_HEADERS(selinux/selinux.h, , AC_MSG_ERROR(selinux header (selinux/selinux.h) not found. You may need to install an selinux development package.))
])

AS_IF([test "x$enable_move_between_devices" = "xyes"], [
  AC_DEFINE(HAVE_MOVE_BETWEEN_DEVICES, 1, [True for enable-move-between-devices])
])

AS_IF([test $(uname) = "Linux"], [
  AC_DEFINE(HAVE_GNU_BLOCKSIZE, 1, [True for Linux or Hurd systems])
])

AS_IF([test $(uname) = "GNU"], [
  AC_DEFINE(HAVE_GNU_BLOCKSIZE, 1, [True for Linux or Hurd systems])
])

AM_CONDITIONAL([DARWIN], [test $(uname) = "Darwin"])
AM_CONDITIONAL([LINUX], [test $(uname) = "Linux"])
AM_CONDITIONAL([HURD], [test $(uname) = "GNU"])
AM_CONDITIONAL([SELINUX], [test "x$with_selinux" = "xyes"])
AC_CONFIG_FILES([ po/Makefile.in
  Makefile
])
AC_OUTPUT
