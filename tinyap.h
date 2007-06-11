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

#ifndef _TINYAP_H__
#define _TINYAP_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifndef _TINYAP_AST_H__
	typedef union _ast_node_t* ast_node_t;
#endif
	typedef struct _tinyap_t* tinyap_t;

	tinyap_t	tinyap_parse(const char*input,const char*grammar);

	int		tinyap_parsed_ok(const tinyap_t);
	ast_node_t*	tinyap_get_output(const tinyap_t);

	int		tinyap_get_error_col(const tinyap_t);
	int		tinyap_get_error_row(const tinyap_t);
	const char*	tinyap_get_error(const tinyap_t);

	void		tinyap_free(tinyap_t);

#ifdef __cplusplus
}

#warn TODO : C++ bindings

namespace TinyaP {

	class AST {
		class Node {
		private:
			
		public:
			Node() {
		}
	}

	class Parser {
	}
};


#endif
#endif


