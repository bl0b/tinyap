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

#include "../src/tinyape.h"
#include <stdlib.h>
#include <stdio.h>

//! internal state for tinycalc
typedef struct _tc_struc {
	//! evaluation result
	int result;
	//! flag for displaying result
	int is_toplevel;
}* tc_t;

//! initialization of tinycalc
void* ape_tinycalc_init(void* init_data) {
	tc_t tc = (tc_t) malloc(sizeof(struct _tc_struc));

	tc->is_toplevel = init_data==NULL;

	return tc;
}

//! fetch the evaluation result
void* ape_tinycalc_result(tc_t i) {
	if(i->is_toplevel) {
		printf("result = %i\n",i->result);
	}
	return (void*)i->result;
}

//! terminate tinycalc and free resources
void ape_tinycalc_free(tc_t data) {
	free(data);
}

//! default visit routine for unknown node types
WalkDirection ape_tinycalc_default(wast_t node, tc_t this) {
	printf("unknown node %s\n",wa_op(node));
	return Done;
}

//! node visit "number"
WalkDirection ape_tinycalc_number(wast_t node, tc_t this) {
	this->result = atoi(wa_op(wa_opd(node,0)));
	return Done;
}

//! node visit "m_add"
WalkDirection ape_tinycalc_m_add(wast_t node, tc_t this) {
	int a = (int)tinyap_walk(wa_opd(node,0),"tinycalc",this);
	int b = (int)tinyap_walk(wa_opd(node,1),"tinycalc",this);
	this->result = a + b;
	return Done;
}

//! node visit "m_sub"
WalkDirection ape_tinycalc_m_sub(wast_t node, tc_t this) {
	int a = (int)tinyap_walk(wa_opd(node,0),"tinycalc",this);
	int b = (int)tinyap_walk(wa_opd(node,1),"tinycalc",this);
	this->result = a - b;
	return Done;
}

//! node visit "m_mul"
WalkDirection ape_tinycalc_m_mul(wast_t node, tc_t this) {
	int a = (int)tinyap_walk(wa_opd(node,0),"tinycalc",this);
	int b = (int)tinyap_walk(wa_opd(node,1),"tinycalc",this);
	this->result = a * b;
	return Done;
}

//! node visit "m_div"
WalkDirection ape_tinycalc_m_div(wast_t node, tc_t this) {
	int a = (int)tinyap_walk(wa_opd(node,0),"tinycalc",this);
	int b = (int)tinyap_walk(wa_opd(node,1),"tinycalc",this);
	this->result = a / b;
	return Done;
}

//! node visit "m_minus"
WalkDirection ape_tinycalc_m_minus(wast_t node, tc_t this) {
	int a = (int)tinyap_walk(wa_opd(node,0),"tinycalc",this);
	this->result = - a;
	return Done;
}

//! node visit "m_expr"
WalkDirection ape_tinycalc_m_expr(wast_t node, tc_t this) {
	return Down;
}

