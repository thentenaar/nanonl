ACLOCAL_AMFLAGS = -I m4
includedir=$(includedir)/nanonl

lib_LTLIBRARIES = libnanonl.la
include_HEADERS = src/nl.h
libnanonl_la_SOURCES = src/nl.c

if NL_GENERIC
include_HEADERS += src/nl_gen.h
libnanonl_la_SOURCES += src/nl_gen.c
endif

if NL_NETFILTER
include_HEADERS += src/nl_nf.h
libnanonl_la_SOURCES += src/nl_nf.c
endif

if NL_NFQUEUE
include_HEADERS += src/nl_nfqueue.h
libnanonl_la_SOURCES += src/nl_nfqueue.c
endif

