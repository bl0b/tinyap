include_HEADERS = tinyap.h tinyape.h

bin_PROGRAMS = tinyap
lib_LTLIBRARIES = libtinyap.la

tinyap_SOURCES = main.c
tinyap_LDADD = -ldl libtinyap.la
tinyap_LDFLAGS = -Wl,--export-dynamic

libtinyap_la_SOURCES = ast.c tokenizer.c ape_prettyprint.c node_cache.c pilot_cache.c pilot_manager.c stack.c tinyap.c walkableast.c walker.c serialize.c bootstrap.c
