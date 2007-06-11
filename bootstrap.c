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


#include "ast.h"
#include "tokenizer.h"
#include "bootstrap.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


ast_node_t* ast_unserialize(const char*input);

/*
 * BOOTSTRAP
 */
/* TODO : think of a more BNF-like syntax (like specific case for OPR rules, and pseudo-nonterminals like _identifier or _number */

const char*explicit_bnff_rules = "((Grammar\n"
"(TransientRule	elem		(RE [_a-zA-Z][0-9a-zA-Z_]*))\n"
"(OperatorRule	T		(Seq (T \\\") (RE \\(\\\\\\\\\\\"|[^\"]\\)+) (T \\\")))\n"
"(OperatorRule	NT		(Seq (T <) (NT elem) (T >)))\n"
"(OperatorRule	RE		(Seq (T /) (RE [^/]+) (T /)))\n"
"(TransientRule	rule		(Alt (NT OperatorRule) (NT TransientRule)))\n"
"(OperatorRule	OperatorRule	(Seq (NT elem) (T ::=) (NT rule_expr) (T .)))\n"
"(OperatorRule	TransientRule	(Seq (NT elem) (T =) (NT rule_expr) (T .)))\n"
"(TransientRule	rule_expr	(Alt (Seq (T \\() (NT Alt) (T \\))) (NT Seq) (NT rule_elem)))\n"
"(OperatorRule	Seq		(Seq (NT rule_elem) (NT seq_expr)))\n"
"(OperatorRule	Alt		(NT alt_expr))\n"
"(TransientRule	seq_expr	(Alt (Seq (NT rule_elem) (NT seq_expr)) (NT rule_elem)))\n"
"(TransientRule	alt_expr	(Alt (Seq (NT rule_elem) (T |) (NT alt_expr)) (Seq (NT Seq) (T |) (NT alt_expr)) (NT Seq) (NT rule_elem)))\n"
"(TransientRule	rule_elem	(Alt (NT T) (NT NT) (NT RE) (Seq (T \\() (NT Alt) (T \\))) (NT EOF)))\n"
"(TransientRule	_start		(NT Grammar))\n"
"(OperatorRule	Grammar	(NT _loop))\n"
"(TransientRule	_loop		(Alt (EOF) (Seq (NT rule) (NT _loop))))\n"
"(OperatorRule	EOF		(T EOF))\n"
"))\n";


const char*CamelCased_bnff_rules = "((Grammar\n"
"(TransientRule	camelIdent	(RE [A-Z][0-9a-z]*\\([A-Z][0-9a-z]*\\)*))\n"
"(TransientRule	elem		(RE [_a-z][0-9a-zA-Z_]*))\n"
"(OperatorRule	T		(Seq (T \\\") (RE \\(\\\\\\\\\\\"|[^\"]\\)+) (T \\\")))\n"
"(OperatorRule	NT		(Seq (T <) (Alt (NT camelIdent) (NT elem)) (T >)))\n"
"(OperatorRule	RE		(Seq (T /) (RE [^/]+) (T /)))\n"
"(TransientRule	rule		(Alt (NT OperatorRule) (NT TransientRule)))\n"
"(OperatorRule	OperatorRule	(Seq (NT camelIdent) (T ::=) (NT rule_expr) (T .)))\n"
"(OperatorRule	TransientRule	(Seq (NT elem) (T ::=) (NT rule_expr) (T .)))\n"
"(TransientRule	rule_expr	(Alt (NT Alt) (NT Seq) (NT rule_elem)))\n"
"(OperatorRule	Seq		(Seq (NT rule_elem) (NT seq_expr)))\n"
"(OperatorRule	Alt		(Seq (T \\() (NT alt_expr) (T \\))))\n"
"(TransientRule	seq_expr	(Alt (Seq (NT rule_elem) (NT seq_expr)) (NT rule_elem)))\n"
"(TransientRule	alt_expr	(Alt (Seq (NT Seq) (T |) (NT alt_expr)) (Seq (NT rule_elem) (T |) (NT alt_expr)) (NT Seq) (NT rule_elem)))\n"
"(TransientRule	rule_elem	(Alt (NT T) (NT NT) (NT RE) (NT Alt) (NT EOF)))\n"
"(TransientRule	_start		(NT Grammar))\n"
"(OperatorRule	Grammar		(NT _loop))\n"
"(TransientRule	_loop		(Alt (EOF) (Seq (NT rule) (NT _loop))))\n"
"(OperatorRule	EOF		(T EOF))\n"
"))\n";


ast_node_t* init_BNF_rules() {
	return ast_unserialize(explicit_bnff_rules);
}

ast_node_t* get_ruleset(const char*name) {
	struct stat st;
	char*buf;
	ast_node_t*ret=NULL;
	if(!strcmp(name,GRAMMAR_EXPLICIT)) {
		ret=ast_unserialize(explicit_bnff_rules);
	} else if(!strcmp(name,GRAMMAR_CAMELCASING)) {
		ret=ast_unserialize(CamelCased_bnff_rules);
	} else if(!stat(name,&st)) {
		/* unserialize from file */
		FILE*f=fopen(name,"r");
		buf=(char*)malloc(st.st_size+1);
		fread(buf,1,st.st_size,f);
		fclose(f);
		ret=ast_unserialize(buf);
		free(buf);
	}
	return ret;
}
