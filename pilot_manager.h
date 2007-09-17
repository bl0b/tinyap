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

#ifndef __TINYAP_PILOT_MANAGER_H__
#define  __TINYAP_PILOT_MANAGER_H__

#include "ast.h"
#include "hashtab.h"
#include "walkableast.h"

typedef enum {
	Node,
	Root,
	InsideNode
} WalkOrigin;

typedef enum {
	Up,
	Down,
	Next,
	Done,
	Error
} WalkDirection;


typedef struct _pilot_t* pilot_t;

typedef WalkDirection (*node_visit_method) (wast_t node, void* pilot_data);

typedef struct _pilot_cache_elem_t {
	const char*name;
	void* dl_handle;
	void* (*init)(void*);
	void (*free)(pilot_t);
	void* (*result)(pilot_t);
	node_visit_method defaultMethod;
	hashtab_t methods;
}* pilot_cache_elem_t;

struct _pilot_t {
	pilot_cache_elem_t p_type;
	void* data;
};

pilot_t new_pilot(const char* p_name, void*init_data);

pilot_t get_pilot(const char* p_name);

void* get_pilot_data(pilot_t pilot);

node_visit_method get_visit_method(pilot_t p, const char* nodetype);

WalkDirection do_visit(pilot_t p, wast_t node);

void init_pilot_manager();


#endif

