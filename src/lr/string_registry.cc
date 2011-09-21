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
#include <cstdlib>
#include <cstdio>
#include <ext/hash_map>

//extern "C" {
//	const char* op2string(int typ); 	/* defined in tokenizer.c */
//}


static inline unsigned int _srh(const char*notnull) {
	register unsigned int accum = 0;
//	if((unsigned int)notnull<0x100) {
//		 /* suspect an optimized tag (not-a-string) */
//		notnull = op2string((int)notnull);
//	}
	while(*notnull) {
		/*accum = (accum<<5)^((accum>>27) | (int)*notnull);*/
		accum = (accum<<7) + *notnull;
		++notnull;
	}
	/*return accum%HASH_SIZE;*/
	return accum;
}

unsigned int strreg_h(char*str) {
	return str?_srh(str):0;
}


struct k_h {
	size_t operator()(const char*x) const {
		return x?_srh(x):0;
	}
};

struct k_cmp {
	bool operator()(const char*a, const char*b) const {
		return !strcmp(a, b);
	}
};

typedef __gnu_cxx::hash_map<const char*, int, k_h, k_cmp> str_reg_t;
static str_reg_t str_registry;

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
	STR__whitespace		= regstr_impl("_whitespace");
	STR__start		= regstr_impl("_start");
	STR_Grammar		= regstr_impl("Grammar");
	STR_Comment		= regstr_impl("Comment");
	STR_eos			= regstr_impl("eos");
	STR_Alt			= regstr_impl("Alt");
	STR_RawSeq		= regstr_impl("RawSeq");
	STR_Seq			= regstr_impl("Seq");
	STR_NT			= regstr_impl("NT");
	STR_Epsilon		= regstr_impl("Epsilon");
	STR_EOF			= regstr_impl("EOF");
	STR_Rep0N		= regstr_impl("Rep0N");
	STR_Rep01		= regstr_impl("Rep01");
	STR_Rep1N		= regstr_impl("Rep1N");
	STR_RE			= regstr_impl("RE");
	STR_RPL			= regstr_impl("RPL");
	STR_STR			= regstr_impl("STR");
	STR_BOW			= regstr_impl("BOW");
	STR_AddToBag		= regstr_impl("AddToBag");
	STR_BKeep		= regstr_impl("BKeep");
	STR_T			= regstr_impl("T");
	STR_Prefix		= regstr_impl("Prefix");
	STR_Postfix		= regstr_impl("Postfix");
	STR_TransientRule	= regstr_impl("TransientRule");
	STR_OperatorRule	= regstr_impl("OperatorRule");
	STR_Space		= regstr_impl("Space");
	STR_NewLine		= regstr_impl("NewLine");
	STR_Indent		= regstr_impl("Indent");
	STR_Dedent		= regstr_impl("Dedent");
	STR_strip_me		= regstr_impl("strip.me");
	/*atexit(deinit_strreg);*/
}

char* regstr_impl(const char* str) {
	str_reg_t::iterator i = str_registry.find(str);
	const char* ret;
	if(i==str_registry.end()) {
		ret = _strdup(str);
		str_registry[ret] = 1;
	} else {
		ret = (*i).first;
		(*i).second+=1;
	}
	return (char*)ret;
}

void unregstr(const char*str) {
	str_reg_t::iterator i = str_registry.find(str);
	if(!str) { return; }
	if(i!=str_registry.end()) {
		--(*i).second;
		if((*i).second<=0) {
			char* x = (char*) (*i).first;
			str_registry.erase(i);
			_strfree(x);
		}
		return;
	}
	_strfree((char*)str);
}



void deinit_strreg() {
	/*clean_hashtab(&str_registry, _free_key);*/
#if 1
	str_reg_t::iterator i, j=str_registry.end();
	std::vector<char*> stack;
	stack.reserve(str_registry.size());
	for(i=str_registry.begin();i!=j;++i) {
		if((*i).first) {
			stack.push_back((char*)(*i).first);
		}
	}
	std::vector<char*>::iterator vi, vj = stack.end();
	for(vi=stack.begin();vi!=vj;++vi) {
		_strfree(*vi);
	}
#endif
	str_registry.clear();
}



}

#if 0

char* regstr(const char* str) {
	if(!str) {
		return "{null}";
	} else {
		htab_entry_t e = hash_find_e(&str_registry, (hash_key) (str<(char*)0x100?op2string((int)str):str));
		char* ret;
		if(!e) {
			ret = _strdup(str);
			hash_addelem(&str_registry, (hash_key) ret, (hash_elem) 1);
			/*printf("register string \"%s\" (1)\n", ret);*/
		} else {
			e->e = (hash_elem) (((unsigned int)e->e)+1);
			ret = (char*) e->key;
			/*printf("register string \"%s\" (%u)\n", ret, (unsigned int)e->e);*/
		}
		return ret;
	}
}

void unregstr(const char* str) {
	htab_entry_t e = hash_find_e(&str_registry, (hash_key) (str<(char*)0x100?op2string((int)str):str));
	if(!e) {
		/* whine ? */
		return;
	}
	e->e = (hash_elem) (((unsigned int)e->e)-1);
	if(!e->e) {
		char*k = e->key;
		hash_delelem(&str_registry, (hash_key) str);
		_strfree(k);
	}
}

void _free_key(htab_entry_t e) {
	_strfree(e->key);
}

void deinit_strreg() {
	clean_hashtab(&str_registry, _free_key);
}

#endif


