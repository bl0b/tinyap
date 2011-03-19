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

#ifndef _TINYAP_TOKEN_UTILS_H_
#define _TINYAP_TOKEN_UTILS_H_


#include "ast.h"
#include "parse_context.h"

#ifdef __cplusplus
extern "C" {
#endif


#include <pcre.h>
#define RE_TYPE pcre*

#define OP_EOF       1 // immed
#define OP_RE        2 // immed, append or fail
#define OP_T         3 // immed, skip or fail
#define OP_RTR       4 // enter, produce subseq, leave, append or fail
#define OP_ROP       5 // enter, get tag, produce, make [tag], leave, append or fail
#define OP_PREFX     6 // enter, produce car, produce cadr, leave, prefix, append or fail
#define OP_NT        7 // select rule, goto OP_RTR or OP_ROP, append or fail
#define OP_SEQ       8 // SEQUENCE. produce all elements or fail, append
#define OP_ALT       9 // BREAKPOINT. push breakpoint, enter, produce or fail, append
#define OP_POSTFX   10 // enter, produce car, produce cadr, leave, postfix, append or fail
#define OP_RAWSEQ   11 // SEQUENCE. (no whitespace trimming)
#define OP_REP_0N   12 // enter, try produce any times, leave, append
#define OP_REP_01   13 // enter, try produce, leave, append
#define OP_REP_1N   14 // enter, produce, produce any times, leave, append or fail
#define OP_EPSILON  15 // skip
#define OP_RPL      16 // OBSOLETED.
#define OP_STR      17 // immed, append or fail
#define OP_BOW      18 // immed, keep ? append : skip
#define OP_ADDTOBAG 19 // immed, keep ? append : skip
#define OP_BKEEP    20 // .


static inline ast_node_t SafeAppend(ast_node_t a, ast_node_t b) {
	return a==PRODUCTION_OK_BUT_EMPTY
		? b
		: b==PRODUCTION_OK_BUT_EMPTY
			? a
			: Append(a, b);
}

const char* op2string(int typ);
int string2op(const char* tag);

ast_node_t find_nterm(ast_node_t grammar, const char* tag);

unsigned long match_bow(parse_context_t pda, char* name);
void token_bow_add(parse_context_t pda, char* name, char* word);

RE_TYPE token_regcomp(const char*reg_expr);
char*match2str(const char*src,const size_t start,const size_t end);

ast_node_t copy_node(ast_node_t);
void escape_ncpy(char**dest, char**src, int count, int delim);




static inline int re_exec(const RE_TYPE re, const char*source, unsigned int offset, unsigned int size, int* matches, int sz) {
	int ret = pcre_exec(re, NULL, source+offset, size-offset, 0, PCRE_ANCHORED, matches, sz);
	/*printf("RE DEBUG : match %p against \"%10.10s\"\n", re, pda->source+pda->ofs);*/
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


#ifdef __cplusplus
}
#endif

#endif

