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
#ifndef _TINYAP_PDASM_H_
#define _TINYAP_PDASM_H_

#include <pcre.h>
#define RE_TYPE pcre*

#include "ast.h"
#include "hashtab.h"

struct _pos_cache_t {
	int last_ofs;
	int last_nlofs;
	int row;
	int col;
};


struct _pda_output_t {
	ast_node_t output;
	unsigned long final_offset;
	ast_node_t _cdr;
};

typedef struct _pda* pda_t;

pda_t pda_new(ast_node_t grammar, const char* whitespace);
void pda_free(pda_t);

ast_node_t pda_parse(pda_t pda, const char* source, unsigned long size, ast_node_t _start, int flags);

#define FULL_PARSE 1

int pda_parse_error_count(pda_t pda);
int pda_parse_error_row(pda_t pda, int);
int pda_parse_error_col(pda_t pda, int);
const char* pda_parse_error(pda_t pda, int);
ast_node_t pda_parse_error_expected(pda_t pda, int);

#endif

