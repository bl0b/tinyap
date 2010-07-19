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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _TINYAP_TOKENIZER_H_
#define _TINYAP_TOKENIZER_H_

/*#include <regex.h>*/
#include <pcre.h>

#define RE_TYPE pcre*

#include "stack.h"

struct _pos_cache_t {
	int last_ofs;
	int last_nlofs;
	int row;
	int col;
};

#include "node_cache.h"


#define OFSTACK_SIZE 4096

#define STRIP_TERMINALS 1
#define INPUT_IS_CLEAN 2

#define __fastcall __attribute__((fastcall))
typedef struct _token_context_t {
	char*source;
	size_t size;
	size_t flags;
	size_t ofs;
	RE_TYPE garbage;
	size_t ofstack[OFSTACK_SIZE];
	size_t ofsp;
	size_t farthest;
	tinyap_stack_t farthest_stack;
	tinyap_stack_t node_stack;
	tinyap_stack_t raw_stack;
	ast_node_t expected;
	ast_node_t grammar;
	struct _pos_cache_t pos_cache;
	node_cache_t cache;
} token_context_t;


ast_node_t __fastcall token_produce_any(token_context_t*t,ast_node_t expr);
ast_node_t __fastcall find_nterm(const ast_node_t ruleset,const char*ntermid);
ast_node_t __fastcall clean_ast(ast_node_t t);

ast_node_t __fastcall token_produce_re(token_context_t*t,const RE_TYPE expr);
ast_node_t __fastcall token_produce_str(token_context_t*t,const char*token);

RE_TYPE token_regcomp(const char*reg_expr);
token_context_t*token_context_new(const char*src,const size_t length,const char*garbage_regex,ast_node_t greuh,size_t drapals);
/*size_t token_context_peek(const token_context_t*t);
void token_context_push(token_context_t*t);
void token_context_validate(token_context_t*t);
void token_context_pop(token_context_t*t);
*/
void token_context_free(token_context_t*t);

const char* parse_error(token_context_t*t);
int parse_error_line(token_context_t*t);
int parse_error_column(token_context_t*t);

#define MAX_TOK_SIZE 256

#if 0
static inline int regexec_hack(RE_TYPE re, token_context_t* t, int count, regmatch_t* tokens, int flags) {
	int ret;
	if(t->ofs+256<t->size) {
		char* ptr = t->source+t->ofs+256;
		char bak = *ptr;
		*ptr = 0;
		ret = regexec(re, t->source+t->ofs, count, tokens, flags);
		*ptr = bak;
	} else {
		ret = regexec(re, t->source+t->ofs, count, tokens, flags);
	}
	return ret;	
}
#endif

/*#define re_exec(_re, _t, _m, _sz) (pcre_exec(_re, NULL, (_t)->source, (_t)->size, (_t)->ofs, PCRE_DOLLAR_ENDONLY|PCRE_NEWLINE_ANY, _m, _sz)>=0)*/

static inline int re_exec(const RE_TYPE re, token_context_t* t, int* matches, int sz) {
	int ret = pcre_exec(re, NULL, t->source+t->ofs, t->size-t->ofs, 0, PCRE_ANCHORED, matches, sz);
	/*printf("RE DEBUG : match %p against \"%10.10s\"\n", re, t->source+t->ofs);*/
	if(ret<0) {
		/*printf("PCRE error %i\n", ret);*/
		return 0;
	}
	if(matches[0]!=0) {
		printf("PCRE match not at start (%i)\n", matches[0]);
		return 0;
	}
	return 1;
}

void __fastcall update_pos_cache(token_context_t*t);

static inline int token_context_is_raw(token_context_t*t) {
	return (int)_peek(t->raw_stack);
}

/* silent garbage filter, used before each tokenization */
static inline void _filter_garbage(token_context_t*t) {
	int token[3];
	if((!(token_context_is_raw(t)||(t->flags&INPUT_IS_CLEAN)))&&re_exec(t->garbage, t, token, 3)) {
		t->ofs+=token[1];
		update_pos_cache(t);
	}
	t->flags|=INPUT_IS_CLEAN;
}


#endif

