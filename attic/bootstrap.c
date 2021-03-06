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
#include "bootstrap.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


ast_node_t  ast_unserialize(const char*input);

/*
 * BOOTSTRAP
 */

#if 1

const char*short_rules = "((Grammar\n"
"(TransientRule toto (Rep01 (NT pouet)))"
"(Comment \\ Production\\ Atoms)\n"
"(TransientRule	symbol			(RE [_a-zA-Z][0-9a-zA-Z_]*))\n"
"(OperatorRule	T				(STR \" \"))\n"
"(OperatorRule	NT				(NT symbol))\n"
"(OperatorRule	RE				(STR / /))\n"
"(OperatorRule	STR				(RawSeq (T ~) (RE [^~,]?)  (T ,)"
" 										(RE [^~,]?) (T ~)))\n"
"(OperatorRule	BOW				(RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*)"
" 										(Rep01 (Seq (T !) (NT BKeep))) (T ~)))\n"
"(OperatorRule	AddToBag		(RawSeq (NT RE) (T :) (NT symbol)"
" 										(Rep01 (Seq (T !) (NT BKeep)))))\n"
"(OperatorRule	BKeep			(Epsilon))\n"
"(TransientRule	elem_atom		(Alt (NT NT) (NT STR) (NT BOW) (NT AddToBag)"
" 									 (NT T) (NT RE)\n"
"									 (NT Epsilon) (NT EOF) \n"
"									 (Seq (T \\() (NT elem_comp) (T \\)))\n"
"								))\n"
"(Comment \\ Compositions)\n"
"(TransientRule	elem			(Alt (NT elem_atom) (NT Rep01) (NT Rep0N)"
" 									 (NT Rep1N)))\n"
/*"(TransientRule	elem_seq_struc 	(Seq (NT elem) (NT Space) (NT _ess_loop)))"*/
/*"(TransientRule _ess_loop 		(Alt (Seq (NT elem) (NT Space) (NT _ess_loop))"*/
									/*"(NT elem)))"*/
/*"(TransientRule	elem_seq_struc 	(Alt (NT elem)"*/
									/*"(Seq (NT elem_seq_struc) (NT Space)"*/
 										 /*"(NT elem))))"*/
"(TransientRule	elem_seq_struc 	(Seq (NT elem) (Rep1N (Seq (NT Space) (NT elem)))))\n"
"(TransientRule	elem_seq 		(Alt (NT RawSeq) (NT Seq) (NT elem)))\n"
"(TransientRule	elem_comp		(Alt (NT RawSeq) (NT Seq) (NT Alt)))\n"
"(OperatorRule	Alt				(Seq (NT elem_seq) (Rep1N (Seq (T |)"
" 														  (NT elem_seq)))))\n"
"(OperatorRule	Seq				(NT elem_seq_struc))\n"
"(OperatorRule	RawSeq			(Seq (T .raw) (NT Space) (NT elem_seq_struc)))\n"
"(OperatorRule	Rep01			(Seq (NT elem_atom) (T ?)))\n"
"(OperatorRule	Rep1N			(Seq (NT elem_atom) (T +)))\n"
"(OperatorRule	Rep0N			(Seq (NT elem_atom) (T *)))\n"
"(TransientRule	_whitespace		(RE \\([\\ \\r\\n\\t]\\)+))\n"
"(OperatorRule	TransientRule	(Seq (NT symbol) (NT Space) (T =)"
" 									 (NT Space) (Alt (NT elem) (NT elem_comp)) (T .)"
" 									 (NT NewLine)))\n"
"(OperatorRule	OperatorRule	(Seq (NT symbol) (NT Space) (T ::=)"
" 									 (NT Space) (Alt (NT elem) (NT elem_comp)) (T .)"
" 									 (NT NewLine)))\n"
"(OperatorRule	Comment	(Seq (T #) (Rep01 (RE [^\\r\\n]+)) (NT NewLine)))\n"
"(OperatorRule	Grammar			(Rep1N (Alt (NT TransientRule)"
" 											(NT OperatorRule)"
" 											(NT Comment))))\n"
"(TransientRule	_start			(Seq (NT Grammar) (EOF)))\n"
"(OperatorRule	EOF			(T _EOF))\n"
"(OperatorRule	Epsilon			(T _epsilon))\n"
"(OperatorRule	Space			(T Space))\n"
"(OperatorRule	NewLine			(T NewLine))\n"
"(OperatorRule	Indent			(T Indent))\n"
"(OperatorRule	Dedent			(T Dedent))\n"
"))";

#else

const char*short_rules = "((Grammar\n"
"(Comment \\ TinyaP\\ :\\ this\\ is\\ not\\ yet\\ another\\ parser.)\n"
"(Comment \\ Copyright\\ \\(C\\)\\ 2007\\ Damien\\ Leroux)\n"
"(Comment \\ Grammar\\ for\\ 'short'\\ dialect.)\n"
"(Comment \\ This\\ program\\ is\\ free\\ software;\\ you\\ can\\ redistribute\\ it\\ and/or)\n"
"(Comment \\ modify\\ it\\ under\\ the\\ terms\\ of\\ the\\ GNU\\ General\\ Public\\ License)\n"
"(Comment \\ as\\ published\\ by\\ the\\ Free\\ Software\\ Foundation;\\ either\\ version\\ 2)\n"
"(Comment \\ of\\ the\\ License,\\ or\\ \\(at\\ your\\ option\\)\\ any\\ later\\ version.)\n"
"(Comment \\ This\\ program\\ is\\ distributed\\ in\\ the\\ hope\\ that\\ it\\ will\\ be\\ useful,)\n"
"(Comment \\ but\\ WITHOUT\\ ANY\\ WARRANTY;\\ without\\ even\\ the\\ implied\\ warranty\\ of)\n"
"(Comment \\ MERCHANTABILITY\\ or\\ FITNESS\\ FOR\\ A\\ PARTICULAR\\ PURPOSE.\\ \\ See\\ the)\n"
"(Comment \\ GNU\\ General\\ Public\\ License\\ for\\ more\\ details.)\n"
"(Comment \\ You\\ should\\ have\\ received\\ a\\ copy\\ of\\ the\\ GNU\\ General\\ Public\\ License)\n"
"(Comment \\ along\\ with\\ this\\ program;\\ if\\ not,\\ write\\ to\\ the\\ Free\\ Software)\n"
"(Comment \\ Foundation,\\ Inc.,\\ 59\\ Temple\\ Place\\ -\\ Suite\\ 330,\\ Boston,\\ MA\\ \\ 02111-1307,\\ USA.)\n"
"(Comment )\n"
"(Comment \\ Production\\ Atoms)\n"
"(Comment )\n"
"(TransientRule	elem			(RE [_a-zA-Z][0-9a-zA-Z_]*))\n"
"(OperatorRule	STR			(RawSeq (T ~) (RE [^~,]?)  (T ,) (RE [^~,]?) (T ~)))\n"
"(OperatorRule	BOW				(RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (NT optBK) (T ~)))\n"
"(OperatorRule	AddToBag		(RawSeq (NT RE) (T :) (RE [_a-zA-Z][_a-zA-Z0-9]*) (NT optBK)))\n"
"(TransientRule	optBK			(Rep01 (Alt (Seq (T !) (NT BKeep)))))\n"
"(OperatorRule	BKeep			(Epsilon))\n"
"(OperatorRule	T				(STR \" \"))\n"
"(OperatorRule	NT				(NT elem))\n"
"(OperatorRule	RE				(STR / /))\n"
"(Comment )\n"
"(Comment \\ Rules)\n"
"(Comment )\n"
"(TransientRule	rule			(Alt (NT OperatorRule) (NT TransientRule)))\n"
"(OperatorRule	OperatorRule	(Seq (NT elem) (NT Space) (T ::=) (NT Space) (NT rule_expr) (T .) (NT NewLine)))\n"
"(OperatorRule	TransientRule	(Seq (NT elem) (NT Space) (T =) (NT Space) (NT rule_expr) (T .) (NT NewLine)))\n"
"(Comment )\n"
"(Comment \\ Expressions)\n"
"(Comment )\n"
"(TransientRule	rule_expr		(Alt (NT Alt) (NT RawSeq) (NT Seq) (NT rule_elem)))\n"
"(OperatorRule	Prefix			(Seq (T [) (NT rule_expr) (T ]) (NT rule_elem_atom)))\n"
"(OperatorRule	Postfix			(Seq (T {) (NT rule_expr) (T }) (NT rule_elem_atom)))\n"
"(OperatorRule	RawSeq			(Seq (T .raw) (NT Space) (NT rule_elem) (Rep1N (Alt (Seq (NT Space) (NT rule_elem))))))\n"
"(OperatorRule	RawSeq			(Seq (T .raw) (NT Space) (NT seq_expr)))\n"
"(OperatorRule	Seq				(Seq (NT rule_elem) (Rep1N (Alt (Seq (NT Space) (NT rule_elem))))))\n"
"(OperatorRule	Seq				(Alt (NT seq_expr) (Seq (T \\() (NT seq_expr) (T \\)))))\n"
"(OperatorRule	Alt				(Seq (T \\() (NT Space) (NT alt_expr) (NT Space) (T \\))))\n"
"(OperatorRule	Alt				(Seq (T \\() (NT alt_elem) (Rep0N (Alt (Seq (NT Space) (T |) (NT Space) (NT alt_elem)))) (T \\))))\n"
"(Comment )\n"
"(Comment \\ Helpers)\n"
"(Comment )\n"
"(TransientRule	seq_expr		(Alt (Seq (NT seq_expr) (NT Space) (NT rule_elem)) (Seq (NT rule_elem) (NT Space) (NT rule_elem))))\n"
"(TransientRule	alt_elem		(Alt (NT RawSeq) (NT Seq) (NT rule_elem)))\n"
//"(TransientRule	alt_expr	(Alt (Seq (NT alt_expr) (T |) (NT Space) (NT alt_elem)) (NT alt_elem)))\n"
"(TransientRule	alt_expr		(Seq (NT alt_elem) (Rep0N (Alt (Seq (T |) (NT Space) (NT alt_elem))))))\n"
"(TransientRule	rule_elem		(Alt (NT EOF) (NT Comment) (NT Rep) (NT rule_elem_atom)))\n"
"(TransientRule	rule_elem_atom	(Alt (NT Epsilon) (NT Prefix) (NT Postfix) (NT T) (NT STR) (NT BOW) (NT AddToBag) (NT RE) (NT NT) (NT Alt)))\n"
"(Comment )\n"
"(Comment \\ Entry\\ point)\n"
"(Comment )\n"
"(TransientRule	_start			(NT Grammar))\n"
"(OperatorRule	Grammar			(NT _loop))\n"
"(TransientRule	_loop			(Alt (EOF) (Seq (Alt (NT Comment) (NT rule)) (NT _loop))))\n"
"(Comment )\n"
"(Comment \\ Builtins)\n"
"(Comment )\n"
"(OperatorRule	EOF				(T _EOF))\n"
"(OperatorRule	Epsilon			(T _epsilon))\n"
"(OperatorRule	Space			(T Space))\n"
"(OperatorRule	NewLine			(T NewLine))\n"
"(OperatorRule	Indent			(T Indent))\n"
"(OperatorRule	Dedent			(T Dedent))\n"
"(TransientRule _whitespace	(RE \\([\\ \\r\\n\\t]|#[^\\r\\n]*[\\r\\n]+\\)+))\n"
"(TransientRule	_whitespace		(RE \\([\\ \\r\\n\\t]\\)+))\n"
"(Comment )\n"
"(Comment \\ Repetitions)\n"
"(Comment )\n"
"(OperatorRule	Rep1N		(Seq (NT rule_elem_atom) (T +)))\n"
"(OperatorRule	Rep0N		(Seq (NT rule_elem_atom) (T *)))\n"
"(OperatorRule	Rep01		(Seq (NT rule_elem_atom) (T ?)))\n"
"(TransientRule	Rep			(Alt (NT Rep1N) (NT Rep0N) (NT Rep01)))\n"
"(Comment )\n"
"(Comment \\ Comments)\n"
"(Comment )\n"
"(OperatorRule	Comment	(Seq (T #) (Rep01 (Alt (Seq (NT Space) (RE [^\\r\\n]+)))) (NT NewLine)))\n"
"(OperatorRule	Comment	(RawSeq (T #) (Rep01 (RE [^\\r\\n]*)) (NT NewLine)))\n"
"))\n";

#endif



const char*test_rules = "((Grammar\n"
"(TransientRule	_start			(Seq (Alt"
										 /*"(NT test_postfix1)"*/
										 /*"(NT test_prefix1)"*/
										 /*"(NT test_leftrec)"*/
										 /*"(NT test_rep)"*/
										 /*"(NT test_alt1)"*/
										 "(NT test_alt_in_rep)"
									") (EOF)))"
"(OperatorRule	test_seq		(Seq (RE a) (RE b+) (EOF)))"
"(OperatorRule	test_alt1		(Alt (NT test_prefix1) (NT test_seq) (Seq (RE a) (RE b+) (EOF))))"
"(OperatorRule	test_prefix1	(Prefix (RE a) (NT PFX)))"
"(OperatorRule	test_postfix1	(Postfix (RE a) (NT PFX)))"
"(OperatorRule	PFX				(RE b+))"
"(OperatorRule	test_leftrec	(Alt (Seq (NT test_leftrec) (RE bb?)) (RE a)))"
"(TransientRule	test_rep		(Alt (NT test_r01) (NT test_r0N) (NT test_r1N)))"
"(OperatorRule	test_r01        (Seq (Rep01 (RE a)) (Rep01 (T toto)) (RE b+)))"
"(OperatorRule	test_r0N        (Seq (RE a) (Rep0N (RE b))))"
/*"(OperatorRule	test_r1N        (Seq (RE a) (Rep1N (RE b))))"*/
"(OperatorRule	test_r1N        (Rep1N (RE a?b)))"
"(OperatorRule  test_alt_in_rep (Rep0N (Alt (RE a) (RE b))))"
"))";





size_t node_pool_size();
extern volatile int _node_alloc_count;

ast_node_t tinyap_get_ruleset(const char*name) {
	struct stat st;
	char*buf;
	ast_node_t ret=NULL;
//	printf("before tinyap_get_ruleset : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	if(!strcmp(name,GRAMMAR_SHORT)) {
		ret=ast_unserialize(short_rules);
	} else if(!strcmp(name,"test")) {
		ret=ast_unserialize(test_rules);
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
