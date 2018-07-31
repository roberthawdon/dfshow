AUTOMAKE_OPTIONS = foreign
LDADD = -lncurses -lm
bin_PROGRAMS = show
show_SOURCES = src/main.c src/views.c src/functions.c src/menus.c src/colors.c
show_LDADD = $(LDADD)
man_MANS = doc/show.1
include_HEADERS = src/main.h src/views.h src/functions.h src/menus.h src/colors.h
EXTRA_DIST = doc
