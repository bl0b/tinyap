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

#ifndef _TINYAP_STR_REG_H_
#define _TINYAP_STR_REG_H_

#include <string.h>
#include "tinyap_alloc.h"

void init_strreg();
char* regstr(const char*);
void unregstr(const char* str);
void deinit_strreg();

#define TINYAP_STRCMP(_a, _b) ( (_a) != (_b) )


static inline char* _stralloc(unsigned long l) {
	struct __allocator* A = _select_alloca(l);
	return A?_alloc(A):malloc(l);
}

static inline char* _strdup(const char*str) {
	unsigned long l = strlen(str)+1;
	return memcpy(_stralloc(l), str, l);
}

static inline void _strfree(char*str) {
	register unsigned long l = strlen(str)+1;
	struct __allocator* A = _select_alloca(l);
	A ? _free(A, str) : free(str);
}


extern char* STR__start;
extern char* STR__whitespace;
extern char* STR_Grammar;
extern char* STR_Comment;
extern char* STR_eos;
extern char* STR_Alt;
extern char* STR_Seq;
extern char* STR_NT;
extern char* STR_Epsilon;
extern char* STR_EOF;
extern char* STR_Rep0N;
extern char* STR_Rep01;
extern char* STR_Rep1N;
extern char* STR_RE;
extern char* STR_RPL;
extern char* STR_T;
extern char* STR_Prefix;
extern char* STR_Postfix;
extern char* STR_TransientRule;
extern char* STR_OperatorRule;
extern char* STR_Space;
extern char* STR_NewLine;
extern char* STR_Indent;
extern char* STR_Dedent;
extern char* STR_strip_me;

#endif
