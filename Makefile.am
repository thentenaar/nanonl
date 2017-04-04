ACLOCAL_AMFLAGS = -I m4
incdir=$(includedir)/nanonl

if HAVE_CHECK
check_PROGRAMS = tests
tests_LDADD    = -lcheck
tests_SOURCES  = test/gen.c test/nfqueue.c test/nl.c test/test.c

check-local: tests
	@./tests
endif

lib_LTLIBRARIES = libnanonl.la
inc_HEADERS = src/nl.h
libnanonl_la_SOURCES = src/nl.c

if NL_GENERIC
inc_HEADERS += src/nl_gen.h
libnanonl_la_SOURCES += src/nl_gen.c
endif

if NL_NETFILTER
inc_HEADERS += src/nl_nf.h
libnanonl_la_SOURCES += src/nl_nf.c
endif

if NL_NFQUEUE
inc_HEADERS += src/nl_nfqueue.h
libnanonl_la_SOURCES += src/nl_nfqueue.c
endif

if NL_IFINFO
inc_HEADERS += src/nl_ifinfo.h
libnanonl_la_SOURCES += src/nl_ifinfo.c
endif

if NL_IFADDR
inc_HEADERS += src/nl_ifaddr.h
libnanonl_la_SOURCES += src/nl_ifaddr.c
endif

examples: 
	@$(MAKE) -C example all

clean-local:
	@$(MAKE) -C example clean

