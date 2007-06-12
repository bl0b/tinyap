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
#endif
	typedef struct _tinyap_t* tinyap_t;

#include "bootstrap.h"

	/*! \brief create a new parser with default context.
	 * create a parser with "explicit" metagrammar dialect, " \r\t\n" whitespaces, and input from stdin.
	 */
	tinyap_t	tinyap_new();

	/*! \brief access to whitespace characters list */
	const char*	tinyap_get_whitespace(tinyap_t);
	/*! \brief modification of whitespace characters list */
	void		tinyap_set_whitespace(tinyap_t,const char*);
	/*! \brief modification of whitespace recognition regexp */
	void		tinyap_set_whitespace_regexp(tinyap_t,const char*);

	/*! \brief access to grammar source */
	const char*	tinyap_get_grammar(tinyap_t);
	/*! \brief modification of grammar source (any filename, or "explicit" or "CamelCasing") */
	void		tinyap_set_grammar(tinyap_t,const char*);

	/*! \brief access to grammar ast */
	ast_node_t	tinyap_get_grammar_ast(tinyap_t);
	/*! \brief modification of grammar ast */
	void		tinyap_set_grammar_ast(tinyap_t,ast_node_t);

	/*! \brief access to filename of text input source (may be NULL) */
	const char*	tinyap_get_source_file(tinyap_t);
	/*! \brief access to buffer of text input source (i.e. contents of file or configured memory buffer) */
	const char*	tinyap_get_source_buffer(tinyap_t);
	/*! \brief access to length of text input source */
	unsigned int	tinyap_get_source_buffer_length(tinyap_t);
	/*! \brief set named file as text input
	 * CAUTION : if source filename is "-" or "stdin", standard input will be read until closed, and the function will block meanwhile.
	 */
	void		tinyap_set_source_file(tinyap_t,const char*);
	/*! \brief set buffer as text input source */
	void		tinyap_set_source_buffer(tinyap_t,const char*,const unsigned int);

	//void		

	/*! \brief perform parsing of configured source with configured grammar
	 * \return 1 if parsing was successful, 0 otherwise
	 */
	int tinyap_parse(tinyap_t);
	/*! \brief perform parsing of configured source with configured grammar and uses the output as the new grammar
	 * \return 1 if parsing was successful, 0 otherwise
	 */
	int tinyap_parse_as_grammar();

	/*! \brief predicate "the last parsing was successful"
	 * \return 1 if last parsing was successful, 0 otherwise
	 * equates to "the output was not NULL"
	 */
	int		tinyap_parsed_ok(const tinyap_t);

	/*! \brief access to last parsing output */
	ast_node_t	tinyap_get_output(const tinyap_t);

	/*! \brief get the column where the parsing error occured in source text (offsets start at 1) */
	int		tinyap_get_error_col(const tinyap_t);
	/*! \brief get the line where the parsing error occured in source text (offsets start at 1) */
	int		tinyap_get_error_row(const tinyap_t);

	/*! \brief get the source text around the parsing error, and a cursor indicating the column where the error occured */
	const char*	tinyap_get_error(const tinyap_t);

	/*! \brief delete an instance of a parser */
	void		tinyap_delete(tinyap_t);

	/*! \brief predicate "this node is nil"
	 * \return 1 if node pointer is NULL, 0 otherwise
	 */
	int		tinyap_node_is_nil(const ast_node_t);
	/*! \brief predicate "this node is a list"
	 * \return 1 if node is a pair, 0 otherwise
	 */
	int		tinyap_node_is_list(const ast_node_t);
	/*! \brief access to the n-th element of this list */
	ast_node_t	tinyap_list_get_element(const ast_node_t,int);
	/*! \brief predicate "this node is a string"
	 * \return 1 if node is an atom, 0 otherwise
	 */
	int		tinyap_node_is_string(const ast_node_t);
	/*! \brief access to the string value of this atom */
	const char*	tinyap_node_get_string(const ast_node_t);
	/*! \brief access to the n-th operand of this node ( (n+1)-th element of list) */
	ast_node_t	tinyap_node_get_operand(const ast_node_t,int);
	/*! \brief get the number of operands in this node (count(list)-1) */
	int		tinyap_node_get_operand_count(const ast_node_t);
	/*! \brief get the operator label of this node */
	const char*	tinyap_node_get_operator(const ast_node_t);

	void tinyap_serialize_to_file(const ast_node_t,const char*);
	const char*tinyap_serialize_to_string(const ast_node_t);

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


