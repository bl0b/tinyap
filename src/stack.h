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

typedef struct _stack_t {
	size_t sz;
	size_t sp;
	void** stack;
}* tinyap_stack_t;

tinyap_stack_t new_stack();
tinyap_stack_t stack_dup(tinyap_stack_t);
void push(tinyap_stack_t s, void* w);
void* _pop(tinyap_stack_t s);
#define pop(__t,__s) ((__t)_pop(__s))
void* _peek(tinyap_stack_t s);
#define peek(__t,__s) ((__t)_peek(__s))
void free_stack(tinyap_stack_t s);

#define is_empty(_s) (_s->sp==-1)
#define not_empty(_s) (_s->sp!=-1)

#endif

