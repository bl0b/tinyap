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

#include <stdlib.h>
#include <string.h>
#include "stack.h"

stack_t new_stack() {
	stack_t ret = (stack_t) malloc(sizeof(struct _stack_t));
	memset(ret,0,sizeof(struct _stack_t));
	ret->sp=-1;
	return ret;
}

void push(stack_t s, void* w) {
	s->sp += 1;
	if(s->sz == s->sp) {
		s->sz+=16;
		s->stack = (void**) realloc(s->stack, s->sz*sizeof(void*));
	}
	s->stack[s->sp] = w;
}

void* _pop(stack_t s) {
	void* ret = s->stack[s->sp];
	s->sp -= 1;
	return ret;
}

void* _peek(stack_t s) {
	return s->stack[s->sp];
}

void free_stack(stack_t s) {
	if(s->stack) {
		free(s->stack);
	}
	free(s);
}


