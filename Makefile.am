AUTOMAKE_OPTIONS = foreign

SUBDIRS = po .

dfshowconfdir = $(sysconfdir)
dfshowdatadir = $(datadir)/dfshow

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS = -DSYSCONFIG=\"$(dfshowconfdir)\" -DDATADIR=\"$(dfshowdatadir)\" -D_XOPEN_SOURCE_EXTENDED -fno-common

LDADD = -lm -lconfig $(LIBINTL)

if DARWIN
LDADD += -lncurses
else
LDADD += -lncursesw
endif

if LINUX
LDADD += -lacl -ltinfo
endif

if HURD
LDADD += -lacl -ltinfo
endif

if SELINUX
LDADD += -lselinux
endif

bin_PROGRAMS = $(top_builddir)/bin/show $(top_builddir)/bin/sf
__top_builddir__bin_show_SOURCES = $(top_builddir)/src/show.c $(top_builddir)/src/showfunctions.c $(top_builddir)/src/sffunctions.c $(top_builddir)/src/showmenus.c $(top_builddir)/src/sfmenus.c $(top_builddir)/src/colors.c $(top_builddir)/src/common.c $(top_builddir)/src/menu.c $(top_builddir)/src/input.c $(top_builddir)/src/display.c $(top_builddir)/src/settings.c $(top_builddir)/src/i18n.c
# __top_builddir__bin_show_SOURCES = $(top_builddir)/src/show.c $(top_builddir)/src/showfunctions.c $(top_builddir)/src/showmenus.c $(top_builddir)/src/colors.c $(top_builddir)/src/common.c $(top_builddir)/src/menu.c $(top_builddir)/src/input.c $(top_builddir)/src/display.c $(top_builddir)/src/settings.c $(top_builddir)/src/i18n.c
__top_builddir__bin_show_LDADD = $(LDADD)
__top_builddir__bin_sf_SOURCES = $(top_builddir)/src/sf.c $(top_builddir)/src/sffunctions.c $(top_builddir)/src/sfmenus.c $(top_builddir)/src/colors.c $(top_builddir)/src/common.c $(top_builddir)/src/menu.c $(top_builddir)/src/input.c $(top_builddir)/src/display.c $(top_builddir)/src/settings.c $(top_builddir)/src/i18n.c
__top_builddir__bin_sf_LDADD = $(LDADD)
dfshowconf_DATA = $(top_builddir)/conf/dfshow.conf
dfshowdata_DATA = $(top_builddir)/themes/*

man_MANS = $(top_builddir)/man/show.1 $(top_builddir)/man/sf.1
noinst_HEADERS = $(top_builddir)/src/show.h $(top_builddir)/src/sf.h $(top_builddir)/src/showfunctions.h $(top_builddir)/src/showmenus.h $(top_builddir)/src/sfmenus.h $(top_builddir)/src/colors.h $(top_builddir)/src/common.h $(top_builddir)/src/banned.h $(top_builddir)/src/display.h $(top_builddir)/src/input.h $(top_builddir)/src/menu.h $(top_builddir)/src/settings.h $(top_builddir)/src/i18n.h $(top_builddir)/src/gettext.h

EXTRA_DIST = doc
