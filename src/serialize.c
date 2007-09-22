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


#define OPEN_PAR ((void*)(-1))
#define CLOSE_PAR ((void*)(-2))


#define MAX_SER_TOKEN_SIZE 16384	/* max 16k tokens */

const struct {
	char escaped;
	char unescaped;
} escape_characters[] = {
	{'n','\n'},
	{'t','\t'},
	{'r','\r'},
	{'\\','\\'},
	{'(','('},
	{')',')'},
	{'"','"'},
	{' ',' '},
	{0,0}
};


/* unescape first character in *src, put it in *dest, and advance pointers */
void unescape_chr(char**src,char**dest) {
	/* index to search for character escaping combination */
	int i;
	char ret=**src;
	if(!ret) {
		**dest=0;
		return;
	}
	*src+=1;
	if(ret=='\\') {
		i=0;
		/* there may be an escaped character following */
		while(escape_characters[i].escaped!=0&&**src!=escape_characters[i].escaped) {
			i+=1;
		}
		if(escape_characters[i].escaped) {
			/* if we do have an escaped character, swallow it before returning */
//			debug_writeln("unescaping \\%c",escape_characters[i].escaped);
			ret=escape_characters[i].unescaped;
			*src+=1;
		}
	}
	/* either ret is not \ or there's no valid escaped character following, thus we push raw ret in dest */
	**dest=ret;
	*dest+=1;
}





/* escape first character in *src, put it in *dest, and advance pointers */
void escape_chr(char**src,int(*func)(int,void*),void*param) {
	/* index to search for character escaping combination */
	int i=0;
	char ret=**src;
	if(!ret) {
		func(0,param);
		return;
	}

	/* search for an escaping combination for this character */
	while(escape_characters[i].unescaped!=0&&**src!=escape_characters[i].unescaped) {
		i+=1;
	}

	if(escape_characters[i].unescaped!=0) {
		/* have to escape character, two bytes will be pushed onto *dest */
		func('\\',param);
		func(escape_characters[i].escaped,param);
	} else {
		/* push raw **src into **dest */
		func(ret,param);
	}
	/* now dest is all set up, advance source */
	*src+=1;
}









char* usrlz_token(token_context_t*t,const char*whitespaces,const char*terminators) {
	static char buffer[MAX_SER_TOKEN_SIZE];

	char*srcptr,*destptr=buffer;
	int isTerminator;

	//debug_writeln("TOKENIZING %20.20s",t->source+t->ofs);

	/* strip whitespace */
	while(*(t->source+t->ofs)!=0&&strchr(whitespaces,*(t->source+t->ofs))!=NULL) {
		t->ofs+=1;
	}

	srcptr=t->source+t->ofs;

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
			unescape_chr(&srcptr,&destptr);
			isTerminator=strchr(terminators,*srcptr)||strchr(whitespaces,*srcptr);
		} while(*srcptr!=0 &&!isTerminator);
		t->ofs+=(size_t)(srcptr-t->source-t->ofs);
	}
	/* always append/overwrite the trailing \0 */
	*destptr=0;
	//debug_writeln("* <<%s>>",buffer);
	return buffer;
}




ast_node_t  _qlp_elem(token_context_t*t);

ast_node_t  _qlp_list(token_context_t*t) {
	ast_node_t car=_qlp_elem(t);
	if(car) {
//		printf("\tcar="); dump_node(car); printf("\n");
		//return newPair(car,_qlp_list(t,sym));
		return newPair(car,_qlp_list(t),0,0);
	}
	return NULL;
}

ast_node_t  _qlp_elem(token_context_t*t) {
/* ( => cons(_qlp_rec(t),_qlp_rec(t))
 * ) => NULL
 * \. => atom(.)
 * [^()]* => symbol
 */
	ast_node_t ret=NULL;
	char*cur=t->source+t->ofs;
	char*token;

	debug_enter();

	cur=t->source+t->ofs;

	token=usrlz_token(t,"\n\r\t ","()");
/*	t->ofs+=token_length;*/
	if(token==OPEN_PAR) {
		ret=_qlp_list(t);
	} else if(token==CLOSE_PAR) {
		ret=NULL;
	} else if(!*token) {
		ret=NULL;
	} else {
		//debug_write("token='%s' ",token);
		ret=newAtom(token,0,0);
	}

	debug_leave();
	return ret;
}


ast_node_t  ast_unserialize(const char*input) {
	token_context_t toktext;/*=token_context_new(input,"[\t\n\r ]*",NULL,0);*/
	toktext.ofs=0;
	toktext.source=(char*)input;
	return _qlp_elem(&toktext);	/* symbol is anything but parenthesis or whitespace */

}


void ast_serialize(const ast_node_t ast,int(*func)(int,void*),void*param);

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


void ast_serialize(const ast_node_t ast,int(*func)(int,void*),void*param) {
	char*srcptr;
	/* if ast is nil, output '()' */
	if(!ast) {
		func('(',param);
		func(')',param);
	/* if ast is pair, serialize list */
	} else if(isPair(ast)) {
		func('(',param);
		ast_ser_list(ast,func,param);
		func(')',param);
	/* if ast is atom, output atom */
	} else if(isAtom(ast)) {
		srcptr=getAtom(ast);
		while(*srcptr!=0) {
			escape_chr(&srcptr,func,param);
		}
/*		*output+=strlen(getAtom(ast));*/
	}
	func('\0',param);
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
	ast_serialize(ast,(int(*)(int,void*))fputc,(void*)f);
}
