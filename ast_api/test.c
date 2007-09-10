#include <dlfcn.h>
#include <stdio.h>

#include "../tinyap.h"
#include "walker.h"



typedef const char* (*fun_t) ();

const char* plop() { return "hop."; }

int main(int argc, const char**argv) {
	tinyap_t parser;
	ast_node_t tree;
	char*src;

	void* ld_self = dlopen(NULL, RTLD_LAZY);
	void* pouet = dlsym( ld_self, "plop" );
	printf("self = %p\n",ld_self);
	printf("pouet = %p\n",pouet);
	if(pouet) {
		printf("%s\n",((fun_t)pouet)());
	} else {
		printf("%s\n",dlerror());
	}
	dlclose(ld_self);

	printf("\n\n");

	init_pilot_manager();

	parser = tinyap_new();

	tinyap_set_source_file(parser,"../math.gram");
	tinyap_parse_as_grammar(parser);

	src = "23*42+23-42;";

	tinyap_set_source_buffer(parser,src,strlen(src));
	tinyap_parse(parser);
	if(tinyap_parsed_ok(parser)) {
		tree = tinyap_get_output(parser);
		int k = *(int*) do_walk(make_wast(Car(tree)),"test",NULL);
		printf("=> %i\n",k);
	} else {
		printf(tinyap_get_error(parser));
	}

	tinyap_delete(parser);

	return 0;
}

