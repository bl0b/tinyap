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

#include "walkableast.h"

typedef struct _stack_t {
	size_t sz;
	size_t sp;
	wast_t* stack;
}* stack_t;

stack_t new_stack();
void push(stack_t s, wast_t w);
wast_t pop(stack_t s);
wast_t peek(stack_t s);
void free_stack(stack_t s);
#endif

