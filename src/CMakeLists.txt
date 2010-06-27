set(libtinyap_src
	3rd_party/lookup3.c
	3rd_party/hash_32a.c
	#builder.c
	ast.c
	bootstrap.c
	node_cache.c
	pilot_cache.c
	pilot_manager.c
	serialize.c
	stack.c
	tinyap.c
	tinyap_alloc.c
	tokenizer.c
	unparser.c
	unrepl.c
	walkableast.c
	walker.c
	ape_prettyprint.c
	string_registry.c
)

set(tinyap_ldflags "-ldl -rdynamic -lpcre")

add_library(libtinyap SHARED ${libtinyap_src})
set_target_properties(libtinyap
	PROPERTIES	OUTPUT_NAME tinyap
			LINK_FLAGS "${tinyap_ldflags}"
	)
			
add_executable(tinyap_static main.c ${libtinyap_src})
set_target_properties(tinyap_static
	PROPERTIES	LINK_FLAGS "${tinyap_ldflags} -pg"
			COMPILE_FLAGS "-pg"
	)

add_executable(tinyap main.c)
set_target_properties(tinyap
	PROPERTIES	LINK_FLAGS "${tinyap_ldflags}"
	)

target_link_libraries(tinyap libtinyap)


INSTALL(TARGETS tinyap tinyap_static DESTINATION bin)
INSTALL(TARGETS libtinyap DESTINATION lib)

