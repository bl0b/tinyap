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
#include "ast.h"

volatile int depth=0;

volatile int _node_alloc_count=0;

#define _node(_tag,_contents) Cons(Atom(_tag),(_contents))

void delete_node(ast_node_t n) {
	if(!n) return;
	switch(n->type) {
	case ast_Atom:
		free(n->atom._str);
		break;
	case ast_Pair:
		delete_node(n->pair._car);
		delete_node(n->pair._cdr);
		break;
	case ast_Nil:;	/* so that -Wall won't complain */
	};
	_node_alloc_count-=1;
	free(n);
}



