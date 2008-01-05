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
#include "config.h"

#include "ast.h"
#include "tinyape.h"
#include "walker.h"
#include "stack.h"
#include "pilot_cache.h"
#include "pilot_manager.h"


void* walk(wast_t a, pilot_t p) {
	pilot_cache_elem_t pce;
	WalkDirection d;
	stack_t stack;
	stack_t ofs_stack;
	wast_t next=NULL;
	size_t opd=0;
	void* result;
//	int i;

	if(!p) {
		return NULL;
	}

	stack = new_stack();
	ofs_stack = new_stack();

	pce = p->p_type;

	d = do_visit(p,a);

	while(d!=Done&&d!=Error) {
//		printf(" ofs_stack : ");
//		for(i=0;i<ofs_stack->sz;i++) {
//			printf(" %li",(size_t)ofs_stack->stack[i]);
//		}
//		printf(" opd = %li\n",opd);
		switch(d) {
		case Down:
//			puts("Down");
			push(stack,a);
			push(ofs_stack,(void*)opd);
			opd=-1;
		case Next:
//			puts("Next");
			opd += 1;
			if(is_empty(stack)) {
				d = Done;
				break;
			} else if(opd<wa_opd_count(peek(wast_t,stack))) {
				next = wa_opd(peek(wast_t,stack),(unsigned int)opd);
				break;
			} else {
				//d=Up;
			}
		case Up:
//			puts("Up");
			next=NULL;
			/* either pop or go to father */
			if(not_empty(stack)) {
				_pop(stack);
				opd = pop(size_t,ofs_stack);
				d=Next;
			} else {
				//next = wa_father(a);
				d = Done;
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
		result = pce->result(p->data);
	} else {
		result = NULL;
	}
	free_pilot(p);
	free_stack(stack);
	free_stack(ofs_stack);
	return result;
}

void* do_walk(wast_t node, const char* pilot, void* init_data) {
	pilot_t p = new_pilot(pilot, init_data);
	return walk(node,p);
}

