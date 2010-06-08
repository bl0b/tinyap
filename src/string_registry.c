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
#include "hashtab.h"
#include "string_registry.h"
#include <stdlib.h>
#include <stdio.h>

struct _hashtable str_registry;

char* STR__start = NULL;
char* STR__whitespace = NULL;
char* STR_Grammar = NULL;
char* STR_Comment = NULL;
char* STR_eos = NULL;
char* STR_Alt = NULL;
char* STR_Seq = NULL;
char* STR_NT = NULL;
char* STR_Epsilon = NULL;
char* STR_EOF = NULL;
char* STR_Rep0N = NULL;
char* STR_Rep01 = NULL;
char* STR_Rep1N = NULL;
char* STR_RE = NULL;
char* STR_RPL = NULL;
char* STR_T = NULL;
char* STR_Prefix = NULL;
char* STR_Postfix = NULL;
char* STR_TransientRule = NULL;
char* STR_OperatorRule = NULL;
char* STR_Space = NULL;
char* STR_NewLine = NULL;
char* STR_Indent = NULL;
char* STR_Dedent = NULL;
char* STR_strip_me = NULL;

static inline unsigned int _srh(char*notnull) {
	unsigned int accum = 0;
	while(*notnull) {
		accum = (accum<<5)^((accum>>27) | (int)*notnull);
		notnull+=1;
	}
	return accum%HASH_SIZE;
}

unsigned int strreg_h(char*str) {
	return str?_srh(str):0;
}

void init_strreg() {
	init_hashtab(&str_registry, (hash_func) strreg_h, (compare_func) strcmp);
	/* pre-fill registry with all hardcoded strings used in the tokenizer */
	STR__whitespace		= regstr("_whitespace");
	STR__start		= regstr("_start");
	STR_Grammar		= regstr("Grammar");
	STR_Comment		= regstr("Comment");
	STR_eos			= regstr("eos");
	STR_Alt			= regstr("Alt");
	STR_Seq			= regstr("Seq");
	STR_NT			= regstr("NT");
	STR_Epsilon		= regstr("Epsilon");
	STR_EOF			= regstr("EOF");
	STR_Rep0N		= regstr("Rep0N");
	STR_Rep01		= regstr("Rep01");
	STR_Rep1N		= regstr("Rep1N");
	STR_RE			= regstr("RE");
	STR_RPL			= regstr("RPL");
	STR_T			= regstr("T");
	STR_Prefix		= regstr("Prefix");
	STR_Postfix		= regstr("Postfix");
	STR_TransientRule	= regstr("TransientRule");
	STR_OperatorRule	= regstr("OperatorRule");
	STR_Space		= regstr("Space");
	STR_NewLine		= regstr("NewLine");
	STR_Indent		= regstr("Indent");
	STR_Dedent		= regstr("Dedent");
	STR_strip_me		= regstr("strip.me");
	/*atexit(deinit_strreg);*/
}

char* regstr(const char* str) {
	htab_entry_t e = hash_find_e(&str_registry, (hash_key) str);
	char* ret;
	if(!e) {
		ret = _strdup(str);
		hash_addelem(&str_registry, (hash_key) ret, (hash_elem) 1);
		/*printf("register string \"%s\" (1)\n", ret);*/
	} else {
		e->e = (hash_elem) (((unsigned int)e->e)+1);
		ret = (char*) e->key;
		/*printf("register string \"%s\" (%u)\n", ret, (unsigned int)e->e);*/
	}
	return ret;
}

void unregstr(const char* str) {
	htab_entry_t e = hash_find_e(&str_registry, (hash_key) str);
	if(!e) {
		/* whine ? */
		return;
	}
	e->e = (hash_elem) (((unsigned int)e->e)-1);
	if(!e->e) {
		char*k = e->key;
		hash_delelem(&str_registry, (hash_key) str);
		_strfree(k);
	}
}

void _free_key(htab_entry_t e) {
	_strfree(e->key);
}

void deinit_strreg() {
	clean_hashtab(&str_registry, _free_key);
}

