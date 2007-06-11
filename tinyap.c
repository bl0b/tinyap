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

struct _tinyap_t {
	token_context_t*toktext;
	ast_node_t*output;
	char*ws;
};



tinyap_t tinyap_parse(const char*input,const char*grammar) {
	tinyap_t ret=(tinyap_t)malloc(sizeof(struct _tinyap_t));
	ast_node_t*greuh=get_ruleset(grammar);
	ast_node_t*start=find_nterm(greuh,"_start");
	ast_node_t*whitespace=find_nterm(greuh,"_whitespace");
	char*ws_str;

	ret->ws=NULL;
	if(whitespace) {
		char*ws_tag;
		ast_node_t*ws_node=getCar(getCdr(getCdr(whitespace)));

		ws_tag=Value(getCar(ws_node));	/* d'uh */

		if(!strcmp(ws_tag,"T")) {
			ws_str=Value(getCar(getCdr(ws_node)));
			ret->ws=(char*)malloc(strlen(ws_str)+1);
			sprintf(ret->ws,"[%s]+",ws_str);
		} else if(!strcmp(ws_tag,"RE")) {
			ret->ws=Value(getCar(getCdr(ws_node)));
		}
	}
	if(ret->ws==NULL) {
		ret->ws=strdup("[ \t\r\n]+");
	}

	ret->toktext=token_context_new(input,strlen(input),ret->ws,greuh,STRIP_TERMINALS);
	if(!start) {
		start=getCar(getCdr(greuh));	/* default to first rule if _start was not specified */
	}
	ret->output=clean_ast(token_produce_any(ret->toktext,start,0));

	return ret;
}

int tinyap_parsed_ok(const tinyap_t t) { return t->output!=NULL; }

ast_node_t*tinyap_get_output(const tinyap_t t) { return t->output; }

int tinyap_get_error_col(const tinyap_t t) { return parse_error_column(t->toktext); }
int tinyap_get_error_row(const tinyap_t t) { return parse_error_line(t->toktext); }
const char* tinyap_get_error(const tinyap_t t) { return parse_error(t->toktext); }

void tinyap_free(tinyap_t t) {
	if(t->toktext) token_context_free(t->toktext);
	if(t->output) delete_node(t->output);
	if(t->ws) free(t->ws);
	free(t);
}

