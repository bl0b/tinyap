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

/*! \brief arbitrary and totally meaningless version string
 */
#define TINYAP_VERSION PACKAGE_VERSION

/*!
 * \mainpage tinyap
 *
 * This is not yet another parser.
 * 
 * \section sec_intro Introduction
 * 
 * Actually, this is an abstract parser. It takes a grammar description and some
 * source text to produce an Abstract Syntax Tree (AST) that structures the information
 * that was contained in the source.
 * It's an LL parser using the recursive descent with backup algorithm, and it can produce
 * litteral strings and regular expressions from its input text.
 *
 * You may wish to read the \ref Tutorial "Tutorial" if it's your first time with tinyap.
 *
 * \ref tinyap.h "Parser API"
 *
 * \ref tinyape.h "Ape API"
 *
 */


#ifdef __cplusplus
extern "C" {
#endif
#ifndef _TINYAP_AST_H__
#endif
	typedef struct _tinyap_t* tinyap_t;
#ifndef _WAST_DEFINED
	typedef struct _walkable_ast_t* wast_t;
	typedef struct _wast_iter_t* wast_iterator_t;
	#define _WAST_DEFINED
#endif

#include "bootstrap.h"

/*! \weakgroup api_parser Parser API */
/*@{*/

	/*! \brief initialize the tinyap environment. */
	void tinyap_init();

	/*! \brief create a new parser with default context.
	 * create a parser with "explicit" metagrammar dialect, " \r\t\n" whitespaces, and input from stdin.
	 */
	tinyap_t	tinyap_new();

	/*! \brief set the parser verbosity */
	void tinyap_set_verbose(int);

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
	/*! \brief access to duration of last parsing, in seconds */
	float		tinyap_get_parse_time(tinyap_t);
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
	int tinyap_parse_as_grammar(tinyap_t);

	/*! \brief predicate "the last parsing was successful"
	 * \return 1 if last parsing was successful, 0 otherwise
	 * equates to "the output was not NULL"
	 */
	int		tinyap_parsed_ok(const tinyap_t);

	/*! \brief access to last parsing output
	 */
	ast_node_t	tinyap_get_output(const tinyap_t);

	/*! \brief get the column where the parsing error occured in source text (offsets start at 1)
	 */
	int		tinyap_get_error_col(const tinyap_t);
	/*! \brief get the line where the parsing error occured in source text (offsets start at 1)
	 */
	int		tinyap_get_error_row(const tinyap_t);

	/*! \brief get the source text around the parsing error, and a cursor indicating the column where the error occured
	 */
	const char*	tinyap_get_error(const tinyap_t);

	/*! \brief delete an instance of a parser
	 */
	void		tinyap_delete(tinyap_t);

	/*! \brief plug (NT plugin) at head of (Alt) element of rule (* plug (Alt)). Plugs must have this form.
	 */
	void tinyap_plug(tinyap_t parser, const char*plugin, const char*plug);

	/*! \brief plug pin at head of (Alt) element of rule (* plug (Alt)). Plugs must have this form.
	 */
	void tinyap_plug_node(tinyap_t parser, ast_node_t pin, const char*plugin, const char* plug);

	/*! \brief append this grammar to the existing grammar.
	 */
	void tinyap_append_grammar(tinyap_t parser, ast_node_t supp);
/*@}*/
/*! \weakgroup api_ast AST API */
/*@{*/
	/*! \brief predicate "this node is nil"
	 * \return 1 if node pointer is NULL, 0 otherwise
	 */
	int		tinyap_node_is_nil(const ast_node_t);
	/*! \brief predicate "this node is a list"
	 * \return 1 if node is a pair, 0 otherwise
	 */
	int		tinyap_node_is_list(const ast_node_t);
	/*! \brief get the length of the list
	 */
	unsigned int	tinyap_list_get_size(const ast_node_t);
	/*! \brief access to the n-th element of this list
	 */
	ast_node_t	tinyap_list_get_element(const ast_node_t,int);
	/*! \brief predicate "this node is a string"
	 * \return 1 if node is an atom, 0 otherwise
	 */
	int		tinyap_node_is_string(const ast_node_t);
	/*! \brief access to the string value of this atom
	 */
	const char*	tinyap_node_get_string(const ast_node_t);
	/*! \brief access to the n-th operand of this node ( (n+1)-th element of list)
	 */
	ast_node_t	tinyap_node_get_operand(const ast_node_t,int);
	/*! \brief get the number of operands in this node (count(list)-1)
	 */
	int		tinyap_node_get_operand_count(const ast_node_t);
	/*! \brief get the operator label of this node
	 */
	const char*	tinyap_node_get_operator(const ast_node_t);

	/*! \brief get the offset corresponding to this node in source text
	 */
	int		tinyap_node_get_row(const ast_node_t);
	/*! \brief get the col corresponding to this node in source text
	 */
	int		tinyap_node_get_col(const ast_node_t);

	/*! \brief serialize this node to the named file
	 */
	void		tinyap_serialize_to_file(const ast_node_t,const char*);
	/*! \brief serialize this node into a new C string (must be freed by the user)
	 */
	const char*	tinyap_serialize_to_string(const ast_node_t);

	/*! \brief make this AST walkable.
	 * \return a walkable copy of the given tree.
	 */
	wast_t		tinyap_make_wast(const ast_node_t);

	/*! \brief make this walkable AST a serializable AST.
	 * \return a serializable copy of the given tree.
	 */
	ast_node_t	tinyap_make_ast(const wast_t);

	/*! \brief free this walkable AST.
	 */
	void		tinyap_free_wast(const wast_t);

	/*! \brief create an iterator on this AST.
	 */
	wast_iterator_t	tinyap_wi_new(const wast_t);
	#define wi_new tinyap_wi_new

	/*! \brief reset an iterator to its initial state (on root, backup cleared).
	 */
	wast_iterator_t	tinyap_wi_reset(wast_iterator_t);
	#define wi_reset tinyap_wi_reset

	/*! \brief destroy an iterator on an AST.
	 */
	void		tinyap_wi_delete(wast_iterator_t);
	#define wi_delete tinyap_wi_delete

	/*! \brief get node under iterator
	 */
	wast_t		tinyap_wi_node(wast_iterator_t);
	#define wi_node tinyap_wi_node

	/*! \brief iterator jumps to parent
	 */
	wast_iterator_t	tinyap_wi_up(wast_iterator_t);
	#define wi_up tinyap_wi_up

	/*! \brief iterator jumps to first child
	 */
	wast_iterator_t	tinyap_wi_down(wast_iterator_t);
	#define wi_down tinyap_wi_down

	/*! \brief iterator jumps to next sibling
	 */
	wast_iterator_t	tinyap_wi_next(wast_iterator_t);
	#define wi_next tinyap_wi_next

	/*! \brief backup iterator state
	 */
	wast_iterator_t	tinyap_wi_backup(wast_iterator_t);
	#define wi_backup tinyap_wi_backup

	/*! \brief restore iterator state
	 */
	wast_iterator_t	tinyap_wi_restore(wast_iterator_t);
	#define wi_restore tinyap_wi_restore

	/*! \brief forget previous backup
	 */
	wast_iterator_t	tinyap_wi_validate(wast_iterator_t);
	#define wi_validate tinyap_wi_validate

	/*! \brief get iterator's root.
	 */
	wast_t		tinyap_wi_root(wast_iterator_t);
	#define wi_root tinyap_wi_root

	/*! \brief get iterator's current parent.
	 */
	wast_t		tinyap_wi_parent(wast_iterator_t);
	#define wi_root tinyap_wi_root

	/*! \brief predicate "iterator is on root", i.e. "NOT (node has a parent)"
	 */
	wast_iterator_t	tinyap_wi_dup(const wast_iterator_t);
	#define wi_dup tinyap_wi_dup

	/*! \brief predicate "iterator is on root", i.e. "NOT (node has a parent)"
	 */
	int		tinyap_wi_on_root(wast_iterator_t);
	#define wi_on_root tinyap_wi_on_root

	/*! \brief predicate "iterator is on a leaf", i.e. "NOT (node has children)"
	 */
	int		tinyap_wi_on_leaf(wast_iterator_t);
	#define wi_on_leaf tinyap_wi_on_leaf

	/*! \brief predicate "iterator has a next sibling", i.e. "NOT (node is last child)"
	 */
	int		tinyap_wi_has_next(wast_iterator_t);
	#define wi_has_next tinyap_wi_has_next

	/*! \brief Render the text buffer that generated the AST \c ast when parsed using \c grammar.
	 */
	const char*	tinyap_unparse(wast_t grammar, wast_t ast);

	/*! \brief walk the current output using this named pilot with this init data.
	 * \return whatever the pilot evaluated to
	 */
	void*		tinyap_walk_output(const char* pilot_name, void* pilot_init_data);

	/*! \brief walk this subtree using this named pilot with this init data.
	 * \return whatever the pilot evaluated to
	 */
	void*		tinyap_walk(const wast_t subtree, const char* pilot_name, void* pilot_init_data);
/*@}*/

#ifdef __cplusplus
}

#if 0
namespace TinyaP {

	class AstNode {
	public:
		AstNode(ast_node_t ptr)
		{ handle=ptr; initCache(ptr,0); }
		AstNode()
		{ handle=0; cache=0; }
		~AstNode()
		{ if(cache) delete[] cache; }
		bool isNil() const
		{ return tinyap_node_is_nil(handle); }
		bool isList() const
		{ return tinyap_node_is_list(handle); }
		bool isOp() const
		{ return tinyap_node_is_list(handle)&&tinyap_node_is_string(tinyap_list_get_element(handle,0)); }
		bool isString() const
		{ return tinyap_node_is_string(handle); }
		const char* getString() const
		{ return tinyap_node_get_string(handle); }
		AstNode* getOperand(const int n) const
		{ return cache[n+1]; }
		int getOperandCount() const
		{ return cache_sz-1; }
		const char* getOperator() const
		{ return tinyap_node_get_string(tinyap_list_get_element(handle,0)); }
		AstNode* getElement(const int n) const
		{ return cache[n]; }
		const char*toString() const
		{ return tinyap_serialize_to_string(handle); }
		void toFile(const char*fnam) const
		{ return tinyap_serialize_to_file(handle,fnam); }

		int getRow() const
		{ return tinyap_node_get_row(handle); }

		int getCol() const
		{ return tinyap_node_get_col(handle); }

		ast_node_t getHandle() const
		{ return handle; }
	private:
		ast_node_t handle;
		AstNode**cache;
		int cache_sz;
		void initCache(ast_node_t ptr,int length)
		{
			if(tinyap_node_is_string(ptr))
			{
				if(length==0)
				{
					cache=0;
				} else
				{
					cache_sz=length+1;
					cache=new AstNode* [cache_sz];
					cache[length]=new AstNode(ptr);
				}
			} else if(tinyap_node_is_list(ptr))
			{
				initCache(tinyap_list_get_element(ptr,1),length+1);
			} else if(ptr==0)
			{
				cache_sz=length;
				cache = new AstNode* [cache_sz];
			}
		}
	};

	class Parser {
	public:
	/*! \brief create a new parser with default context.
	 * create a parser with "explicit" metagrammar dialect, " \r\t\n" whitespaces, and input from stdin.
	 */	Parser()
		{ handle=tinyap_new(); grammar=output=0; }

	/*! \brief create a new parser with default context.
	 * create a parser with "explicit" metagrammar dialect, " \r\t\n" whitespaces, and input from stdin.
	 * this call may be used to chain setter calls.
	 */	static Parser& New()
		{ return *new Parser(); }

	/*! \brief delete an instance of a parser
	 */	~Parser()
		{
			tinyap_delete(handle);
			if(grammar) delete grammar;
			if(output) delete output;
		}

		const char*getGrammar() const
		{ return tinyap_get_grammar(handle); }

		Parser& setGrammar(const char*gs)
		{
		       tinyap_set_grammar(handle,gs);
		       if(grammar) delete grammar;
		       grammar=new AstNode(tinyap_get_grammar_ast(handle));
		       return *this;
		}

		const char*getWhitespace() const
		{ return tinyap_get_whitespace(handle); }

		Parser& setWhitespace(const char* ws)
		{ tinyap_set_whitespace(handle, ws); return *this; }

		Parser& setWhitespaceRegexp(const char*wsre)
		{ tinyap_set_whitespace_regexp(handle,wsre); return *this; }

		AstNode* getGrammarAst() const
		{ return grammar; }

		Parser& setGrammarAST(const AstNode*gast)
		{
			if(grammar) delete grammar;
			tinyap_set_grammar_ast(handle,gast->getHandle());
			grammar=new AstNode(gast->getHandle());
			return *this;
		}

		const char*getSourceFile() const
		{ return tinyap_get_source_file(handle); }

		const char*getSourceBuffer() const
		{ return tinyap_get_source_buffer(handle); }

		unsigned int getSourceBufferLength() const
		{ return tinyap_get_source_buffer_length(handle); }

		Parser& setSourceFile(const char*fnam)
		{ tinyap_set_source_file(handle,fnam); return *this; }

		Parser& setSourceBuffer(const char*buf,const unsigned int len)
		{ tinyap_set_source_buffer(handle,buf,len); return *this; }

		bool parse()
		{
			tinyap_parse(handle);
			if(output) delete output;
			output=new AstNode(tinyap_get_output(handle));
			return tinyap_parsed_ok(handle);
		}

		bool parseAsGrammar()
		{
			if(tinyap_parse_as_grammar(handle))
			{
				if(grammar) delete grammar;
				grammar=new AstNode(tinyap_get_grammar_ast(handle));
				if(output) delete output;
				output=new AstNode(tinyap_get_output(handle));
			}
			return tinyap_parsed_ok(handle);
		}

		bool parsedOK() const
		{ return tinyap_parsed_ok(handle); }

		AstNode* getOutput() const
		{ return output; }

		int getErrorCol() const
		{ return tinyap_get_error_col(handle); }

		int getErrorRow() const
		{ return tinyap_get_error_row(handle); }

		const char* getError() const
		{ return tinyap_get_error(handle); }

		tinyap_t getHandle() const
		{ return handle; }
	private:
		tinyap_t handle;
		AstNode*grammar,*output;
	};
}
#endif


#endif
#endif


