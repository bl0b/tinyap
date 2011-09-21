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

#ifndef __TINYAPE_H__
#define __TINYAPE_H__

#include "tinyap.h"

#ifndef _WAST_DEFINED
	//! opaque type for walkable AST
	typedef struct _walkable_ast_t* wast_t;
	#define _WAST_DEFINED
#endif

//! create a new AST node
wast_t wa_new(const char* op, int l, int c);
//! delete an AST node
void wa_del(wast_t);
//! get a node's father
wast_t wa_father(wast_t);
//! get a node's label
const char* wa_op(wast_t);
//! get a node's children count
int wa_opd_count(wast_t);
//! get a node's child
wast_t wa_opd(wast_t,const unsigned int);
//! add the right-hand child to the left-hand node
void wa_add(wast_t,wast_t);

//! get the line in source text that corresponds to this node
int wa_row(wast_t);
//! get the column in source text that corresponds to this node
int wa_col(wast_t);

//! transform a serializable AST into a walkable AST
wast_t make_wast(tinyap_t, ast_node_t a);
//! transform a walkable AST into a serializable AST
ast_node_t make_ast(wast_t a);

//! Walk directions to use in returns from visit methods
typedef enum {
	//! to father
	Up,
	//! to first child
	Down,
	//! to next sibling
	Next,
	//! success
	Done,
	//! failure
	Error
} WalkDirection;


//! abstract type for apes management
typedef struct _pilot_t* pilot_t;


#endif
