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
/*#include "hashtab.h"*/
#include "string_registry.h"
#include "registry.h"
#include <cstdlib>
#include <cstdio>
#include "static_init.h"

str_reg_t& str_registry = _static_init.str_reg;

//extern "C" {
//	const char* op2string(int typ); 	/* defined in tokenizer.c */
//}


/*unsigned int strreg_h(char*str) {*/
	/*return str?_srh(str):0;*/
/*}*/

/*struct _hashtable str_registry;*/



char* STR__start = NULL;
char* STR__whitespace = NULL;
char* STR_Grammar = NULL;
char* STR_Comment = NULL;
char* STR_eos = NULL;
char* STR_Alt = NULL;
char* STR_RawSeq = NULL;
char* STR_Seq = NULL;
char* STR_NT = NULL;
char* STR_Epsilon = NULL;
char* STR_EOF = NULL;
char* STR_Rep0N = NULL;
char* STR_Rep01 = NULL;
char* STR_Rep1N = NULL;
char* STR_RE = NULL;
char* STR_RPL = NULL;
char* STR_T = NULL;
char* STR_STR = NULL;
char* STR_BOW = NULL;
char* STR_AddToBag = NULL;
char* STR_BKeep = NULL;
char* STR_Prefix = NULL;
char* STR_Postfix = NULL;
char* STR_TransientRule = NULL;
char* STR_OperatorRule = NULL;
char* STR_Space = NULL;
char* STR_NewLine = NULL;
char* STR_Indent = NULL;
char* STR_Dedent = NULL;
char* STR_strip_me = NULL;

extern "C" {

void init_strreg() {
	/*init_hashtab(&str_registry, (hash_func) strreg_h, (compare_func) strcmp);*/
	/* pre-fill registry with all hardcoded strings used in the tokenizer */
    /*str_registry->clear();*/
	STR__whitespace		= regstr_impl("_whitespace");
	STR__start	        = regstr_impl("_start");
	STR_Grammar	        = regstr_impl("Grammar");
	STR_Comment	        = regstr_impl("Comment");
	STR_eos		        = regstr_impl("eos");
	STR_Alt		        = regstr_impl("Alt");
	STR_RawSeq	        = regstr_impl("RawSeq");
	STR_Seq		        = regstr_impl("Seq");
	STR_NT		        = regstr_impl("NT");
	STR_Epsilon	        = regstr_impl("Epsilon");
	STR_EOF		        = regstr_impl("EOF");
	STR_Rep0N	        = regstr_impl("Rep0N");
	STR_Rep01	        = regstr_impl("Rep01");
	STR_Rep1N	        = regstr_impl("Rep1N");
	STR_RE		        = regstr_impl("RE");
	STR_RPL		        = regstr_impl("RPL");
	STR_STR		        = regstr_impl("STR");
	STR_BOW		        = regstr_impl("BOW");
	STR_AddToBag		= regstr_impl("AddToBag");
	STR_BKeep	        = regstr_impl("BKeep");
	STR_T		        = regstr_impl("T");
	STR_Prefix	        = regstr_impl("Prefix");
	STR_Postfix	        = regstr_impl("Postfix");
	STR_TransientRule	= regstr_impl("TransientRule");
	STR_OperatorRule	= regstr_impl("OperatorRule");
	STR_Space	        = regstr_impl("Space");
	STR_NewLine	        = regstr_impl("NewLine");
	STR_Indent	        = regstr_impl("Indent");
	STR_Dedent	        = regstr_impl("Dedent");
	STR_strip_me		= regstr_impl("strip.me");
	/*atexit(deinit_strreg);*/
}

char* regstr_impl(const char* str) {
    auto it = str_registry.find(str);
    if (it != str_registry.end()) {
        it->second++;
    } else {
        bool uniq;
        std::tie(it, uniq) = str_registry.emplace(strdup(str), 1);
    }
    return const_cast<char*>(it->first);
    
    /*if (!str) {*/
        /*return NULL;*/
    /*}*/
    /*char* ret = const_cast<char*>(str_registry->ref(str).c_str());*/
    /*return ret;*/
}

void unregstr(const char*str) {
    auto it = str_registry.find(str);
    if (it != str_registry.end()) {
        it->second--;
        if (!it->second) {
            free(const_cast<char*>(it->first));
            str_registry.erase(it);
        }
    }
}



void deinit_strreg() {
	str_registry.clear();
}



}


/*struct _str_reg_init {*/
    /*_str_reg_init () { init_strreg(); }*/
    /*~_str_reg_init () { deinit_strreg(); }*/
/*};*/
