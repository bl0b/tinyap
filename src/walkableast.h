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

#ifndef __TINYAP_WALKABLE_AST_H__
#define __TINYAP_WALKABLE_AST_H__

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WAST_DEFINED
	typedef struct _walkable_ast_t* wast_t;
	#define _WAST_DEFINED
#endif

wast_t wa_new(const char* op);
void wa_del(wast_t);
wast_t wa_father(wast_t);
const char* wa_op(wast_t);
int wa_opd_count(wast_t);
wast_t wa_opd(wast_t,const unsigned int);
void wa_add(wast_t,wast_t);

wast_t make_wast(ast_node_t a);
#ifdef __cplusplus
}
#endif

#endif

