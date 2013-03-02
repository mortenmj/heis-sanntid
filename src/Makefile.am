AM_CFLAGS = -std=c99 -lpthread

bin_PROGRAMS = heis
heis_SOURCES = \
	comms.c comms.h \
	queue.c queue.h \
	debug.h \
	main.c

heis_LDADD = $(top_builddir)/libheis/libheis.la $(top_builddir)/cJSON/libcjson.la
