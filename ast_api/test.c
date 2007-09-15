/* Tinya(J)P : this is not yet another (Java) parser.
 * Copyright (C) 2007 Damien Leroux
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

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

//	void* ld_self = dlopen(NULL, RTLD_LAZY);
//	void* pouet = dlsym( ld_self, "plop" );
//	printf("self = %p\n",ld_self);
//	printf("pouet = %p\n",pouet);
//	if(pouet) {
//		printf("%s\n",((fun_t)pouet)());
//	} else {
//		printf("%s\n",dlerror());
//	}
//	dlclose(ld_self);
//
//	printf("\n\n");

	printf("ιθηΰ ploum ploum tralala\n\n");

	init_pilot_manager();

	parser = tinyap_new();

	tinyap_set_source_file(parser, "../math.gram");
	tinyap_parse_as_grammar(parser);

//	src = "23*42+23-42;";
//	src = "1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2))))))));";
	src = "1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2)))))))))))));";

	tinyap_set_source_buffer(parser,src,strlen(src));
//	tinyap_set_source_file(parser, "stdin");
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

