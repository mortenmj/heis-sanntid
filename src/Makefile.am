AM_CFLAGS = -g -std=c99 -lpthread -D_GNU_SOURCE 

bin_PROGRAMS = heis
heis_SOURCES = \
	comms.c comms.h \
	messages.c messages.h \
	operator.c operator.h \
	orderlist.c orderlist.h \
    target.c target.h \
	debug.h \
	main.c

heis_LDADD = $(top_builddir)/libheis/libheis.la $(top_builddir)/cJSON/libcjson.la
