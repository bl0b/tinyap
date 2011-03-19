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

const char*short_rules = "((Grammar\n"
"(TransientRule toto (Rep01 (NT pouet)))"
"(TransientRule toto (RE should_be_merged_into_toto))"
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



const char*test_rules = "((Grammar\n"
"(TransientRule	_start			(Seq (Alt"
										 "(NT test_postfix1)"
										 "(NT test_prefix1)"
										 "(NT test_leftrec)"
										 "(NT test_rep)"
										 "(NT test_alt1)"
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
"(OperatorRule	test_r1N        (Seq (RE a) (Rep1N (RE b))))"
"(OperatorRule	test_r1N        (Rep1N (RE a?b)))"
"(OperatorRule  test_alt_in_rep (Rep0N (Alt (RE a) (RE b))))"
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
		ret=ast_unserialize(short_rules);
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
	//dump_node(ret);
	return ret;
}
