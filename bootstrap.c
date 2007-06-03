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

const char*explicit_bnff_rules = "( Grammar\n"
"(trans_rule	elem		(RE [_a-zA-Z][0-9a-zA-Z_]*))\n"
"(opr_rule	T		(seq (T \") (RE [^\"]+) (T \")))\n"
"(opr_rule	NT		(seq (T <) (NT elem) (T >)))\n"
"(opr_rule	RE		(seq (T /) (RE [^/]+) (T /)))\n"
"(trans_rule	rule		(alt (NT opr_rule) (NT trans_rule)))\n"
"(opr_rule	opr_rule	(seq (NT elem) (T ::=) (NT rule_expr) (T .)))\n"
"(opr_rule	trans_rule	(seq (NT elem) (T =) (NT rule_expr) (T .)))\n"
"(trans_rule	rule_expr	(alt (seq (T \\() (NT alt) (T \\))) (NT seq) (NT rule_elem)))\n"
"(opr_rule	seq		(seq (NT rule_elem) (NT seq_expr)))\n"
"(opr_rule	alt		(NT alt_expr))\n"
"(trans_rule	seq_expr	(alt (seq (NT rule_elem) (NT seq_expr)) (NT rule_elem)))\n"
"(trans_rule	alt_expr	(alt (seq (NT rule_elem) (T |) (NT alt_expr)) (seq (NT seq) (T |) (NT alt_expr)) (NT seq) (NT rule_elem)))\n"
"(trans_rule	rule_elem	(alt (NT T) (NT NT) (NT RE) (seq (T \\() (NT alt) (T \\))) (NT eof)))\n"
"(trans_rule	_start		(NT Grammar))\n"
"(opr_rule	Grammar	(NT _loop))\n"
"(trans_rule	_loop		(alt (eof) (seq (NT rule) (NT _loop))))\n"
"(opr_rule	eof		(T eof))\n"
")\n";


const char*CamelCased_bnff_rules = "( Grammar\n"
"(trans_rule	CamelIdent	(RE [A-Z][0-9a-z]+[A-Z][0-9a-z]*))\n"
"(trans_rule	elem		(RE [_a-z][0-9a-zA-Z_]*))\n"
"(opr_rule	T		(seq (T \") (RE [^\"]+) (T \")))\n"
"(opr_rule	NT		(seq (T <) (NT elem) (T >)))\n"
"(opr_rule	RE		(seq (T /) (RE [^/]+) (T /)))\n"
"(trans_rule	rule		(alt (NT opr_rule) (NT trans_rule)))\n"
"(opr_rule	opr_rule	(seq (NT CamelIdent) (T ::=) (NT rule_expr) (T .)))\n"
"(opr_rule	trans_rule	(seq (NT elem) (T ::=) (NT rule_expr) (T .)))\n"
"(trans_rule	rule_expr	(alt (seq (T \\() (NT alt) (T \\))) (NT seq) (NT rule_elem)))\n"
"(opr_rule	seq		(seq (NT rule_elem) (NT seq_expr)))\n"
"(opr_rule	alt		(NT alt_expr))\n"
"(trans_rule	seq_expr	(alt (seq (NT rule_elem) (NT seq_expr)) (NT rule_elem)))\n"
"(trans_rule	alt_expr	(alt (seq (NT rule_elem) (T |) (NT alt_expr)) (seq (NT seq) (T |) (NT alt_expr)) (NT seq) (NT rule_elem)))\n"
"(trans_rule	rule_elem	(alt (NT T) (NT NT) (NT RE) (seq (T \\() (NT alt) (T \\))) (NT eof)))\n"
"(trans_rule	_start		(NT Grammar))\n"
"(opr_rule	Grammar	(NT _loop))\n"
"(trans_rule	_loop		(alt (eof) (seq (NT rule) (NT _loop))))\n"
"(opr_rule	eof		(T eof))\n"
")\n";


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
