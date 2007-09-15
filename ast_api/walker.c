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

#include "../ast.h"
#include "walkableast.h"
#include "walker.h"
#include "stack.h"
#include "pilot_cache.h"
#include "pilot_manager.h"


void* walk(wast_t a, pilot_t p) {
	pilot_cache_elem_t pce = p->p_type;
	WalkDirection d;
	stack_t stack = new_stack();
	wast_t next;
	int opd;

	d = do_visit(p,a);

	while(d!=Done&&d!=Error) {
		switch(d) {
		case Down:
			push(stack,a);
			opd=-1;
		case Next:
			opd += 1;
			if(opd<wa_opd_count(a)) {
				next = wa_opd(a,opd);
				break;
			} else {
				d=Up;
			}
		case Up:
			/* either pop or go to father */
			if(stack->sp) {
				pop(stack);
				d=Next;
				next=NULL;
			} else {
				next = wa_father(a);
			}
			break;
		default:;
		};
		if(next!=NULL) {
			a = next;
			d = do_visit(p,a);
		}
	}
	if(pce->result) {
		return pce->result(p->data);
	} else {
		return NULL;
	}
}

void* do_walk(wast_t node, const char* pilot, void* init_data) {
	pilot_t p = new_pilot(pilot, init_data);
	return walk(node,p);
}

