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
#include "ast.h"
#include "bootstrap.h"
#include "tokenizer.h"
#include "tinyap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

struct _tinyap_t {
	token_context_t*toktext;
	
	char*grammar_source;
	ast_node_t grammar;
	ast_node_t start;

	ast_node_t output;

	char*ws;
	char*ws_source;

	char*source_file;
	char*source_buffer;
	size_t source_buffer_sz;

	int error;
};

void tinyap_delete(tinyap_t t) {
	if(t->toktext) token_context_free(t->toktext);

	if(t->grammar_source) free(t->grammar_source);
	if(t->grammar) delete_node(t->grammar);
	/* start was inside grammar */

	if(t->output) delete_node(t->output);

	if(t->ws) free(t->ws);
	if(t->ws_source) free(t->ws_source);

	if(t->source_file) free(t->source_file);
	if(t->source_buffer) free(t->source_buffer);
	free(t);
}


tinyap_t tinyap_new() {
	tinyap_t ret=(tinyap_t)malloc(sizeof(struct _tinyap_t));
	memset(ret,0,sizeof(struct _tinyap_t));
	tinyap_set_grammar(ret,"explicit");
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
	t->ws_source=strdup(ws);
	t->ws=(char*)malloc(strlen(t->ws_source)+1);
	sprintf(t->ws,"[%s]+",t->ws_source);
}

void tinyap_set_whitespace_regexp(tinyap_t t,const char*re) {
	if(t->ws_source) {
		free(t->ws_source);
		t->ws_source=NULL;
	}
	t->ws=strdup(re);
}

const char* tinyap_get_grammar(tinyap_t t) {
	return t->grammar_source;
}

/* common to set_grammar and set_grammar_ast */
void init_grammar(tinyap_t t) {
	ast_node_t ws_node=find_nterm(t->grammar,"_whitespace");
	t->start=find_nterm(t->grammar,"_start");
	if(!t->start) {
		t->start=getCar(getCdr(t->grammar));
	}
	if(ws_node) {
		char*ws_tag;
		ast_node_t ws_node=getCar(getCdr(getCdr(ws_node)));

		ws_tag=Value(getCar(ws_node));

		if(!strcmp(ws_tag,"T")) {
			tinyap_set_whitespace(t,Value(getCar(getCdr(ws_node))));
		} else if(!strcmp(ws_tag,"RE")) {
			tinyap_set_whitespace_regexp(t,Value(getCar(getCdr(ws_node))));
		}
	}
	if(t->ws==NULL) {
		tinyap_set_whitespace(t," \t\r\n");
	}
}

void tinyap_set_grammar(tinyap_t t,const char*g) {
	if(t->grammar_source) {
		free(t->grammar_source);
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

void tinyap_set_source_file(tinyap_t t,const char*fnam) {
	if(fnam) {
		struct stat st;
		FILE*f;

		if(t->source_file) {
			free(t->source_file);
		}
		t->source_file=strdup(fnam);

		if(strcmp(fnam,"stdin")&&strcmp(fnam,"-")) {
			if(stat(t->source_file,&st)) {
				/* error */
				perror("stat");
				tinyap_set_source_buffer(t,NULL,0);
			} else {
				f=fopen(t->source_file,"r");
			}
		} else {
			f = stdin;
		}
	
		t->source_buffer=(char*)malloc(st.st_size);
		t->source_buffer_sz=st.st_size;
			
		fread(t->source_buffer,1,st.st_size,f);
		if(f!=stdin) {
			fclose(f);
		}
	} else {
		tinyap_set_source_buffer(t,NULL,0);
	}
}

void tinyap_set_source_buffer(tinyap_t t,const char* b,const unsigned int sz) {
	if(t->source_file) {
		free(t->source_file);
		t->source_file=NULL;
	}
	t->source_buffer=(char*)malloc(sz);
	strncpy(t->source_buffer,b,sz);
	t->source_buffer_sz=sz;
	
}


int tinyap_parse(tinyap_t t) {
	if(t->toktext) {
		token_context_free(t->toktext);
	}

	t->toktext=token_context_new(
			t->source_buffer,
			t->source_buffer_sz,
			t->ws,
			t->grammar,
			STRIP_TERMINALS);

	t->output=clean_ast(
			token_produce_any(
				t->toktext,
				t->start,
				0));

	return (t->error=(t->output!=NULL));
	
}


int tinyap_parse_as_grammar(tinyap_t t) {
	if(tinyap_parse(t)) {
		tinyap_set_grammar_ast(t,tinyap_get_output(t));
		t->output=NULL;
	}
	return t->error;
}

int tinyap_parsed_ok(const tinyap_t t) { return t->error; }

ast_node_t tinyap_get_output(const tinyap_t t) { return t->output; }

int tinyap_get_error_col(const tinyap_t t) { return parse_error_column(t->toktext); }
int tinyap_get_error_row(const tinyap_t t) { return parse_error_line(t->toktext); }
const char* tinyap_get_error(const tinyap_t t) { return parse_error(t->toktext); }

int tinyap_node_is_nil(const ast_node_t  n) {
	return n==NULL;
}

int tinyap_node_is_list(const ast_node_t  n) {
	return isPair(n);
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
	return Value(n);
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



