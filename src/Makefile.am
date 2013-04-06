AM_CFLAGS = -std=c99 -lpthread -D_GNU_SOURCE 

bin_PROGRAMS = heis
heis_SOURCES = \
	comms.c comms.h \
	messages.c messages.h \
	queue.c queue.h \
	operator.c operator.h \
	orderLists.c orderLists.h
	debug.h \
	main.c

heis_LDADD = $(top_builddir)/libheis/libheis.la $(top_builddir)/cJSON/libcjson.la
