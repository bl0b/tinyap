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

#ifndef __TINYAP_NODE_CACHE_H__
#define __TINYAP_NODE_CACHE_H__

#include "ast.h"

/*#define NODE_CACHE_SIZE 32768*/
/*#define NODE_CACHE_SIZE 32749*/
/*#define NODE_CACHE_SIZE 65533*/
#define NODE_CACHE_SIZE 65521
/*#define NODE_CACHE_SIZE 33333*/
/*#define NODE_CACHE_SIZE 8191		// Mersenne prime*/

typedef struct _node_cache_entry_t* node_cache_entry_t;

typedef node_cache_entry_t node_cache_t[NODE_CACHE_SIZE];

void node_cache_init(node_cache_t cache);
int node_cache_retrieve(node_cache_t cache, int l, int c, const char* rule, ast_node_t* node_p, size_t* ofs);
void node_cache_add(node_cache_t cache, int l, int c, const char* expr_op, ast_node_t node, size_t ofs);
void node_cache_flush(node_cache_t cache);


#endif

