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

#include <regex.h>

#define OFSTACK_SIZE 4096

#define STRIP_TERMINALS 1

typedef struct _token_context_t {
	char*source;
	size_t size;
	size_t flags;
	size_t ofs;
	regex_t*garbage;
	size_t ofstack[OFSTACK_SIZE];
	size_t ofsp;
	size_t farthest;
	ast_node_t grammar;
	struct {
		int last_ofs;
		int last_nlofs;
		int row;
		int col;
	} pos_cache;
} token_context_t;


ast_node_t  token_produce_any(token_context_t*t,ast_node_t expr,int strip_T);
ast_node_t find_nterm(const ast_node_t ruleset,const char*ntermid);
ast_node_t clean_ast(ast_node_t t);

ast_node_t token_produce_re(token_context_t*t,const regex_t*expr);
ast_node_t token_produce_str(token_context_t*t,const char*token);

regex_t*token_regcomp(const char*reg_expr);
token_context_t*token_context_new(const char*src,const size_t length,const char*garbage_regex,ast_node_t greuh,size_t drapals);
size_t token_context_peek(const token_context_t*t);
void token_context_push(token_context_t*t);
void token_context_validate(token_context_t*t);
void token_context_pop(token_context_t*t);
void token_context_free(token_context_t*t);

const char* parse_error(token_context_t*t);
int parse_error_line(token_context_t*t);
int parse_error_column(token_context_t*t);

/* silent garbage filter, used before each tokenization */
static inline void _filter_garbage(token_context_t*t) {
	regmatch_t token;
//	printf("\tdebug-- current string [[%10.10s...]]\n",t->source+t->ofs);
	if(regexec(t->garbage,t->source+t->ofs,1,&token,0)!=REG_NOMATCH) {
//		printf("\tdebug-- matched garbage [%i-%i]\n",token.rm_so,token.rm_eo);
		assert(token.rm_so==0);
		t->ofs+=token.rm_eo;
//		printf("\tdebug-- now ofs=%i\n",t->ofs);
	} else {
//		printf("\tdebug-- no garbage\n");
	}
//	printf("\tdebug-- now string is [[%10.10s...]]\n",t->source+t->ofs);
}




