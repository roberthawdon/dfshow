AUTOMAKE_OPTIONS = foreign

dfshowconfdir = $(sysconfdir)
dfshowdatadir = $(datadir)/dfshow

# AM_CFLAGS = -DSYSCONFIG=\"$(dfshowconfdir)\" -DDATADIR=\"$(dfshowdatadir)\" -D_XOPEN_SOURCE_EXTENDED -D__USE_MINGW_ANSI_STDIO=1 -O3 -Wall
AM_CFLAGS = -DSYSCONFIG=\"$(dfshowconfdir)\" -DDATADIR=\"$(dfshowdatadir)\" -D_XOPEN_SOURCE_EXTENDED -D__USE_MINGW_ANSI_STDIO=1 -O3

if MACOS
LDADD = -lncurses -lm -lconfig
OS_SOURCE = src/os_unix.c
else
if WINDOWS
LDADD = -lncursesw -lm -lconfig -lpcre2-posix -lpcre2-8
OS_SOURCE = src/os_mswin.c
else
LDADD = -lncursesw -lm -lconfig
OS_SOURCE = src/os_unix.c
endif
endif

bin_PROGRAMS = $(top_builddir)/bin/show $(top_builddir)/bin/sf
# bin_PROGRAMS = $(top_builddir)/bin/sf
__top_builddir__bin_show_SOURCES = src/show.c src/showfunctions.c src/showmenus.c src/colors.c src/common.c $(OS_SOURCE)
__top_builddir__bin_show_LDADD = $(LDADD)
__top_builddir__bin_sf_SOURCES = src/sf.c src/sfmenus.c src/colors.c src/common.c $(OS_SOURCE)
__top_builddir__bin_sf_LDADD = $(LDADD)
dfshowconf_DATA = conf/dfshow.conf
dfshowdata_DATA = themes/*

man_MANS = $(top_builddir)/man/show.1 $(top_builddir)/man/sf.1
noinst_HEADERS = $(top_builddir)/src/show.h $(top_builddir)/src/sf.h $(top_builddir)/src/showfunctions.h $(top_builddir)/src/showmenus.h $(top_builddir)/src/sfmenus.h $(top_builddir)/src/colors.h $(top_builddir)/src/common.h
EXTRA_DIST = doc
