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
#include "pilot_manager.h"
#include "walker.h"


void* ape_prettyprint2_init(void* init_data) { return NULL; }

void apt_prettyprint2_free(void*data) {}

void prettyprint2_node_header(wast_t father, wast_t node) {
	char c[4]={' ',' ',0,0};
	if(!father) {
		return;
	}
	if(!node) {
		c[1]='-';
		node=father;
		father=wa_father(node);
	}
	if(father) {
		prettyprint2_node_header(wa_father(father), father);
		if(wa_opd(father,wa_opd_count(father)-1)!=node) {
			c[0] = '|';
		} else {
			if(c[1]=='-') {
				c[0] = '`';
			}
		}
		fputs(c,stdout);
	}
}



WalkDirection ape_prettyprint2_default(wast_t node, void*___) {
	prettyprint2_node_header(node,NULL);
	puts(wa_op(node));

	if(wa_opd_count(node)) {
		return Down;
	} else {
		return Next;
	}
}
