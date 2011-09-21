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

#ifdef __cplusplus
extern "C" {
#endif
void init_strreg();
char* regstr_impl(const char*);
void unregstr(const char* str);
void deinit_strreg();
#ifdef __cplusplus
}
#endif

#define TINYAP_STRCMP(_a, _b) ( (_a) != (_b) )


static inline char* _stralloc_impl(unsigned long l) {
	struct __allocator* A = _select_alloca(l);
	return (char*) (A?_alloc(A):malloc(l));
}

static inline void _strfree_impl(char*str) {
	unsigned long l = strlen(str)+1;
	struct __allocator* A = _select_alloca(l);
#ifdef DEBUG_ALLOCS
	A ? _free_debug(A, str) : free(str);
#else
	A ? _free(A, str) : free(str);
#endif
}

#ifdef DEBUG_ALLOCS
static inline char* _stralloc_debug(unsigned long l, const char* f, size_t line, const char* what) {
	char* ret = _stralloc_impl(l);
	record_alloc(ret, f, l, what?what:"_stralloc");
	return ret;
}

static inline void _strfree_debug(char*str) {
	_strfree_impl(str);
}

static inline char* regstr_debug(const char* str, const char* f, size_t l) {
	char* ret = regstr_impl(str);
	record_alloc(ret, f, l, "regstr");
	return ret;
}

#define _stralloc(_l) _stralloc_debug(_l, __FILE__, __LINE__, "_stralloc")
#define _stralloc_what(_l, _w) _stralloc_debug(_l, __FILE__, __LINE__, _w)
#define _strfree(_s) _strfree_debug(_s)
#define regstr(_x_) regstr_debug(_x_, __FILE__, __LINE__)
#else
#define _stralloc(_l) _stralloc_impl(_l)
#define _strfree(_s) _strfree_impl(_s)
#define regstr(_x_) regstr_impl(_x_)
#endif

static inline char* _strdup(const char*str) {
	unsigned long l = strlen(str)+1;
	return (char*) memcpy(_stralloc(l), str, l);
}


extern char* STR__start;
extern char* STR__whitespace;
extern char* STR_Grammar;
extern char* STR_Comment;
extern char* STR_eos;
extern char* STR_Alt;
extern char* STR_RawSeq;
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
extern char* STR_STR;
extern char* STR_BOW;
extern char* STR_AddToBag;
extern char* STR_BKeep;
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
