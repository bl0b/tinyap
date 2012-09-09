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
#include "pda.h"

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

unsigned long match_bow(pda_t pda, char* name);
void token_bow_add(pda_t pda, char* name, char* word);

RE_TYPE token_regcomp(const char*reg_expr);
char*match2str(const char*src,const size_t start,const size_t end);

ast_node_t copy_node(ast_node_t);
void escape_ncpy(char**dest, char**src, int count, int delim);

#endif

