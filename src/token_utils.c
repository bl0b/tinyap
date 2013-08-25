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
#include "token_utils.h"
#include "string_registry.h"
#include "trie.h"
#include "serialize.h"
#include "tinyap.h"

#define _RE   2
#define _T    4


/*const char* ast_serialize_to_string(const ast_node_t ast, int show_offset);*/
/*void delete_node(ast_node_t n);*/

size_t hash_str(hash_key k);

#define node_tag(_x) Value(Car(_x))



ast_node_t find_nterm(const ast_node_t ruleset,const char*ntermid) {
	ast_node_t root=getCar(ruleset);
	ast_node_t n=getCdr(root);	/* skip tag */
	assert(!TINYAP_STRCMP(Value(getCar((ast_node_t )root)),STR_Grammar));	/* and be sure it made sense */
//	dump_node(n);
//	printf("\n");
	while(n&&((!TINYAP_STRCMP(node_tag(getCar(n)), STR_Comment))||TINYAP_STRCMP(node_tag(getCdr(getCar(n))),ntermid))) {	/* skip operator tag to fetch rule name */
//		debug_writeln("skip rule ");
//		dump_node(getCar(n));
		n=getCdr(n);
	}
	if(n) {
//		debug_writeln("FIND_NODE SUCCESSFUL\n");
//		dump_node(getCar(n));
		return getCar(n);
	}
	return NULL;
}


#if 0
const char* op2string(int typ) {
	switch(typ) {
	case OP_EOF: return STR_EOF;
	case OP_RE: return STR_RE;
	case OP_T: return STR_T;
	case OP_STR: return STR_STR;
	case OP_BOW: return STR_BOW;
	case OP_ADDTOBAG: return STR_AddToBag;
	case OP_BKEEP: return STR_AddToBag;
	case OP_RTR: return STR_TransientRule;
	case OP_ROP: return STR_OperatorRule;
	case OP_PREFX: return STR_Prefix;
	case OP_NT: return STR_NT;
	case OP_SEQ: return STR_Seq;
	case OP_ALT: return STR_Alt;
	case OP_POSTFX: return STR_Postfix;
	case OP_RAWSEQ: return STR_RawSeq;
	case OP_REP_0N: return STR_Rep0N;
	case OP_REP_01: return STR_Rep01;
	case OP_REP_1N: return STR_Rep1N;
	case OP_EPSILON: return STR_Epsilon;
	case OP_RPL: return STR_RPL;
	default: return NULL;
	};
}


int string2op(const char* tag) {
	int typ=0;
	if(!TINYAP_STRCMP(tag,STR_NT)) {
		typ = OP_NT;
	} else if(!TINYAP_STRCMP(tag,STR_Alt)) {
		typ = OP_ALT;
	} else if(!TINYAP_STRCMP(tag,STR_Seq)) {
		typ = OP_SEQ;
	} else if(!TINYAP_STRCMP(tag,STR_BOW)) {
		typ = OP_BOW;
	} else if(!TINYAP_STRCMP(tag,STR_BKeep)) {
		typ = OP_BKEEP;
	} else if(!TINYAP_STRCMP(tag,STR_AddToBag)) {
		typ = OP_ADDTOBAG;
	} else if(!TINYAP_STRCMP(tag,STR_RawSeq)) {
		typ = OP_RAWSEQ;
	} else if(!TINYAP_STRCMP(tag,STR_Rep0N)) {
		typ = OP_REP_0N;
	} else if(!TINYAP_STRCMP(tag,STR_Rep1N)) {
		typ = OP_REP_1N;
	} else if(!TINYAP_STRCMP(tag,STR_Rep01)) {
		typ = OP_REP_01;
	} else if(!TINYAP_STRCMP(tag,STR_RE)) {
		typ = OP_RE;
	} else if(!TINYAP_STRCMP(tag,STR_RPL)) {
		typ = OP_RPL;
	} else if(!TINYAP_STRCMP(tag,STR_T)) {
		typ = OP_T;
	} else if(!TINYAP_STRCMP(tag,STR_STR)) {
		typ = OP_STR;
	} else if(!TINYAP_STRCMP(tag,STR_Prefix)) {
		typ = OP_PREFX;
	} else if(!TINYAP_STRCMP(tag,STR_Postfix)) {
		typ = OP_POSTFX;
	} else if(!TINYAP_STRCMP(tag,STR_TransientRule)) {
		typ = OP_RTR;
	} else if(!TINYAP_STRCMP(tag,STR_OperatorRule)) {
		typ = OP_ROP;
	} else if(!TINYAP_STRCMP(tag,STR_EOF)) {
		typ = OP_EOF;
	} else if(!TINYAP_STRCMP(tag,STR_Epsilon)) {
		typ = OP_EPSILON;
	}
	/*fprintf(stderr, "      string2op(%s) = %i\n", tag, typ);*/
	return typ;
}
#endif


int dump_node(const ast_node_t n) {
	const char*ptr=tinyap_serialize_to_string(n);
	/*debug_writeln("%s", ptr);*/
	fputs(ptr, stdout);
	free((char*)ptr);
	return 0;
}









trie_t token_find_bow(parse_context_t pda, char* name) {
	trie_t ret = (trie_t) hash_find(&pda->bows, name);
	if(!ret) {
		ret = trie_new();
		hash_addelem(&pda->bows, name, ret);
	}
	return ret;
}

void token_bow_add(parse_context_t pda, char* name, char* word) {
	trie_insert(token_find_bow(pda, name), word);
}

unsigned long match_bow(parse_context_t pda, char*name) {
	return trie_match_prefix(token_find_bow(pda, name), pda->source+pda->ofs);
}

/* prepends a ^ to reg_expr and returns the compiled extended POSIX regexp */
RE_TYPE token_regcomp(const char*reg_expr) {
	int error_ofs;
	const char* error;
	RE_TYPE initiatur = pcre_compile(reg_expr, 0, &error, &error_ofs, NULL);
	/*fprintf(stderr, "Compiling regex \"%s\"\n", reg_expr);*/
	if(error) {
		fprintf(stderr, "Error : regex compilation of \"%s\" failed (%s at #%i)\n", reg_expr, error, error_ofs);
		return NULL;
	}
	return initiatur;
}

void escape_ncpy(char**dest, char**src, int count, const char* delim) {
	const char* base=*src;
	while( (*src-base) < count) {
		/*unescape_chr(src,dest, -1, delim);*/
		switch(**src) {
		case '\n' :
			**dest = '\\';
			*dest+=1;
			**dest = 'n';
			break;
		case '\t' :
			**dest = '\\';
			*dest+=1;
			**dest = 't';
			break;
		case '\r' :
			**dest = '\\';
			*dest+=1;
			**dest = 'r';
			break;
		default:
			**dest = **src;
		};
		*src+=1;
		*dest+=1;
	}
	**dest=0;
}



char*match2str(const char*src,const size_t start,const size_t end, const char*long_delim) {
	char* buf = _stralloc(end-start+1);
    char* ptr = buf;
    size_t ofs;
    for (ofs = start; ofs < end; ++ofs) {
        if (*(src + ofs) == '\\') {
            ++ofs;
            switch(*(src + ofs)) {
                /*case 'b': *ptr++ = '\b'; break;*/
                case 'r': *ptr++ = '\r'; break;
                case 't': *ptr++ = '\t'; break;
                case 'n': *ptr++ = '\n'; break;
                default: *ptr++ = '\\'; *ptr++ = *(src + ofs);
            };
        } else {
            *ptr++ = *(src + ofs);
        }
    }
    *ptr = 0;
    char* ret = regstr(buf);
    _strfree(buf);
    return ret;
}

#if 0
char*match2str(const char*src,const size_t start,const size_t end, const char*long_delim) {
	char* buf = _stralloc(end-start+1);
	char* rd = (char*)src+start;
	char* wr = buf;
	size_t sz = end-start-1, ofs = 0;
	char* ret = NULL;

	if(end>start) {
//	printf("match2str orig = \"%*.*s\" sz=%li\n",(int)(end-start),(int)(end-start),rd,sz);
//		memset(buf,0,end-start);
//	printf("              => \"%s\"\n",buf);
		while(ofs<sz) {
			unescape_chr_l(&rd, &wr, 0, long_delim);
			ofs = rd-src-start;
//		printf("match2str orig = \"%*.*s\"\n",(int)(end-start-ofs),(int)(end-start-ofs),rd);
//		printf("              => \"%s\" %p %p %li\n",buf,rd,buf,ofs);
		};
		if(ofs==sz) {
			*wr = *rd;
			wr += 1;
		}
	}
	*wr = 0;

//	printf("match2str => \"%s\"\n\n",buf);

//	static char buf[256];
//	memset(buf,0,256);
//	strncpy(buf,src+start,end-start);
	ret = regstr(buf);
	_strfree(buf);
	return ret;
}
#endif




#if 0

static inline const char* node_compare_tag(const char* n) {
	return n<((const char*)0x100) ? op2string((int)n) : n;
}

int node_compare(ast_node_t tok1, ast_node_t tok2) {
	if(tok1==tok2) {
		return 0;
	}
	if(isNil(tok1)) {
		return isNil(tok2)?0:-1;
	} else if(isNil(tok2)) {
		return 1;
	} else if(isAtom(tok1)) {
		return isPair(tok2)
			? 1
			: isAtom(tok2)
				? strcmp(node_compare_tag(Value(tok1)), node_compare_tag(Value(tok2)))
				: 0;
	} else if(isPair(tok1)) {
		if(isPair(tok2)) {
			int ret = node_compare(Car(tok1), Car(tok2));
			return ret?ret:node_compare(Cdr(tok1), Cdr(tok2));
		} else {
			return 1;
		}
	}

	return tok1>tok2?1:-1;
}


#endif

