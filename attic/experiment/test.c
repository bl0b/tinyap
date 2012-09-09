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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void bla(int a, short b, char c) {
	printf("a=%i b=%d c=%c\n", a, b, c);
}

typedef void (*ftype_t )();
typedef void (*ftype1_t)(void*);
typedef void (*ftype2_t)(void*,void*);
typedef void (*ftype3_t)(void*,void*,void*);
typedef void (*ftype4_t)(void*,void*,void*,void*);
typedef void (*ftype5_t)(void*,void*,void*,void*,void*);

void v(ftype_t f, int argc, ...) {
	static void* _[5];
	va_list ap;
	int i=0;
	va_start(ap, argc);
	while(i<argc&&i<5) {
		_[i] = va_arg(ap, void*);
		i+=1;
	}
	va_end(ap);
	switch(argc) {
	case 0:
		f();
		break;
	case 1:
		((ftype1_t)f)(_[0]);
		break;
	case 2:
		((ftype2_t)f)(_[0], _[1]);
		break;
	case 3:
		((ftype3_t)f)(_[0], _[1], _[2]);
		break;
	case 4:
		((ftype4_t)f)(_[0], _[1], _[2], _[3]);
		break;
	case 5:
		((ftype5_t)f)(_[0], _[1], _[2], _[3], _[4]);
		break;
	default:;
		abort();
	};
}


int main(int argc, char**argv) {
	struct {} t;
	v((ftype_t)bla, 3, (2<<23)-1, 65535, '@');
	return 0;
}

#if 0

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
	src = "1+2;";

	tinyap_set_source_file(parser, "../test.math");
	//tinyap_set_source_buffer(parser,src,strlen(src));
//	tinyap_set_source_file(parser, "stdin");
	tinyap_parse(parser);
	tree = tinyap_get_output(parser);
	if(tinyap_parsed_ok(parser)) {
		wast_t w=make_wast(Car(tree));
		int k = *(int*) do_walk(w,"test",NULL);
		printf("=> %i\n",k);
		puts(tinyap_serialize_to_string(tree));
		do_walk(w,"prettyprint",NULL);
	} else {
		printf(tinyap_get_error(parser));
	}

	tinyap_delete(parser);

	return 0;
}

#endif

