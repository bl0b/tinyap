/* TinyaP : this is not yet another (Java) parser.
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

const char* short_rules_2 =
"((Grammar (Comment #\\ TinyaP\\ :\\ this\\ is\\ not\\ yet\\ another\\ ) (Comment #\\ Copyright\\ \\(C\\)\\ 2007-2011\\ Damien\\ Leroux) (Comment #) (Comment #\\ This\\ program\\ is\\ free\\ software;\\ you\\ can\\ redistribute\\ it\\ and/or) (Comment #\\ modify\\ it\\ under\\ the\\ terms\\ of\\ the\\ GNU\\ General\\ Public\\ License) (Comment #\\ as\\ published\\ by\\ the\\ Free\\ Software\\ Foundation;\\ either\\ version\\ 2) (Comment #\\ of\\ the\\ License,\\ or\\ \\(at\\ your\\ option\\)\\ any\\ later\\ version.) (Comment #) (Comment #\\ This\\ program\\ is\\ distributed\\ in\\ the\\ hope\\ that\\ it\\ will\\ be\\ useful,)"
" (Comment #\\ but\\ WITHOUT\\ ANY\\ WARRANTY;\\ without\\ even\\ the\\ implied\\ warranty\\ of) (Comment #\\ MERCHANTABILITY\\ or\\ FITNESS\\ FOR\\ A\\ PARTICULAR\\ PURPOSE.\\ \\ See\\ the) (Comment #\\ GNU\\ General\\ Public\\ License\\ for\\ more\\ details.) (Comment #) (Comment #\\ You\\ should\\ have\\ received\\ a\\ copy\\ of\\ the\\ GNU\\ General\\ Public\\ License) (Comment #\\ along\\ with\\ this\\ program;\\ if\\ not,\\ write\\ to\\ the\\ Free\\ Software) (Comment #\\ Foundation,\\ Inc.,\\ 59\\ Temple\\ Place\\ -\\ Suite\\ 330,\\ Boston,\\ MA\\ \\ 02111-1307,\\ USA.) (Comment #) "
" (Comment #) (Comment #\\ Production\\ Atoms) (Comment #)"
" (TransientRule symbol (RE [_a-zA-Z][0-9a-zA-Z_]*)) (OperatorRule T (STR \" \")) (OperatorRule NT (NT symbol)) (OperatorRule RE (STR / /)) (OperatorRule STR (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))) (OperatorRule BOW (RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (RE !?) (T ~))) (OperatorRule AddToBag (Seq (NT RE) (T :) (NT symbol) (RE !?)))"
" (Comment #) (Comment #\\ Compositions) (Comment #)"
" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (Seq (Space) (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))))) (TransientRule alt_elem (Alt (NT RawSeq) (NT Seq) (NT single))) (OperatorRule Seq (Seq (NT single) (Rep1N (Seq (Space) (NT single))))) (OperatorRule Alt (Seq (NT alt_elem) (Rep1N (Seq (Space) (T |) (Space) (NT alt_elem)))))"
" (OperatorRule Rep01 (Seq (NT single_norep) (T ?))) (OperatorRule Rep1N (Seq (NT single_norep) (T +))) (OperatorRule Rep0N (Seq (NT single_norep) (T *))) (TransientRule single (Alt (NT Rep01) (NT Rep0N) (NT Rep1N) (NT single_norep) (NT Space) (NT NewLine) (NT Indent) (NT Dedent))) (TransientRule single_norep (Alt (NT Prefix) (NT Postfix) (NT NT) (NT STR) (NT BOW) (NT AddToBag) (NT T) (NT RE) (NT Epsilon) (NT EOF) (NT sub_rmb)))"
"(OperatorRule Prefix (Seq (T [) (NT rmember) (T ]) (NT NT)))"
"(OperatorRule Postfix (Seq (T {) (NT rmember) (T }) (NT NT)))"
" (Comment #) (Comment #\\ Top-level) (Comment #)"
" (OperatorRule TransientRule (Seq (NT symbol) (Space) (T =) (Space) (NT rmember) (T .) (NewLine)))"
" (OperatorRule OperatorRule (Seq (NT symbol) (Space) (T ::=) (Space) (NT rmember) (T .) (NewLine))) (TransientRule rmember (Alt (NT Alt) (NT alt_elem))) (TransientRule sub_rmb (Seq (T \\() (Alt (NT Seq) (NT Alt) (NT RawSeq)) (T \\))))"
/*" (OperatorRule Comment (Alt (Seq (RawSeq (T #) (RE .+|$)) (NewLine)) (Seq (T #) (NewLine))))"*/
" (OperatorRule Comment (Seq (RE #.*) (NewLine)))"
" (OperatorRule Grammar (Rep1N (NT gram_toplevel))) (TransientRule gram_toplevel (Alt (NT TransientRule) (NT OperatorRule) (NT Comment)))"
" (Comment #) (Comment #\\ Miscellaneous) (Comment #)"
" (TransientRule _start (NT Grammar)) (TransientRule _whitespace (RE [\\ \\r\\n\\t]+)) (OperatorRule EOF (T .eof)) (OperatorRule Epsilon (T .epsilon)) (OperatorRule Space (T .space)) (OperatorRule NewLine (T .newline)) (OperatorRule Indent (T .indent)) (OperatorRule Dedent (T .dedent))))";


const char*short_rules = "((Grammar\n"
"(Comment \\ TinyaP\\ :\\ this\\ is\\ not\\ yet\\ another\\ (Java)\\ parser.)"
"(Comment \\ Copyright\\ \\(C\\)\\ 2007-2011\\ Damien\\ Leroux)"
"(Comment )"
"(Comment \\ This\\ program\\ is\\ free\\ software;\\ you\\ can\\ redistribute\\ it\\ and/or)"
"(Comment \\ modify\\ it\\ under\\ the\\ terms\\ of\\ the\\ GNU\\ General\\ Public\\ License)"
"(Comment \\ as\\ published\\ by\\ the\\ Free\\ Software\\ Foundation;\\ either\\ version\\ 2)"
"(Comment \\ of\\ the\\ License,\\ or\\ \\(at\\ your\\ option\\)\\ any\\ later\\ version.)"
"(Comment )"
"(Comment \\ This\\ program\\ is\\ distributed\\ in\\ the\\ hope\\ that\\ it\\ will\\ be\\ useful,)"
"(Comment \\ but\\ WITHOUT\\ ANY\\ WARRANTY;\\ without\\ even\\ the\\ implied\\ warranty\\ of)"
"(Comment \\ MERCHANTABILITY\\ or\\ FITNESS\\ FOR\\ A\\ PARTICULAR\\ PURPOSE.\\ \\ See\\ the)"
"(Comment \\ GNU\\ General\\ Public\\ License\\ for\\ more\\ details.)"
"(Comment )"
"(Comment \\ You\\ should\\ have\\ received\\ a\\ copy\\ of\\ the\\ GNU\\ General\\ Public\\ License)"
"(Comment \\ along\\ with\\ this\\ program;\\ if\\ not,\\ write\\ to\\ the\\ Free\\ Software)"
"(Comment \\ Foundation,\\ Inc.,\\ 59\\ Temple\\ Place\\ -\\ Suite\\ 330,\\ Boston,\\ MA\\ \\ 02111-1307,\\ USA.)"
"(Comment )"
"(Comment )"
"(Comment \\ Production\\ Atoms)\n"
"(Comment )"
"(TransientRule	symbol			(RE [_a-zA-Z][0-9a-zA-Z_]*))\n"
"(OperatorRule	T				(STR \" \"))\n"
"(OperatorRule	NT				(NT symbol))\n"
"(OperatorRule	RE				(STR / /))\n"
"(OperatorRule	STR				(RawSeq (T ~) (RE \\\\\\\\?[^~,]?) (T ,)"
" 										(RE \\\\\\\\?[^~,]?) (T ~)))\n"
"(OperatorRule	BOW				(RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (RE !?) (T ~)))\n"
"(OperatorRule	AddToBag		(Seq (NT RE) (T :) (NT symbol) (RE !?)))\n"
"(Comment )"
"(Comment \\ Compositions)\n"
"(Comment )"
/* FIXME : this construct is buggy ! */
"(OperatorRule	RawSeq			(Seq (T .raw) (Rep1N (Seq (Space) (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag))))))"
/*"(OperatorRule	RawSeq			(Seq (T .raw) (Rep1N (NT rawseq_cts))))"*/
/*"(TransientRule	rawseq_cts		(Seq (Space) (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag))))"*/
"(TransientRule	alt_elem		(Alt (NT RawSeq) (NT Seq) (NT single)))\n"
"(OperatorRule	Seq				(Seq (NT single) (Rep1N (Seq (Space) (NT single)))))\n"
"(OperatorRule	Alt				(Seq (NT alt_elem) (Rep1N (Seq (Space) (T |) (Space) (NT alt_elem)))))\n"
"(OperatorRule	Rep01			(Seq (NT single_norep) (T ?)))\n"
"(OperatorRule	Rep1N			(Seq (NT single_norep) (T +)))\n"
"(OperatorRule	Rep0N			(Seq (NT single_norep) (T *)))\n"
"(TransientRule	single			(Alt (NT Rep01) (NT Rep0N) (NT Rep1N) (NT single_norep) (NT EOF) (NT Space) (NT NewLine) (NT Indent) (NT Dedent)))\n"
"(TransientRule	single_norep	(Alt (NT Prefix) (NT Postfix) (NT NT) (NT STR) (NT BOW) (NT AddToBag) (NT T) (NT RE) (NT sub_rmb)))\n"
"(OperatorRule	Prefix			(Seq (T [) (NT rmember) (T ]) (NT NT)))"
"(OperatorRule	Postfix			(Seq (T {) (NT rmember) (T }) (NT NT)))"
"(Comment )"
"(Comment \\ Top-level)\n"
"(Comment )"
"(OperatorRule	TransientRule	(Seq (NT symbol) (Space) (T =) (Space) (NT rmember) (T .) (NewLine)))\n"
"(OperatorRule	OperatorRule	(Seq (NT symbol) (Space) (T ::=) (Space) (NT rmember) (T .) (NewLine)))\n"
"(TransientRule	rmember			(Alt (NT Epsilon) (NT Alt) (NT alt_elem)))\n"
"(TransientRule	sub_rmb			(Seq (T \\() (Alt (NT Epsilon) (NT Seq) (NT Alt) (NT RawSeq)) (T \\))))\n"
"(OperatorRule	Comment			(STR # \\n))\n"
/*"(OperatorRule Comment (Seq (RawSeq (T #) (RE [^\\\\r\\\\n]*)) (Space)))"*/
"(OperatorRule	Grammar			(Rep1N (NT gram_toplevel)))"
"(TransientRule	gram_toplevel	(Alt (NT TransientRule)"
"									 (NT OperatorRule)"
"							 		 (NT Comment)))"
"(Comment )"
"(Comment \\ Miscellaneous)\n"
"(Comment )"
"(TransientRule	_start			(NT Grammar))\n"
"(TransientRule	_whitespace		(RE [\\ \\\\r\\\\n\\t]+))\n"
"(OperatorRule	EOF				(T .eof))\n"
"(OperatorRule	Epsilon			(T .epsilon))\n"
"(OperatorRule	Space			(T .space))\n"
"(OperatorRule	NewLine			(T .newline))\n"
"(OperatorRule	Indent			(T .indent))\n"
"(OperatorRule	Dedent			(T .dedent))\n"
/*"(OperatorRule	BKeep			(T !))\n"*/
/*"(TransientRule	elem_atom		(Alt (NT NT) (NT STR) (NT BOW) (NT AddToBag)"*/
/*" 									 (NT T) (NT RE)\n"*/
/*"									 (NT Epsilon) (NT EOF) \n"*/
/*"									 (Seq (T \\() (NT elem_comp) (T \\)))\n"*/
/*"								))\n"*/
/*"(TransientRule	elem			(Alt (NT elem_atom) (NT Rep01) (NT Rep0N)"*/
/*" 									 (NT Rep1N)))\n"*/
/*"(TransientRule	elem_seq_struc 	(Seq (NT elem) (Space) (NT _ess_loop)))"*/
/*"(TransientRule _ess_loop 		(Alt (Seq (NT elem) (Space) (NT _ess_loop))"*/
									/*"(NT elem)))"*/
/*"(TransientRule	elem_seq_struc 	(Alt (NT elem)"*/
									/*"(Seq (NT elem_seq_struc) (Space)"*/
 										 /*"(NT elem))))"*/
/*"(TransientRule	elem_seq_struc 	(Seq (NT elem) (Rep1N (Seq (Space) (NT elem)))))\n"*/
/*"(TransientRule	elem_seq 		(Alt (NT RawSeq) (NT Seq) (NT elem)))\n"*/
"))";



const char*test_rules = "((Grammar\n"
"(TransientRule	_start			(Seq (Alt"
										 /*"(NT test_postfix1)"*/
										 /*"(NT test_prefix1)"*/
										 /*"(NT test_leftrec)"*/
										 "(NT test_epsilon)"
										 "(NT test_rep)"
										 /*"(NT test_alt1)"*/
										 /*"(NT test_alt_in_rep)"*/
									") (EOF)))"
"(OperatorRule	test_epsilon	(Epsilon))"
/*"(OperatorRule	test_seq		(Seq (RE a) (RE b+) (EOF)))"*/
/*"(OperatorRule	test_alt1		(Alt (NT test_prefix1) (NT test_seq) (Seq (RE a) (RE b+) (EOF))))"*/
/*"(OperatorRule	test_prefix1	(Prefix (RE a) (NT PFX)))"*/
/*"(OperatorRule	test_postfix1	(Postfix (RE a) (NT PFX)))"*/
/*"(OperatorRule	PFX				(RE b+))"*/
/*"(OperatorRule	test_leftrec	(Alt (Seq (NT test_leftrec) (RE bb?)) (RE a)))"*/
"(TransientRule	test_rep		(Alt (NT test_r01) (NT test_r0N) (NT test_r1N)))"
"(OperatorRule	test_r01        (Seq (Rep01 (RE a)) (Rep01 (T toto)) (RE b+)))"
"(OperatorRule	test_r0N        (Seq (RE a) (Rep0N (RE b))))"
"(OperatorRule	test_r1N        (Seq (RE a) (Rep1N (RE b))))"
"(OperatorRule	test_r1N        (Rep1N (RE a?b)))"
/*"(OperatorRule  test_alt_in_rep (Rep0N (Alt (RE a) (RE b))))"*/
"))";




const char*slr_rules = "((Grammar\n"
"(TransientRule	_start	(NT S))"
"(OperatorRule	S		(Alt (Seq (NT L) (T =) (NT R)) (NT R)))"
"(OperatorRule	L		(Alt (Seq (T *) (NT R)) (RE id)))"
"(OperatorRule	R		(NT L))"
"))";


const char* debug = "((Grammar"
"(TransientRule	_start (NT debug_iter))"
"(OperatorRule	debug_iter (Alt (T a) (T b) (Seq (T a) (NT debug_iter))))"
"))";


const char* debug_nl = "((Grammar"
"(TransientRule	_start	(NT sentence))"
"(OperatorRule	sentence		(Seq (NT n_p) (NT v_p)))"
"(OperatorRule	sentence		(Seq (NT sentence) (NT p_p)))"
"(OperatorRule	n_p		(NT n))"
"(OperatorRule	n_p		(Seq (NT det) (NT n)))"
"(OperatorRule	n_p		(Seq (NT n_p) (NT p_p)))"
"(OperatorRule	p_p		(Seq (NT prep) (NT n_p)))"
"(OperatorRule	v_p		(Seq (NT v) (NT n_p)))"
"(TransientRule	det		(RE a|the))"
"(TransientRule	prep	(RE in|with))"
"(TransientRule	n		(RE I|man|telescope|park))"
"(TransientRule	v		(RE saw))"
"))";



size_t node_pool_size();
extern volatile int _node_alloc_count;

ast_node_t  tinyap_get_ruleset(const char*name) {
	struct stat st;
	char*buf;
	ast_node_t ret=NULL;
//	printf("before tinyap_get_ruleset : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	if(!strcmp(name,GRAMMAR_SHORT)) {
		ret=ast_unserialize(short_rules_2);
	} else if(!strcmp(name,"test")) {
		ret=ast_unserialize(test_rules);
	} else if(!strcmp(name,"slr")) {
		ret=ast_unserialize(slr_rules);
	} else if(!strcmp(name,"debug")) {
		ret=ast_unserialize(debug);
	} else if(!strcmp(name,"debug_nl")) {
		ret=ast_unserialize(debug_nl);
	} else if(!stat(name,&st)) {
		/* unserialize from file */
		FILE*f=fopen(name,"r");
		buf=(char*)malloc(st.st_size+1);
		if(fread(buf,1,st.st_size,f)) {}
		buf[st.st_size]=0;
		fclose(f);
		ret=ast_unserialize(buf);
		free(buf);
	}
//	printf("after  tinyap_get_ruleset : %li nodes (%i alloc'd so far)\n",node_pool_size(),_node_alloc_count);
	/*dump_node(ret);*/
	return ret;
}
