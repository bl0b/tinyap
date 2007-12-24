include_HEADERS = tinyap.h tinyape.h bootstrap.h
noinst_HEADERS = ast.h pilot_manager.h pilot_cache.h hashtab.h node_cache.h tokenizer.h stack.h walker.h

#bin_PROGRAMS = tinyap_static
#bin_PROGRAMS = tinyap tinyap_static
bin_PROGRAMS = tinyap
lib_LTLIBRARIES = libtinyap.la

tinyap_SOURCES = main.c
tinyap_LDADD = -ldl libtinyap.la
tinyap_LDFLAGS = -Wl,--export-dynamic

libtinyap_la_SOURCES = ast.c tokenizer.c ape_prettyprint.c node_cache.c pilot_cache.c pilot_manager.c stack.c tinyap.c walkableast.c walker.c serialize.c bootstrap.c


#tinyap_static_SOURCES = $(libtinyap_la_SOURCES) $(tinyap_SOURCES)
#tinyap_static_LDADD = -ldl
#tinyap_static_LDFLAGS = -Wl,--export-dynamic

