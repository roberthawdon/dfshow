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

bin_PROGRAMS = bin/show bin/sf
bin_show_SOURCES = src/show.c src/showfunctions.c src/sffunctions.c src/showmenus.c src/sfmenus.c src/colors.c src/common.c src/menu.c src/input.c src/display.c src/settings.c src/i18n.c
bin_show_LDADD = $(LDADD)
bin_show_CFLAGS = $(AM_CFLAGS) -DAPPLICATION_SHOW -DAPPLICATION_SF
bin_sf_SOURCES = src/sf.c src/sffunctions.c src/sfmenus.c src/colors.c src/common.c src/menu.c src/input.c src/display.c src/settings.c src/i18n.c
bin_sf_LDADD = $(LDADD)
bin_sf_CFLAGS = $(AM_CFLAGS) -DAPPLICATION_SF
dfshowconf_DATA = conf/dfshow.conf
dfshowdata_DATA = themes/*

man_MANS = man/show.1 man/sf.1
noinst_HEADERS = src/show.h src/sf.h src/showfunctions.h src/showmenus.h src/sfmenus.h src/colors.h src/common.h src/banned.h src/display.h src/input.h src/menu.h src/settings.h src/i18n.h src/gettext.h

EXTRA_DIST = doc
