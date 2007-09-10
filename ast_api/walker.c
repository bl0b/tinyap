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

