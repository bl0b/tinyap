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

#include "tinyap.h"
#include "walkableast.h"
#include <stdlib.h>
#include <string.h>


struct _walkable_ast_t {
	const char* label;
	wast_t father;
	unsigned int opd_count;
	wast_t* operands;
	ast_node_t*node;
};

wast_t wa_new(const char* op) {
	wast_t ret = (wast_t)malloc(sizeof(struct _walkable_ast_t));
	memset(ret,0,sizeof(struct _walkable_ast_t));
	ret->label = op;
	return ret;
}

void wa_del(wast_t w) {
	free(w);
}

wast_t wa_father(wast_t w) {
	return w->father;
}

const char* wa_op(wast_t w) {
	return w->label;
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
	if(tinyap_node_is_string(a)) {
		ret = wa_new(tinyap_node_get_string(a));
	} else {
		ret = wa_new(tinyap_node_get_operator(a));
		max=tinyap_node_get_operand_count(a);
		for(i=0;i<max;i+=1) {
			wa_add(ret,make_wast(tinyap_node_get_operand(a,i)));
		}
	}
	return ret;
}


