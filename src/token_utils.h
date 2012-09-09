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


ast_node_t find_nterm(ast_node_t grammar, const char* tag);

unsigned long match_bow(parse_context_t pda, char* name);
void token_bow_add(parse_context_t pda, char* name, char* word);

RE_TYPE token_regcomp(const char*reg_expr);
char*match2str(const char*src,const size_t start,const size_t end, const char* long_delim);

ast_node_t copy_node(ast_node_t);
void escape_ncpy(char**dest, char**src, int count, const char* delim);




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

