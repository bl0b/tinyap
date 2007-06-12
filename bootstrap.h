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
#ifndef _TINYAP_BOOTSTRAP_H__
#define _TINYAP_BOOTSTRAP_H__

/*! \brief AST Node public type
 */
typedef union _ast_node_t* ast_node_t;

#define GRAMMAR_EXPLICIT "explicit"
#define GRAMMAR_CAMELCASING "CamelCasing"

ast_node_t  get_ruleset(const char*name);


#endif
