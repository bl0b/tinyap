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

#ifndef __TINYAP_WAST_STACK_H__
#define __TINYAP_WAST_STACK_H__

#include "tinyape.h"
#include <malloc.h>

typedef struct _stack_t {
	long sz;
	long sp;
	void** stack;
}* tinyap_stack_t;

#ifdef __cplusplus
extern "C" {
#endif
tinyap_stack_t new_stack();
tinyap_stack_t stack_dup(tinyap_stack_t);
tinyap_stack_t stack_clone(tinyap_stack_t, void*(*clone_entry)(void*));
/*void push(tinyap_stack_t s, void* w);*/
/*void* _pop(tinyap_stack_t s);*/
#define pop(__t,__s) ((__t)_pop(__s))
/*void* _peek(tinyap_stack_t s);*/
#define peek(__t,__s) ((__t)_peek(__s))
void free_stack(tinyap_stack_t s);

#define is_empty(_s) (_s->sp==-1)
#define not_empty(_s) (_s->sp!=-1)

static inline void push(tinyap_stack_t s, void* w) {
	s->sp += 1;
	if(s->sz <= s->sp) {
		/*s->sz+=16384;*/
		s->sz+=1024;
		s->stack = (void**) realloc(s->stack, s->sz*sizeof(void*));
		/*printf("resized stack %p to %lu\n", s, s->sz);*/
	}
	/*printf("push %X in %p at %lu\n", w, s, s->sp);*/
	s->stack[s->sp] = w;
}

static inline void* _pop(tinyap_stack_t s) {
	if(s->sp==-1)
		return NULL;
	return s->stack[s->sp--];
	/*void* ret = s->stack[s->sp];*/
	/*s->sp -= 1;*/
	/*return ret;*/
}

static inline void* _peek(tinyap_stack_t s) {
	if(s->sp==-1)
		return NULL;
	return s->stack[s->sp];
}

#ifdef __cplusplus
}
#endif

#endif

