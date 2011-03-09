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
#include "stack.h"
#include "string_registry.h"
#include <stdlib.h>
#include <string.h>


struct _walkable_ast_t {
	const char* label;
	wast_t father;
	unsigned int opd_count;
	wast_t* operands;
	ast_node_t*node;
	int l,c;
};


wast_t wa_new(const char* op, int l, int c) {
	wast_t ret = (wast_t)malloc(sizeof(struct _walkable_ast_t));
	memset(ret,0,sizeof(struct _walkable_ast_t));
	/*ret->label = strdup(op);*/
	ret->label = regstr(op);
	ret->l = l;
	ret->c = c;
	return ret;
}

void wa_del(wast_t w) {
	if(wa_opd_count(w)>0) {
		int i;
		for(i=0;i<wa_opd_count(w);i++) {
			wa_del(wa_opd(w,i));
		}
		free(w->operands);
	}
	if(w->label) {
		/*free((char*)w->label);*/
		unregstr((char*)w->label);
	}
	free(w);
}

int wa_row(wast_t w) {
	return w->l;
}

int wa_col(wast_t w) {
	return w->c;
}

wast_t wa_father(wast_t w) {
	return w->father;
}

const char* wa_op(wast_t w) {
	return w->label?w->label:"";
}

wast_t wa_opd(wast_t w, const unsigned int n) {
	if(w->opd_count<=n) {
		return NULL;
	}
	return w->operands[n];
}

int wa_opd_count(wast_t w) {
	return w->opd_count;
}

int wa_is_leaf(wast_t l) {
	return l->operands==NULL;
}

void wa_add(wast_t f,wast_t s) {
	wast_t* tmp;
//	printf("wa_add %p %li\n",f->operands,f->opd_count+1);
	if(f->operands) {
		tmp = (wast_t*) realloc(f->operands,(f->opd_count+1)*sizeof(wast_t));
	} else {
		tmp = (wast_t*) malloc((f->opd_count+1)*sizeof(wast_t));
	}
	if(tmp) {
		f->operands = tmp;
		f->operands[f->opd_count] = s;
		f->opd_count += 1;
		s->father = f;
//	} else {
//		printf("failed.\n");
	}
}



wast_t make_wast(ast_node_t a) {
	wast_t ret;
	int i;
	int max;
	if(!a) {
		return NULL;
	}
	if(tinyap_node_is_string(a)) {
		ret = wa_new(tinyap_node_get_string(a),tinyap_node_get_row(a), tinyap_node_get_col(a));
	} else {
		ret = wa_new(tinyap_node_get_operator(a),tinyap_node_get_row(a), tinyap_node_get_col(a));
		max=tinyap_node_get_operand_count(a);
		for(i=0;i<max;i+=1) {
			/* FIXME : quadratic complexity in the AST size instead of linear complexity, it suxx. recursion over cdr(a) would perform better. */
			wa_add(ret,make_wast(tinyap_node_get_operand(a,i)));
		}
	}
	return ret;
}


ast_node_t make_ast(wast_t t) {
	int i;
	ast_node_t ret=NULL;
	if(!t) {
		return NULL;
	}
	if(wa_opd_count(t)==0) {
		return newAtom(wa_op(t),0);
	}
	for(i=wa_opd_count(t)-1;i>=0;i-=1) {
		ret = newPair( make_ast(wa_opd(t,i)), ret);
	}
	ret = newPair( newAtom(wa_op(t), wa_row(t)), ret);
	return ret;
}





struct _wast_iter_t {
	wast_t  root;
	wast_t  parent;
	size_t  child;
	tinyap_stack_t pstack;
	tinyap_stack_t cstack;
};


wast_iterator_t tinyap_wi_new(const wast_t root) {
	wast_iterator_t ret = (wast_iterator_t) malloc(sizeof(struct _wast_iter_t));
	memset(ret, 0, sizeof(struct _wast_iter_t));
	ret->root = root;
	ret->pstack = new_stack();
	ret->cstack = new_stack();
	return ret;
}

wast_iterator_t tinyap_wi_dup(const wast_iterator_t src) {
	wast_iterator_t ret = wi_new(wi_root(src));
	ret->child = src->child;
	ret->parent = src->parent;
	return ret;
}

wast_iterator_t tinyap_wi_reset(wast_iterator_t wi) {
	wi->parent = NULL;
	wi->child = 0;
	if(wi->pstack) {
		free_stack(wi->pstack);
	}
	if(wi->cstack) {
		free_stack(wi->cstack);
	}
	wi->pstack = new_stack();
	wi->cstack = new_stack();
	return wi;
}

void tinyap_wi_delete(wast_iterator_t wi) {
	free_stack(wi->pstack);
	free_stack(wi->cstack);
	free(wi);
}

wast_t tinyap_wi_node(wast_iterator_t wi) {
	if(wi->parent) {
		return wa_opd(wi->parent, wi->child);
	} else {
		return wi->child?NULL:wi->root;
	}
}

wast_t tinyap_wi_root(wast_iterator_t wi) {
	return wi->root;
}

wast_iterator_t tinyap_wi_up(wast_iterator_t wi) {
	if(wi->parent) {
		wast_t cur = wi->parent;
		wi->parent = wa_father(wi->parent);
		wi->child  = 0;
		if(wi->parent) {
			while(cur!=tinyap_wi_node(wi)) {
				tinyap_wi_next(wi);
			}
		}
	}
	return wi;
}

wast_iterator_t tinyap_wi_down(wast_iterator_t wi) {
	/*if(!tinyap_wi_on_leaf(wi)) {*/
		wi->parent = tinyap_wi_node(wi);
		wi->child  = 0;
	/*}*/
	return wi;
}

wast_iterator_t tinyap_wi_next(wast_iterator_t wi) {
	/*if(tinyap_wi_has_next(wi)) {*/
		wi->child += 1;
	/*}*/
	return wi;
}

int tinyap_wi_on_root(wast_iterator_t wi) {
	return !wi->parent;
}

int tinyap_wi_on_leaf(wast_iterator_t wi) {
	return !wa_opd_count(tinyap_wi_node(wi));
}

int tinyap_wi_has_next(wast_iterator_t wi) {
	if(wi->parent) {
		return ((int)wi->child) < ((int)(wa_opd_count(wi->parent)-1));
	} else {
		return 0;
	}
}

wast_iterator_t	tinyap_wi_backup(wast_iterator_t wi) {
	push(wi->pstack, (void*) wi->parent);
	push(wi->cstack, (void*) wi->child);
	return wi;
}

wast_iterator_t	tinyap_wi_restore(wast_iterator_t wi) {
	wi->parent = (wast_t) _pop(wi->pstack);
	wi->child = (size_t) _pop(wi->cstack);
	return wi;
}

wast_iterator_t	tinyap_wi_validate(wast_iterator_t wi) {
	_pop(wi->pstack);
	_pop(wi->cstack);
	return wi;
}

