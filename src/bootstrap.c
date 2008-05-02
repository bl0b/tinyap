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

#include "config.h"

#include "ast.h"
#include "tokenizer.h"
#include "bootstrap.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


ast_node_t  ast_unserialize(const char*input);

/*
 * BOOTSTRAP
 */
/* TODO : think of a more BNF-like syntax (like specific case for OPR rules, and pseudo-nonterminals like _identifier or _number */

const char*short_rules = "((Grammar\n"
"(_comment TinyaP\\ :\\ this\\ is\\ not\\ yet\\ another\\ parser.)\n"
"(_comment Copyright\\ \\(C\\)\\ 2007\\ Damien\\ Leroux)\n"
"(_comment Grammar\\ for\\ 'short'\\ dialect.)\n"
"(_comment This\\ program\\ is\\ free\\ software;\\ you\\ can\\ redistribute\\ it\\ and/or)\n"
"(_comment modify\\ it\\ under\\ the\\ terms\\ of\\ the\\ GNU\\ General\\ Public\\ License)\n"
"(_comment as\\ published\\ by\\ the\\ Free\\ Software\\ Foundation;\\ either\\ version\\ 2)\n"
"(_comment of\\ the\\ License,\\ or\\ \\(at\\ your\\ option\\)\\ any\\ later\\ version.)\n"
"(_comment This\\ program\\ is\\ distributed\\ in\\ the\\ hope\\ that\\ it\\ will\\ be\\ useful,)\n"
"(_comment but\\ WITHOUT\\ ANY\\ WARRANTY;\\ without\\ even\\ the\\ implied\\ warranty\\ of)\n"
"(_comment MERCHANTABILITY\\ or\\ FITNESS\\ FOR\\ A\\ PARTICULAR\\ PURPOSE.\\ \\ See\\ the)\n"
"(_comment GNU\\ General\\ Public\\ License\\ for\\ more\\ details.)\n"
"(_comment You\\ should\\ have\\ received\\ a\\ copy\\ of\\ the\\ GNU\\ General\\ Public\\ License)\n"
"(_comment along\\ with\\ this\\ program;\\ if\\ not,\\ write\\ to\\ the\\ Free\\ Software)\n"
"(_comment Foundation,\\ Inc.,\\ 59\\ Temple\\ Place\\ -\\ Suite\\ 330,\\ Boston,\\ MA\\ \\ 02111-1307,\\ USA.)\n"

"(_comment Production\\ Atoms)\n"
"(TransientRule	elem		(RE [_a-zA-Z][0-9a-zA-Z_]*))\n"
"(OperatorRule	T		(RPL \\\"\\(\\([^\\\"\\\\]|[\\\\][\\\"\\ trnb]\\)*\\)\\\" \\1))"
"(OperatorRule	NT		(NT elem))\n"
"(TransientRule	re_re		(RE \\([^\\\\/]|[\\\\][][\\\\/\\ <>trnb\"]\\)*))\n"
"(OperatorRule	RE		(Seq (T /) (NT re_re) (T /)))\n"
"(OperatorRule	RPL		(Seq (T //) (NT re_re) (T /) (RE \\([^\\r\\n\\\\\\\\\\/]+|\\\\\\\\[\\\\\\\\\\/0-9]\\)+) (T /)))\n"

"(_comment Rules)\n"
"(TransientRule	rule		(Alt (NT OperatorRule) (NT TransientRule)))\n"
"(OperatorRule	OperatorRule	(Seq (NT elem) (NT SPACE) (T ::=) (NT SPACE) (NT rule_expr) (T .) (NT NEWLINE)))\n"
"(OperatorRule	TransientRule	(Seq (NT elem) (NT SPACE) (T =) (NT SPACE) (NT rule_expr) (T .) (NT NEWLINE)))\n"
"(_comment Expressions)\n"
"(TransientRule	rule_expr	(Alt (NT Alt) (NT Seq) (NT rule_elem)))\n"
"(OperatorRule	Prefix		(Seq (T [) (NT rule_expr) (T ]) (NT rule_elem_atom)))\n"
"(OperatorRule	Postfix		(Seq (T {) (NT rule_expr) (T }) (NT rule_elem_atom)))\n"
"(OperatorRule	Seq		(Seq (NT rule_elem) (NT seq_expr)))\n"
"(OperatorRule	Alt		(Seq (T \\() (NT SPACE) (NT alt_expr) (T \\))))\n"

"(_comment Helpers)\n"
"(TransientRule	seq_expr	(Alt (Seq (NT seq_expr) (NT rule_elem)) (NT rule_elem)))\n"
"(TransientRule alt_elem	(Alt (NT Seq) (NT rule_elem)))\n"
"(TransientRule	alt_expr	(Alt (Seq (NT alt_expr) (T |) (NT SPACE) (NT alt_elem)) (NT alt_elem)))\n"
"(TransientRule	rule_elem	(Alt (NT EOF) (NT _comment) (NT Rep) (NT rule_elem_atom)))\n"
"(TransientRule rule_elem_atom	(Alt (Seq (Alt (NT epsilon) (NT T) (NT NT) (NT RPL) (NT RE) (NT Alt) (NT Prefix) (NT Postfix)) (NT SPACE))))\n"

"(_comment Entry\\ point)\n"
"(TransientRule	_start		(NT Grammar))\n"
"(OperatorRule	Grammar		(NT _loop))\n"
"(TransientRule	_loop		(Alt (EOF) (Seq (Alt (NT _comment) (NT rule)) (NT _loop))))\n"
"(_comment Builtins)\n"
"(OperatorRule	EOF		(T _EOF))\n"
"(OperatorRule	epsilon		(T _epsilon))\n"
"(OperatorRule	SPACE		(T SPACE))\n"
"(OperatorRule	NEWLINE		(T NEWLINE))\n"
"(OperatorRule	INDENT		(T INDENT))\n"
"(OperatorRule	DEDENT		(T DEDENT))\n"
/*"(TransientRule _whitespace	(RE \\([\\ \\r\\n\\t]|#[^\\r\\n]*[\\r\\n]+\\)+))\n"*/
"(TransientRule _whitespace	(RE \\([\\ \\r\\n\\t]\\)+))\n"
"(_comment Repetitions)\n"
"(OperatorRule Rep1N		(Seq (NT rule_elem_atom) (T +)))\n"
"(OperatorRule Rep0N		(Seq (NT rule_elem_atom) (T *)))\n"
"(OperatorRule Rep01		(Seq (NT rule_elem_atom) (T ?)))\n"
"(TransientRule Rep		(Alt (NT Rep1N) (NT Rep0N) (NT Rep01)))\n"
"(_comment Comments)\n"
"(OperatorRule	_comment	(Seq (T #) (Rep01 (Alt (Seq (NT SPACE) (RE [^\\r\\n]+)))) (NT NEWLINE)))\n"
"))\n";


const char*explicit_bnff_rules = "((Grammar "
"(_comment TinyaP\\ :\\ this\\ is\\ not\\ yet\\ another\\ parser.) (_comment Copyright\\ \\(C\\)\\ 2007\\ Damien\\ Leroux) (_comment Grammar\\ for\\ 'explicit'\\ dialect.) (_comment This\\ program\\ is\\ free\\ software;\\ you\\ can\\ redistribute\\ it\\ and\\/or) (_comment modify\\ it\\ under\\ the\\ terms\\ of\\ the\\ GNU\\ General\\ Public\\ License) (_comment as\\ published\\ by\\ the\\ Free\\ Software\\ Foundation;\\ either\\ version\\ 2) (_comment of\\ the\\ License,\\ or\\ \\(at\\ your\\ option\\)\\ any\\ later\\ version.)"
"(_comment This\\ program\\ is\\ distributed\\ in\\ the\\ hope\\ that\\ it\\ will\\ be\\ useful,) (_comment but\\ WITHOUT\\ ANY\\ WARRANTY;\\ without\\ even\\ the\\ implied\\ warranty\\ of) (_comment MERCHANTABILITY\\ or\\ FITNESS\\ FOR\\ A\\ PARTICULAR\\ PURPOSE.\\ \\ See\\ the) (_comment GNU\\ General\\ Public\\ License\\ for\\ more\\ details.) (_comment You\\ should\\ have\\ received\\ a\\ copy\\ of\\ the\\ GNU\\ General\\ Public\\ License) (_comment along\\ with\\ this\\ program;\\ if\\ not,\\ write\\ to\\ the\\ Free\\ Software) (_comment Foundation,\\ Inc.,\\ 59\\ Temple\\ Place\\ -\\ Suite\\ 330,\\ Boston,\\ MA\\ \\ 02111-1307,\\ USA.)"
"(_comment Production\\ Atoms) (TransientRule elem (RE [_a-zA-Z][0-9a-zA-Z_]*)) (OperatorRule T (RPL \\\"\\(\\([^\\\"\\\\]|[\\\\][\\\"\\ trnb]\\)*\\)\\\" \\\\1)) (OperatorRule NT (Seq (T <) (NT elem) (T >))) (TransientRule re_re (RE \\([^\\\\\\/]|[\\\\][][\\\\\\/\\ <>trnb\\\"]\\)*)) (OperatorRule RE (Seq (T \\/) (NT re_re) (T \\/))) (OperatorRule RPL (Seq (T \\/\\/) (NT re_re) (T \\/) (RE \\([^\\r\\n\\\\\\/]+|\\\\[\\\\\\/0-9]\\)+) (T \\/)))"
"(_comment Rules) (TransientRule rule (Alt (NT OperatorRule) (NT TransientRule))) (OperatorRule OperatorRule (Seq (NT elem) (NT SPACE) (T ::=) (NT SPACE) (NT rule_expr) (T .) (NT NEWLINE))) (OperatorRule TransientRule (Seq (NT elem) (NT SPACE) (T =) (NT SPACE) (NT rule_expr) (T .) (NT NEWLINE)))"
"(_comment Expressions) (TransientRule rule_expr (Alt (NT Alt) (NT Seq) (NT rule_elem))) (OperatorRule Prefix (Seq (T [) (NT rule_expr) (T ]) (NT rule_elem_atom))) (OperatorRule Postfix (Seq (T {) (NT rule_expr) (T }) (NT rule_elem_atom))) (OperatorRule Seq (Seq (NT rule_elem) (NT seq_expr))) (OperatorRule Alt (Seq (T \\() (NT SPACE) (NT alt_expr) (T \\))))"
"(_comment Helpers) (TransientRule seq_expr (Alt (Seq (NT seq_expr) (NT rule_elem)) (NT rule_elem))) (TransientRule alt_elem (Alt (NT Seq) (NT rule_elem))) (TransientRule alt_expr (Alt (Seq (NT alt_expr) (T |) (NT SPACE) (NT alt_elem)) (NT alt_elem))) (TransientRule rule_elem (Alt (NT EOF) (NT _comment) (NT Rep) (NT rule_elem_atom))) (TransientRule rule_elem_atom (Alt (Seq (Alt (NT epsilon) (NT T) (NT NT) (NT RPL) (NT RE) (NT Alt) (NT Prefix) (NT Postfix)) (NT SPACE))))"
"(_comment Entry\\ point) (TransientRule _start (NT Grammar)) (OperatorRule Grammar (NT _loop)) (TransientRule _loop (Alt (EOF) (Seq (Alt (NT _comment) (NT rule)) (NT _loop))))"
"(_comment Builtins) (OperatorRule EOF (T EOF)) (OperatorRule epsilon (T epsilon)) (OperatorRule SPACE (T SPACE)) (OperatorRule NEWLINE (T NEWLINE)) (OperatorRule INDENT (T INDENT)) (OperatorRule DEDENT (T DEDENT)) (TransientRule _whitespace (RE \\([\\ \\r\\n\\t]\\)+))"
"(_comment Repetitions) (OperatorRule Rep1N (Seq (NT rule_elem_atom) (T +))) (OperatorRule Rep0N (Seq (NT rule_elem_atom) (T *))) (OperatorRule Rep01 (Seq (NT rule_elem_atom) (T ?))) (TransientRule Rep (Alt (NT Rep1N) (NT Rep0N) (NT Rep01)))"
"(_comment Comments) (OperatorRule _comment (Seq (T #) (Rep01 (Alt (Seq (NT SPACE) (RE [^\\r\\n]+)))) (NT NEWLINE)))"
"))";


ast_node_t  init_BNF_rules() {
	return ast_unserialize(explicit_bnff_rules);
}


size_t node_pool_size();
extern volatile int _node_alloc_count;

ast_node_t  tinyap_get_ruleset(const char*name) {
	struct stat st;
	char*buf;
	ast_node_t ret=NULL;
//	printf("before tinyap_get_ruleset : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	if(!strcmp(name,GRAMMAR_EXPLICIT)) {
		ret=ast_unserialize(explicit_bnff_rules);
	/*} else if(!strcmp(name,GRAMMAR_CAMELCASING)) {*/
		/*ret=ast_unserialize(CamelCased_bnff_rules);*/
	} else if(!strcmp(name,GRAMMAR_SHORT)) {
		ret=ast_unserialize(short_rules);
	} else if(!stat(name,&st)) {
		/* unserialize from file */
		FILE*f=fopen(name,"r");
		buf=(char*)malloc(st.st_size+1);
		fread(buf,1,st.st_size,f);
		buf[st.st_size]=0;
		fclose(f);
		ret=ast_unserialize(buf);
		free(buf);
	}
//	printf("after  tinyap_get_ruleset : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	//dump_node(ret);
	return ret;
}
