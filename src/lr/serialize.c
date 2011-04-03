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
#include "serialize.h"
#include "ast.h"
#include "string_registry.h"
#include "parse_context.h"


#define OPEN_PAR ((void*)(-1))
#define CLOSE_PAR ((void*)(-2))


#define MAX_SER_TOKEN_SIZE 16384	/* max 16k tokens */

#define _LISP 1
#define _RE   2
#define _T    4

const struct _esc_chr escape_characters[] = {
	{'n','\n', -1 },
	{'t','\t', -1 },
	{'r','\r', -1 },
	{'\\','\\', -1 },
	/*{'/','/', _RE },*/
	{'"','"', _T },
	{'(','(', _LISP },
	{')',')', _LISP },
	{' ',' ', _LISP },
	{0,0,0}
};


const char* ctxt2str(int c) {
	switch(c) {
	case 0:	return "none";
	case 1:	return "LISP";
	case 2: return "RE";
	case 3: return "LISP+RE";
	case 4:	return "T";
	case 5: return "LISP+T";
	case 6: return "RE+T";
	case 7: return "LISP+RE+T";
	default:return "undef";
	};
}


void dump_contexts(char c, int cc, int cg) {
	printf("escape %c in contexts %s %s ? %s\n", c, ctxt2str(cc), ctxt2str(cg), (cc&cg)==cg?"OK.":"DON'T !");
}









char* usrlz_token(parse_context_t t,const char*whitespaces,const char*terminators, int context) {
	static char buffer[MAX_SER_TOKEN_SIZE];

	char*srcptr,*destptr=buffer;
	int isTerminator;

	//debug_writeln("TOKENIZING %20.20s",t->source+t->ofs);

	/* strip whitespace */
	while(*(t->source+t->ofs)!=0&&strchr(whitespaces,*(t->source+t->ofs))!=NULL) {
		t->ofs+=1;
	}

	srcptr=(char*)(t->source+t->ofs);

	if((!srcptr)||(!*srcptr)) {
		return "";
	} else if(*srcptr=='(') {
		t->ofs+=1;
		return OPEN_PAR;
		//debug_writeln("* %s PARENTHESIS",*srcptr=='('?"OPENING":"CLOSING"); /*)*/
	} else if(*srcptr==')') {
		t->ofs+=1;
		return CLOSE_PAR;
		//debug_writeln("* %s PARENTHESIS",*srcptr=='('?"OPENING":"CLOSING"); /*)*/
	} else {
		do {
			unescape_chr(&srcptr,&destptr, context, -1);
			isTerminator=strchr(terminators,*srcptr)||strchr(whitespaces,*srcptr);
		} while(*srcptr!=0 &&!isTerminator);
		t->ofs+=(size_t)(srcptr-t->source-t->ofs);
	}
	/* always append/overwrite the trailing \0 */
	*destptr=0;
	//debug_writeln("* <<%s>>",buffer);
	return buffer;
}




ast_node_t  _qlp_elem(parse_context_t t);

ast_node_t  _qlp_list(parse_context_t t) {
	ast_node_t car=_qlp_elem(t);
	if(t->source[t->ofs]=='@') {
		t->ofs+=2;
		car->node_flags|=IS_FOREST;
	}
	if(car) {
//		printf("\tcar="); dump_node(car); printf("\n");
		//return newPair(car,_qlp_list(t,sym));
		return newPair(car,_qlp_list(t));
	}
	return NULL;
}

ast_node_t  _qlp_elem(parse_context_t t) {
/* ( => cons(_qlp_rec(t),_qlp_rec(t))
 * ) => NULL
 * \. => atom(.)
 * [^()]* => symbol
 */
	ast_node_t ret=NULL;
	char*cur=(char*)(t->source+t->ofs);
	char*token;

	/*debug_enter();*/

	cur=(char*)(t->source+t->ofs);

	token=usrlz_token(t,"\n\r\t ","()", _LISP);
/*	t->ofs+=token_length;*/
	if(token==OPEN_PAR) {
		ret=_qlp_list(t);
	} else if(token==CLOSE_PAR) {
		ret=NULL;
	} else if(!*token) {
		ret=NULL;
	} else {
		/*debug_write("token='%s' ",token);*/
		ret=newAtom(token,0);
	}

	/*debug_leave();*/
	return ret;
}


ast_node_t  ast_unserialize(const char*input) {
	ast_node_t ret;
	struct parse_context toktext;/*=token_context_new(input,"[\t\n\r ]*",NULL,0);*/
	toktext.ofs=0;
	toktext.source=(char*)input;
	ret = _qlp_elem(&toktext);	/* symbol is anything but parenthesis or whitespace */

	return ret;
}


void ast_serialize(const ast_node_t ast,int(*func)(int,void*),void*param);

int file_put(int c,void*data) {
	FILE*f=(FILE*)data;
	if(c) {
		fputc(c,f);
	}
	return 1;
}

int str_put(int c,void*data) {
	char**output=(char**)data;
	//fputc(c,stdout);
	**output=(char)c;
	if(c) {
		*output+=1;
	}
	return 1;
}

int incr(int c,void*data) {
	unsigned int*counter=(unsigned int*)data;
	*counter+=1;
	return 1;
}

void ast_ser_list(const ast_node_t ast,int(*func)(int,void*),void*param) {
	/* FIXME shouldn't happen */
	if(isAtom(ast)) {
		ast_serialize(ast,func,param);
		return;
	}

	if(getCar(ast)) {
		ast_serialize(getCar(ast),func,param);
	}
	if(getCdr(ast)) {
		func(' ',param);
		ast_ser_list(getCdr(ast),func,param);
	}
}


extern ast_node_t PRODUCTION_OK_BUT_EMPTY;

void ast_serialize(const ast_node_t ast,int(*func)(int,void*),void*param) {
	char*srcptr;
	/* if ast is nil, output '()' */
	/*inside_lisp = 1;*/
	if(ast==PRODUCTION_OK_BUT_EMPTY) {
		func('E', param);
		func('M', param);
		func('P', param);
		func('T', param);
		func('Y', param);
		func(0, param);
		return;
	}
	if(!ast) {
		func('(',param);
		func(')',param);
	/* if ast is pair, serialize list */
	} else if(isPair(ast)) {
		func('(',param);
		if(ast->node_flags&IS_FOREST) {
		func('@',param);
		func(' ',param);
		}
		ast_ser_list(ast,func,param);
		func(')',param);
	/* if ast is atom, output atom */
	} else if(isAtom(ast)) {
		srcptr=regstr(getAtom(ast));
		while(*srcptr!=0) {
			escape_chr(&srcptr,func,param, _LISP);
		}
/*		*output+=strlen(getAtom(ast));*/
	}
	func('\0',param);
	/*inside_lisp = 0;*/
}


const char* ast_serialize_to_string(const ast_node_t ast) {
	unsigned int size=0;
	char*ret,*tmp;
	ast_serialize(ast,incr,(void*)&size);
//	printf("serialize needs %i bytes\n",size);
	tmp=ret=(char*)malloc(size+1);
	ast_serialize(ast,str_put,(void*)&tmp);
//	printf("serialized to %s (%i)\n",ret,strlen(ret));
	return ret;
}

void ast_serialize_to_file(const ast_node_t ast,FILE*f) {
	ast_serialize(ast,file_put,(void*)f);
}

