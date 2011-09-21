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
#include "config.h"

#include "lr.h"
#include "string_registry.h"
#include "ast.h"
#include "static_init.h"

namespace grammar {
	namespace item {
		void clean_registry_at_exit();
	}
}

struct tinyap_static_init _static_init;





extern "C" {

/*#define _GNU_SOURCE*/

#include "walker.h"
#include "bootstrap.h"
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

	ext::hash_map<const char*, trie_t> bows;
	
	char*grammar_source;
	Ast grammar;
	/*ast_node_t start;*/

	Ast output;

	char*ws;
	char*ws_source;

	unsigned int flags;

	char*source_file;
	char*source_buffer;
	size_t source_buffer_sz;

	float parse_time;

	int error;

	struct position_t : std::pair<int, int> {
		int row() const { return first; }
		int col() const { return second; }
		void row(int r) { first = r; }
		void col(int c) { second = c; }
	};

	struct pos_cache_t {
		std::vector<int> row_offset;
		const char* source;
		size_t sz;
		pos_cache_t() : row_offset() { reset(NULL); }
		pos_cache_t(const char*s) : row_offset() { reset(s); }
		void reset(const char* s, size_t slen) {
			source = s;
			sz = slen;
			row_offset.clear();
			row_offset.push_back(0);
		}
		void reset(const char* s) {
			source = s;
			sz = s?strlen(s):0;
			row_offset.clear();
			row_offset.push_back(0);
		}
		position_t position_of(int ofs) {
			position_t ret;
			if(((size_t)ofs)>sz) {
				std::cerr << "position_of beyond end at " << ofs << std::endl;
				ret.row(-1);
				ret.col(-1);
				throw 0;
				return ret;
			}
			if(ofs>row_offset.back()) {
				/* need to fill the list */
				size_t nl_temp = row_offset.back()+1;
				while(nl_temp < ((size_t)ofs) && nl_temp < sz) {
					nl_temp = strchr(source+nl_temp, '\n')-source;
					if(!nl_temp) {
						break;
					}
					row_offset.push_back(nl_temp);
					++nl_temp;
				}
				ret.row(row_offset.size());
				ret.col(ofs-row_offset.back());
			} else {
				/* dichotomy */
				size_t needle = row_offset.size()>>1;
				size_t delta = needle;
				int line;
				while(delta) {
					line = row_offset[needle];
					if(line > ofs) {
						needle -= delta;
					} else if(row_offset[needle+1]>ofs) {
						break;
					} else if(line <= ofs) {
						needle += delta;
					}
					delta >>= 1;
				}
				ret.row(needle);
				ret.col(ofs-row_offset[needle]);
			}
			return ret;
		}
	} pos_cache;

	_tinyap_t()
		: A(0), G(0), bows(),
		  grammar_source(0), grammar(0),
		  /*start(0),*/ output(0), ws(0), ws_source(0),
		  flags(0), source_file(0), source_buffer(0),
		  source_buffer_sz(0), parse_time(0), error(0)
		  , pos_cache()
	{}

	~_tinyap_t() {
		ext::hash_map<const char*, trie_t>::iterator i, j;
		for(i=bows.begin(), j=bows.end(); i!=j; ++i) {
			trie_free((*i).second);
		}
	}
};

int tinyap_verbose=0;

/*void delete_node(ast_node_t n);*/

ast_node_t copy_node(ast_node_t);

void node_pool_flush();
size_t node_pool_size();
extern volatile int _node_alloc_count;
extern volatile int _node_dealloc_count;
void node_pool_init();
void node_pool_term();


trie_t tinyap_get_bow(const char* tag) {
	return grammar::item::token::Bow::find(tag);
}


void tinyap_set_verbose(int v) {
	tinyap_verbose=v;
}

void flush_nodes() {
	std::set<ast_node_t>::iterator i, j;
	j = _static_init.still_has_refs.end();
	i = _static_init.still_has_refs.begin();
	for(;i!=j;++i) {
		std::cerr << "ast node " << (*i)->raw.ref << " refs " << (*i) << std::endl;
	}
#if 0
	std::list<ast_node_t> to_remove;
	std::set<ast_node_t>::iterator i, j;
	std::list<ast_node_t>::iterator k, l;
	while(	j = _static_init.still_has_refs.end(),
			i = _static_init.still_has_refs.begin(),
			i!=j) {
		to_remove.clear();
		for(;i!=j;++i) {
			std::cerr << "ast node " << (*i)->raw.ref << " refs " << (*i) << std::endl;
			if((*i)->raw.ref==1) {
				to_remove.push_back(*i);
			}
		}
		l=to_remove.end();
		for(k=to_remove.begin();k!=l;++k) {
			delete_node(*k);
		}
	}
#endif
}

static volatile int is_init=0;

void tinyap_terminate() {
	if(!is_init) {
		return;
	}
	is_init=0;
	node_pool_term();
	term_pilot_manager();
	grammar::item::clean_registry_at_exit();
	flush_nodes();
	deinit_strreg();
	term_tinyap_alloc();
}

void tinyap_init() {
	if(is_init) {
		return;
	}
	is_init=1;
	init_tinyap_alloc();
	init_strreg();
	node_pool_init();
	/*atexit(tinyap_terminate);*/
	init_pilot_manager();
//	fprintf(stderr, "after  tinyap_init : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
}



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
	fprintf(stderr, "before tinyap_delete : %u nodes (%u alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	if(t->A) delete t->A;
	if(t->G) delete t->G;

	if(t->grammar_source) free(t->grammar_source);
	/*delete_node(t->grammar);*/
	/* start was inside grammar */

	/*delete_node(t->output);*/

	if(t->ws) free(t->ws);
	if(t->ws_source) free(t->ws_source);

	if(t->source_file) free(t->source_file);
	if(t->source_buffer) free(t->source_buffer);

	/*free(t);*/
	delete t;
	fprintf(stderr, "after tinyap_delete : %i allocs / %i deallocs\n", _node_alloc_count, _node_dealloc_count);
}

tinyap_t tinyap_new() {
	tinyap_t ret=new _tinyap_t();
	/*tinyap_t ret=(tinyap_t)malloc(sizeof(struct _tinyap_t));*/
	/*memset(ret,0,sizeof(struct _tinyap_t));*/
	tinyap_set_grammar(ret,"short");
	ret->flags=0;
	/*init_pilot_manager();*/
	return ret;
}


void ast_serialize_to_file(const ast_node_t ast,FILE*f);
const char* ast_serialize_to_string(const ast_node_t ast, int);

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
	return ast_serialize_to_string(n, 1);
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
	/*fprintf(stderr, "has set whitespace RE to %s\n",t->ws);*/
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
	/*fprintf(stderr, "has set whitespace RE to %s\n",t->ws);*/
}

const char* tinyap_get_grammar(tinyap_t t) {
	return t->grammar_source;
}

/* common to set_grammar and set_grammar_ast */
void init_grammar(tinyap_t t) {
	if(t->G) { delete t->G; }
	if(t->A) { delete t->A; t->A = NULL; }
	t->G = new grammar::Grammar(Cdr(Car((ast_node_t)t->grammar)));
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
	/*delete_node(t->grammar);*/
	t->grammar=tinyap_get_ruleset(g);
	/*t->grammar->raw.ref++;*/
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
	/*delete_node(t->grammar);*/
	t->grammar=g;
	/*t->grammar->raw.ref++;*/
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
				t->pos_cache.reset("");
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
			t->pos_cache.reset(t->source_buffer);

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
			/*fprintf(stderr, "stdin input currently disabled. Sorry for the inconvenience.\n");*/
			/*abort();*/
			t->pos_cache.reset(t->source_buffer, t->source_buffer_sz);
		}
		if(tinyap_verbose) {
			gettimeofday(&t0, NULL);
			fprintf(stderr, "took %.3f seconds to read file contents.\n", 1.e-6f*(t0.tv_usec-t1.tv_usec)+t0.tv_sec-t1.tv_sec);
		}
	} else {
		tinyap_set_source_buffer(t,"",0);
		t->pos_cache.reset("");
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
	/*fprintf(stderr, "Tinyap is about to parse this buffer (%u characters long) :\n"*/
		/*"================================================\n"*/
		/*"%*.*s\n"*/
		/*"================================================\n",*/
		/*sz, sz, sz, t->source_buffer);*/
	t->source_buffer_sz=sz;
}

int tinyap_parse(tinyap_t t, int full) {
//	fprintf(stderr, "before tinyap_parse : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
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

	/*if(t->output) {*/
		/*delete_node(t->output);*/
	/*}*/

	if(tinyap_verbose) {
		gettimeofday(&t0, NULL);
		fprintf(stderr, "took %.3f seconds to init parsing context.\n", 1.e-6f*(t0.tv_usec-t1.tv_usec)+t0.tv_sec-t1.tv_sec);
	}

	gettimeofday(&t0, NULL);
	t->output = t->A->parse(t->source_buffer, t->source_buffer_sz, !!full);
	/*t->output->raw.ref++;*/
	/*t->output = token_produce_any(t->toktext, t->start, NULL);*/
	/*t->output = pda_parse(t->pda, t->source_buffer, t->source_buffer_sz, t->start, t->flags);*/
	gettimeofday(&t1, NULL);

	if(tinyap_verbose) {
		/*fprintf(stderr, "TinyaP parsed %u of %u characters.\n",t->toktext->farthest,t->toktext->size);*/
		fprintf(stderr, "TinyaP parsed %u of %u characters.\n",t->A->furthest,t->source_buffer_sz);
	}
//	token_context_free(t->toktext);
//	t->toktext=NULL;

	fprintf(stderr, "after  tinyap_parse : %u nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	deltasec = t1.tv_sec-t0.tv_sec;
	t->parse_time = 1.e-6f*(t1.tv_usec-t0.tv_usec)+deltasec;

	return (t->error=(t->output!=NULL));
	
}


int tinyap_parse_as_grammar(tinyap_t t) {
	if(tinyap_parse(t, false)) {
		tinyap_set_grammar_ast(t,tinyap_get_output(t));
		/*delete_node(t->output);*/
		t->output=NULL;
	}
	return t->error;
}

/*int tinyap_parsed_ok(const tinyap_t t) { return t->error||(t->toktext->ofs!=t->source_buffer_sz); }*/
int tinyap_parsed_ok(const tinyap_t t) { return t->error||(t->A->furthest!=t->source_buffer_sz); }

ast_node_t tinyap_get_output(const tinyap_t t) { return t->output; }
void tinyap_set_output(const tinyap_t t, ast_node_t o) { t->output = o; }

#if 1
int tinyap_get_error_col(const tinyap_t t) {
	_tinyap_t::position_t p = t->pos_cache.position_of(t->A->furthest);
	return p.col();
	return -1 /*parse_error_column(t->context)*/;
}
int tinyap_get_error_row(const tinyap_t t) {
	_tinyap_t::position_t p = t->pos_cache.position_of(t->A->furthest);
	return p.row();
	return -1 /*parse_error_line(t->context)*/;
}
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
	/*if(n->node_flags&ATOM_IS_NOT_STRING) {*/
		/*ret = op2string((int)ret);*/
	/*}*/
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

int tinyap_node_get_row(tinyap_t parser, const ast_node_t n) {
	if(!n) {
		return 0;
	}
	switch(n->type) {
		case ast_Atom:
			return parser->pos_cache.position_of(getOffset(n)).row();
		case ast_Pair:
			return tinyap_node_get_row(parser, Car(n));
		default:
			return 0;
	};
}

int tinyap_node_get_col(tinyap_t parser, const ast_node_t n) {
	if(!n) {
		return 0;
	}
	switch(n->type) {
		case ast_Atom:
			return parser->pos_cache.position_of(getOffset(n)).col();
		case ast_Pair:
			return tinyap_node_get_col(parser, Car(n));
		default:
			return 0;
	};
}


ast_node_t tinyap_make_ast(const wast_t n) {
	return make_ast((wast_t)n);
}


wast_t tinyap_make_wast(tinyap_t t, const ast_node_t n) {
	return make_wast(t, (ast_node_t)n);
}

void tinyap_free_wast(const wast_t w) {
	wa_del(w);
}


void* tinyap_walk(const wast_t subtree, const char* pilot_name, void* pilot_init_data) {
	return do_walk(subtree,pilot_name,pilot_init_data);
}


/* FIXME : il faut réécrire ça façon fonctionnel récursif
 * pour ne pas hacker un ast_node_t ce qui foutrait tout tinyap en l'air
 * et tant qu'à faire relâcher la contrainte sur le type du rmember :
 * - si c'est un alt, cool, on ajoute la nouvelle alternative à la fin (donc on reconstruit tout le alt en fait)
 *   est-ce que c'est gênant si le plugin est collé au début ?
 *   	l'ordre n'importe pas pour le parsing
 *   	l'ordre ne devrait pas importer pour l'unparsing
 *   	=> reconstruire Pair(Atom("Alt"), Pair(plugin, Cdr(rmember)))
 * - si c'est pas un alt, on crée un alt avec (old_rmb, plugin)
 */

ast_node_t rec_plug_node(ast_node_t node, const char* plug, ast_node_t pin, bool& found_plug) {
	ast_node_t x = Car(node);
	const char* tag = Value(Car(x));
	ast_node_t cdr = Cdr(x);
	if(!(strcmp(tag, STR_OperatorRule)&&strcmp(tag, STR_TransientRule))) {
		tag = Value(Car(cdr));
		if(!strcmp(tag, plug)) {
			ast_node_t backup = node;
			node = newPair(	newPair(Car(x), newPair(Car(cdr), rec_plug_node(Cdr(cdr), plug, pin, found_plug))),
							rec_plug_node(Cdr(node), plug, pin, found_plug));
			delete_node(backup);
			found_plug = true;
		}
	} else if(strcmp(tag, STR_Comment)) {
		/* If NOT on Comment or *Rule, it is a rmember */
		if(!strcmp(tag, STR_Alt)) {
			/* On Alt, insert at head */
			ast_node_t backup = node;
			while(backup && Car(backup)!=pin) { backup = Cdr(backup); }		/* if the alternative already exists, just skip */
			if(!backup) {													/* otherwise, insert */
				backup = node;
				node = newPair(	newPair(newAtom(STR_Alt, 0), newPair(pin, Cdr(x))),
								rec_plug_node(Cdr(node), plug, pin, found_plug));
				delete_node(backup);
			}
		} else {
			ast_node_t backup = node;
			node = newPair(	newPair(newAtom(STR_Alt, 0), newPair(pin, x)),
							rec_plug_node(Cdr(node), plug, pin, found_plug));
			delete_node(backup);
		}
	}
	return node;
}

void tinyap_plug_node(tinyap_t parser, ast_node_t pin, const char* plugin, const char* plug) {
	bool found_plug = false;
	ast_node_t ret = rec_plug_node(
							tinyap_list_get_element(tinyap_get_grammar_ast(parser), 0),
							plug,
							newPair(newAtom(STR_NT, 0), newAtom(plugin, 0)),
							found_plug);
	if(!found_plug) {
		fprintf(stderr,"tinyap: can't plug %s into %s : %s doesn't exist.\n",plugin,plug,plug);
	} else if(ret==tinyap_get_grammar_ast(parser)) {
		/* if the rule was found and nothing happened, it must be because the new (NT plugin) already existed as an alternative in that rule... */
		fprintf(stderr,"tinyap: can't plug %s into %s : alternative already exists in rule.\n",plugin,plug);
	} else {
		parser->grammar = NULL;		/* the node has already been deleted in rec_plug_node */
		tinyap_set_grammar_ast(parser, ret);
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
	parser->grammar = append(Car((ast_node_t)parser->grammar), Cdr(supp));
}

} /* extern "C" */


