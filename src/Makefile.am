AM_CFLAGS = -std=c99

bin_PROGRAMS = heis
heis_SOURCES = \
	comms.c comms.h \
	queue.c queue.h \
	ui.c ui.h \
    debug.h \
	main.c

heis_LDADD = $(top_builddir)/libheis/libheis.la
LDADD = $(top_builddir)/libheis/libheis.la
