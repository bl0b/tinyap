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
#include "lr.h"

extern "C" {

/*#define _GNU_SOURCE*/

#include "config.h"
#include "ast.h"
#include "walker.h"
#include "bootstrap.h"
#include "string_registry.h"
#include "tinyap.h"
#include "parse_context.h"
#include "token_utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <stdio.h>
#include <sys/time.h>

/*typedef int pda_t;*/
/*void pda_free(pda_t x) {}*/
/*pda_t pda_new() { return 0; }*/

struct _tinyap_t {
	/*token_context_t*toktext;*/
	/*pda_t pda;*/
	/*parse_context_t context;*/
	lr::automaton* A;
	grammar::Grammar* G;
	
	char*grammar_source;
	ast_node_t grammar;
	ast_node_t start;

	ast_node_t output;

	char*ws;
	char*ws_source;

	unsigned int flags;

	char*source_file;
	char*source_buffer;
	size_t source_buffer_sz;

	float parse_time;

	int error;
};

int tinyap_verbose=0;

void delete_node(ast_node_t n);

ast_node_t copy_node(ast_node_t);

void node_pool_flush();
size_t node_pool_size();
extern volatile int _node_alloc_count;
void node_pool_init();
void node_pool_term();


void tinyap_set_verbose(int v) {
	tinyap_verbose=v;
}

void tinyap_terminate() {
	node_pool_term();
	term_pilot_manager();
	deinit_strreg();
	term_tinyap_alloc();
}

void tinyap_init() {
	static int is_init=0;
	if(is_init) {
		return;
	}
	is_init=1;
	init_tinyap_alloc();
	init_strreg();
	node_pool_init();
	/*atexit(tinyap_terminate);*/
	init_pilot_manager();
//	printf("after  tinyap_init : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
}


static struct tinyap_static_init {
	tinyap_static_init() { tinyap_init(); }
	~tinyap_static_init() { tinyap_terminate(); }
} _static_init;


void tinyap_dump_stack(tinyap_t t, const char*fnam) {
	if(t->A && t->A->stack) {
		std::ofstream of(fnam);
		of << "digraph tinyap_gss {" << std::endl;
		of << *t->A->stack;
		of << '}' << std::endl;
	}
}

void tinyap_print_states(tinyap_t t) {
	if(!t->A) {
		struct timeval t0, t1;
		gettimeofday(&t1, NULL);
		t->A = new lr::automaton(t->G);
		if(tinyap_verbose) {
			gettimeofday(&t0, NULL);
			fprintf(stderr, "took %.3f seconds to compute automaton.\n", 1.e-6f*(t0.tv_usec-t1.tv_usec)+t0.tv_sec-t1.tv_sec);

		}
		/*t->A->dump_states();*/
	}
	t->A->dump_states();
}

void tinyap_delete(tinyap_t t) {
//	printf("before tinyap_delete : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	if(t->A) delete t->A;
	if(t->G) delete t->G;

	if(t->grammar_source) free(t->grammar_source);
	/*if(t->grammar) delete_node(t->grammar);*/
	/* start was inside grammar */

	if(t->output) delete_node(t->output);

	if(t->ws) free(t->ws);
	if(t->ws_source) free(t->ws_source);

	if(t->source_file) free(t->source_file);
	if(t->source_buffer) free(t->source_buffer);

	free(t);
//	printf("after  tinyap_delete : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
}


tinyap_t tinyap_new() {
	tinyap_t ret=(tinyap_t)malloc(sizeof(struct _tinyap_t));
	memset(ret,0,sizeof(struct _tinyap_t));
	tinyap_set_grammar(ret,"short");
	ret->flags=0;
	/*init_pilot_manager();*/
	return ret;
}


void ast_serialize_to_file(const ast_node_t ast,FILE*f);
const char* ast_serialize_to_string(const ast_node_t ast);

void tinyap_serialize_to_file(const ast_node_t n,const char*fnam) {
	FILE*f;
	if(!(strcmp(fnam,"stdout")&&strcmp(fnam,"-"))) {
		f=stdout;
	} else {
		f=fopen(fnam,"w");
	}
	ast_serialize_to_file(n,f);
	fputc('\n',f);
	fclose(f);
}


const char*tinyap_serialize_to_string(const ast_node_t n) {
	return ast_serialize_to_string(n);
}



const char* tinyap_get_whitespace(tinyap_t t) {
	return t->ws;
}

void tinyap_set_whitespace(tinyap_t t,const char*ws) {
	if(t->ws_source) {
		free(t->ws_source);
	}
	if(t->ws) {
		free(t->ws);
	}
	t->ws_source=strdup(ws);
	t->ws=(char*)malloc(strlen(t->ws_source)+4);
	sprintf(t->ws,"[%s]+",t->ws_source);
	/*printf("has set whitespace RE to %s\n",t->ws);*/
}

void tinyap_set_whitespace_regexp(tinyap_t t,const char*re) {
	if(t->ws_source) {
		free(t->ws_source);
		t->ws_source=NULL;
	}
	if(t->ws) {
		free(t->ws);
	}
	t->ws=strdup(re);
	/*printf("has set whitespace RE to %s\n",t->ws);*/
}

const char* tinyap_get_grammar(tinyap_t t) {
	return t->grammar_source;
}

/* common to set_grammar and set_grammar_ast */
void init_grammar(tinyap_t t) {
	if(t->G) { delete t->G; }
	if(t->A) { delete t->A; t->A = NULL; }
	t->G = new grammar::Grammar(Cdr(Car(t->grammar)));
	/*grammar::visitors::debugger d;*/
	/*t->G->accept(&d);*/
	/*std::cout << std::endl;*/
}

void tinyap_set_grammar(tinyap_t t,const char*g) {
	if(t->grammar_source) {
		free(t->grammar_source);
		t->grammar_source=NULL;
	}
	t->grammar_source=strdup(g);
	if(t->grammar) {
		delete_node(t->grammar);
	}
	t->grammar=tinyap_get_ruleset(g);
	init_grammar(t);
}


ast_node_t tinyap_get_grammar_ast(tinyap_t t) {
	return t->grammar;
}

void tinyap_set_grammar_ast(tinyap_t t,ast_node_t g) {
	if(t->grammar_source) {
		free(t->grammar_source);
		t->grammar_source=NULL;
	}
	if(t->grammar) {
		delete_node(t->grammar);
	}
	t->grammar=g;
	init_grammar(t);
}

const char* tinyap_get_source_file(tinyap_t t) {
	return t->source_file;
}

const char* tinyap_get_source_buffer(tinyap_t t) {
	return t->source_buffer;
}

unsigned int tinyap_get_source_buffer_length(tinyap_t t) {
	return (unsigned int)t->source_buffer_sz;
}

float tinyap_get_parse_time(tinyap_t t) {
	return t->parse_time;
}

void tinyap_set_source_file(tinyap_t t,const char*fnam) {
	if(fnam) {
		struct stat st;
		FILE*f = NULL;

		struct timeval t0, t1;

		if(tinyap_verbose) {
			gettimeofday(&t1, NULL);
		}

		if(t->source_file) {
			free(t->source_file);
		}
		t->source_file=strdup(fnam);

		if(strcmp(fnam,"stdin")&&strcmp(fnam,"-")) {
			if(stat(t->source_file,&st)) {
				/* error */
				fprintf(stderr,"Couldn't stat %s :\n",t->source_file);
				perror("stat");
				tinyap_set_source_buffer(t,"",0);
				return;
			} else {
				f=fopen(t->source_file,"r");
			}
		
			if(t->source_buffer) {
				free(t->source_buffer);
			}
			t->source_buffer=(char*)malloc(st.st_size+1);
			t->source_buffer_sz=st.st_size;

			if(fread(t->source_buffer,1,st.st_size,f)) {}
			t->source_buffer[t->source_buffer_sz] = 0;

			fclose(f);
		} else {
			static char buf[4096];
			FILE* mem;
			size_t n;
			t->source_buffer = NULL;
			t->source_buffer_sz = 0;
			mem = open_memstream(&t->source_buffer, &t->source_buffer_sz);
			f = stdin;
			while((n=fread(buf, 1, 4096, f))>0) {
				fwrite(buf, 1, n, mem);
			}
			fflush(mem);
			/*printf("stdin input currently disabled. Sorry for the inconvenience.\n");*/
			/*abort();*/
		}
		if(tinyap_verbose) {
			gettimeofday(&t0, NULL);
			fprintf(stderr, "took %.3f seconds to read file contents.\n", 1.e-6f*(t0.tv_usec-t1.tv_usec)+t0.tv_sec-t1.tv_sec);
		}
	} else {
		tinyap_set_source_buffer(t,"",0);
	}
}

void tinyap_set_source_buffer(tinyap_t t,const char* b,const unsigned int sz) {
	if(t->source_file) {
		free(t->source_file);
		t->source_file=NULL;
	}
	if(t->source_buffer) {
		free(t->source_buffer);
	}
	t->source_buffer=(char*)malloc(sz+1);
	strncpy(t->source_buffer,b,sz);
	t->source_buffer[sz]=0;
	/*printf("Tinyap is about to parse this buffer (%u characters long) :\n"*/
		/*"================================================\n"*/
		/*"%*.*s\n"*/
		/*"================================================\n",*/
		/*sz, sz, sz, t->source_buffer);*/
	t->source_buffer_sz=sz;
}

int tinyap_parse(tinyap_t t) {
//	printf("before tinyap_parse : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	struct timeval t0, t1;
	unsigned long deltasec;

	if(!t->A) {
		gettimeofday(&t1, NULL);
		t->A = new lr::automaton(t->G);
		if(tinyap_verbose) {
			gettimeofday(&t0, NULL);
			fprintf(stderr, "took %.3f seconds to compute automaton.\n", 1.e-6f*(t0.tv_usec-t1.tv_usec)+t0.tv_sec-t1.tv_sec);

		}
		/*t->A->dump_states();*/
	}

	gettimeofday(&t1, NULL);

	if(t->output) {
		delete_node(t->output);
	}

	if(tinyap_verbose) {
		gettimeofday(&t0, NULL);
		fprintf(stderr, "took %.3f seconds to init parsing context.\n", 1.e-6f*(t0.tv_usec-t1.tv_usec)+t0.tv_sec-t1.tv_sec);
	}

	gettimeofday(&t0, NULL);
	t->output = t->A->parse(t->source_buffer, t->source_buffer_sz);
	/*t->output = token_produce_any(t->toktext, t->start, NULL);*/
	/*t->output = pda_parse(t->pda, t->source_buffer, t->source_buffer_sz, t->start, t->flags);*/
	gettimeofday(&t1, NULL);

	if(tinyap_verbose) {
		/*fprintf(stderr, "TinyaP parsed %u of %u characters.\n",t->toktext->farthest,t->toktext->size);*/
		fprintf(stderr, "TinyaP parsed %u of %u characters.\n",t->A->furthest,t->source_buffer_sz);
	}
//	token_context_free(t->toktext);
//	t->toktext=NULL;

//	printf("after  tinyap_parse : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	deltasec = t1.tv_sec-t0.tv_sec;
	t->parse_time = 1.e-6f*(t1.tv_usec-t0.tv_usec)+deltasec;

	return (t->error=(t->output!=NULL));
	
}


int tinyap_parse_as_grammar(tinyap_t t) {
	if(tinyap_parse(t)) {
		tinyap_set_grammar_ast(t,tinyap_get_output(t));
		t->output=NULL;
	}
	return t->error;
}

/*int tinyap_parsed_ok(const tinyap_t t) { return t->error||(t->toktext->ofs!=t->source_buffer_sz); }*/
int tinyap_parsed_ok(const tinyap_t t) { return t->error||(t->A->furthest!=t->source_buffer_sz); }

ast_node_t tinyap_get_output(const tinyap_t t) { return t->output; }
void tinyap_set_output(const tinyap_t t, ast_node_t o) { t->output = o; }

#if 1
int tinyap_get_error_col(const tinyap_t t) { return -1 /*parse_error_column(t->context)*/; }
int tinyap_get_error_row(const tinyap_t t) { return -1 /*parse_error_line(t->context)*/; }
const char* tinyap_get_error(const tinyap_t t) { return "TODO" /*parse_error(t->context)*/; }
#endif

int tinyap_node_is_nil(const ast_node_t  n) {
	return n==NULL;
}

int tinyap_node_is_list(const ast_node_t  n) {
	return isPair(n);
}

unsigned int tinyap_list_get_size(const ast_node_t n) {
	ast_node_t  o=n;
	unsigned int i=0;
	while(o) {
		o=getCdr(o);
		i+=1;
	}
	return i;
}

ast_node_t tinyap_list_get_element(const ast_node_t n,int i) {
	ast_node_t ret=n;
	while(ret&&i>0) {
		ret=getCdr(ret);
		i-=1;
	}
	if(!ret) return NULL;
	return getCar(ret);
}


int tinyap_node_is_string(const ast_node_t  n) {
	return isAtom(n);
}

const char* tinyap_node_get_string(const ast_node_t  n) {
	const char* ret = Value(n);
	if(n->node_flags&ATOM_IS_NOT_STRING) {
		ret = op2string((int)ret);
	}
	return ret;
}

ast_node_t  tinyap_node_get_operand(const ast_node_t  n,int i) {
	return tinyap_list_get_element(getCdr(n),i);
}

const char*	tinyap_node_get_operator(const ast_node_t  n) {
	return Value(getCar(n));
}

int tinyap_node_get_operand_count(const ast_node_t  n) {
	ast_node_t  o=getCdr(n);
	int i=0;
	while(o) {
		o=getCdr(o);
		i+=1;
	}
	return i;
}

int tinyap_node_get_row(const ast_node_t n) {
	return getOffset(n);
}

int tinyap_node_get_col(const ast_node_t n) {
	return 0;
}


ast_node_t tinyap_make_ast(const wast_t n) {
	return make_ast((wast_t)n);
}


wast_t tinyap_make_wast(const ast_node_t n) {
	return make_wast((ast_node_t)n);
}

void tinyap_free_wast(const wast_t w) {
	wa_del(w);
}


void* tinyap_walk(const wast_t subtree, const char* pilot_name, void* pilot_init_data) {
	return do_walk(subtree,pilot_name,pilot_init_data);
}



void tinyap_plug_node(tinyap_t parser, ast_node_t pin, const char* plugin, const char* plug) {
	ast_node_t p = find_nterm(parser->grammar,plug);
	ast_node_t alt/*,left*/,right,tmp;
	const char*tag;
	//assert(p);
	//assert(tinyap_node_get_operand_count(p)==3);
	if(p) {
		if(tinyap_node_get_operand_count(p)==2) {
			alt=tinyap_list_get_element(p,2);
			tag=tinyap_node_get_operator(alt);
			//assert(!strcmp(tag,"Alt"));
			if(!strcmp(tag,"Alt")) {
				/* now for the hack : alt <- cons(cadr(alt), cons(pin, cons(cddr(alt)))) */
				//left = Cdr(alt);
				//left = alt;
				// assume that the node consists of (* [string]) nodes
				tmp = Cdr(alt);
				while(tmp&&strcmp(Value(Car(Cdr(Car(tmp)))),plugin)) {
					tmp = Cdr(tmp);
				}
				if(!tmp) {
					right = Cdr(alt);
					pin->pair._cdr=right;
					alt->pair._cdr=pin;
				} else {
					fprintf(stderr,"tinyap: can't plug %s into %s : alternative already exists in rule.\n",plugin,plug);
				}
			} else {
				fprintf(stderr,"tinyap: can't plug %s into %s : the right hand side element in %s has to be an alternative.\n",plugin,plug,plug);
			}
		} else {
			fprintf(stderr,"tinyap: can't plug %s into %s : %s should have 1 right hand side element and not %i.\n",plugin,plug,plug,tinyap_node_get_operand_count(p)-1);
		}
	} else {
		fprintf(stderr,"tinyap: can't plug %s into %s : %s doesn't exist.\n",plugin,plug,plug);
	}
}

void tinyap_plug(tinyap_t parser, const char*plugin, const char*plug) {
	ast_node_t pin = newPair(newPair(newAtom("NT",0),
					 newPair(newAtom(plugin,0),
						 NULL)),
				 NULL);
	tinyap_plug_node(parser,pin,plugin,plug);
}


void tinyap_append_grammar(tinyap_t parser, ast_node_t supp) {
	grammar::rule::internal::append append;
	parser->grammar = append(Car(parser->grammar), Cdr(supp));
	///* yet another little hack */
	//Append(parser->grammar->pair._car,supp->pair._cdr);
}

} /* extern "C" */


