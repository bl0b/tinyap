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

#include "tinyap.h"
#include "ast.h"
#include "tokenizer.h"

void ast_serialize(const ast_node_t ast,char**output);

int dump_node(const ast_node_t n) {
	const char*ptr=tinyap_serialize_to_string(n);
	debug_writeln(ptr);
	free((char*)ptr);
	return 0;
}


void update_pos_cache(token_context_t*t);




/* prepends a ^ to reg_expr and returns the compiled extended POSIX regexp */
regex_t*token_regcomp(const char*reg_expr) {
	static char buf[256];
	regex_t*initiatur=(regex_t*)malloc(sizeof(regex_t));
	sprintf(buf,"^%s",reg_expr);
	regcomp(initiatur,buf,REG_EXTENDED);
	return initiatur;
}




char*match2str(const char*src,const size_t start,const size_t end) {
	static char buf[256];
	memset(buf,0,256);
	strncpy(buf,src+start,end-start);
	return buf;
}



token_context_t*token_context_new(const char*src,const size_t length,const char*garbage_regex,ast_node_t greuh,size_t drapals) {
	token_context_t*t=(token_context_t*)malloc(sizeof(token_context_t));
	t->source=strdup(src);
	t->ofs=0;
	t->size=length;
	t->ofsp=0;
	t->flags=drapals;
	if(garbage_regex) {
		t->garbage=token_regcomp(garbage_regex);
	} else {
		t->garbage=NULL;
	}
	t->grammar=greuh;			/* grou la grammaire */
	t->farthest=0;
	t->pos_cache.last_ofs=0;
	t->pos_cache.last_nlofs=0;
	t->pos_cache.row=1;
	t->pos_cache.col=1;

	node_cache_init(t->cache);

	return t;
}



size_t token_context_peek(const token_context_t*t) {
	if(!t->ofsp)
		return 0;
	return t->ofstack[t->ofsp-1];
}



void token_context_push(token_context_t*t) {
	t->ofstack[t->ofsp]=t->ofs;
	t->ofsp+=1;
}



void token_context_validate(token_context_t*t) {
	/* TODO : implement node caching here */
	t->ofsp-=1;		/* release space on stack, don't update t->ofs */
	t->farthest=t->ofs;
}



void token_context_pop(token_context_t*t) {
	if(!t->ofsp)
		return;
	t->ofsp-=1;
	t->ofs=t->ofstack[t->ofsp];
}



void token_context_free(token_context_t*t) {
	if(t->garbage) {
		regfree(t->garbage);
	}
	node_cache_flush(t->cache);
	free(t->source);
	free(t);
}


/*
 * basic production rule from regexp : [garbage]token_regexp
 * return NULL on no match
 */

ast_node_t token_produce_re(token_context_t*t,const regex_t*expr) {
	regmatch_t token;
	char*ret;
	int r,c;
	/* perform some preventive garbage filtering */
	_filter_garbage(t);
	update_pos_cache(t);
	r=t->pos_cache.row;
	c=t->pos_cache.col;
	if(regexec(expr,t->source+t->ofs,1,&token,0)!=REG_NOMATCH&&token.rm_so==0) {
		assert(token.rm_so==0);
		ret=match2str(t->source+t->ofs,0,token.rm_eo);
		t->ofs+=token.rm_eo;
//		debug_write("debug-- matched token [%s]\n",ret);
		update_pos_cache(t);
		return newPair(newAtom(ret,t->pos_cache.row,t->pos_cache.col),NULL,r,c);
		//return newAtom(ret,t->pos_cache.row,t->pos_cache.col);
	} else {
//		debug_write("debug-- no good token\n");
	}
	return NULL;
}


ast_node_t token_produce_str(token_context_t*t,const char*token) {
	int r,c;
	_filter_garbage(t);
	update_pos_cache(t);
	r=t->pos_cache.row;
	c=t->pos_cache.col;
	if(!strncmp(t->source+t->ofs,token,strlen(token))) {
		t->ofs+=strlen(token);
		update_pos_cache(t);
		return newPair(newAtom(token,t->pos_cache.row,t->pos_cache.col),NULL,r,c);
		//return newAtom(token,t->pos_cache.row,t->pos_cache.col);
	}
	return NULL;
}

/*

elem = /.../
T ::= "\"" /.../ "\""
NT ::= "<" /.../ ">"
RE ::= "/" /.../ "/"

rule = ( <opr_rule> | <trans_rule> ) .

opr_rule ::= <Elem> "::=" <rule_expr> "." .
trans_rule ::= <Elem> "=" <rule_expr> "." .

rule_expr = ( "(" <alt> ")" | <seq> ) .

seq ::= ( <rule_elem> <rule_seq> | <rule_elem> ) .

alt ::= ( <rule_seq> "|" <alt> | <rule_seq> ) .

rule_elem = ( <TermID> | <NTermID> | <RegexID> | "(" <alt> ")" ) .

_start=(<Rule> <_start> | <rule>).

#
# The AST is defined by the parser result with all terminal tokens stripped
#
#

*/



ast_node_t  token_produce_any(token_context_t*t,ast_node_t expr,int strip_T);




#define node_tag(_x) Value(Car(_x))
#define node_cdr(_x) Value(Car(Cdr(_x)))



ast_node_t find_nterm(const ast_node_t ruleset,const char*ntermid) {
	ast_node_t root=getCar(ruleset);
	ast_node_t n=getCdr(root);	/* skip tag */
	assert(!strcmp(Value(getCar((ast_node_t )root)),"Grammar"));	/* and be sure it made sense */
//	dump_node(n);
//	printf("\n");
	while(n&&strcmp(node_tag(getCdr(getCar(n))),ntermid)) {	/* skip operator tag to fetch rule name */
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


ast_node_t _produce_seq_rec(token_context_t*t,ast_node_t seq) {
	ast_node_t tmp,rec;

	/* if seq is Nil, don't fail */
	if(!seq) {
		update_pos_cache(t);
		return newAtom("eos",t->pos_cache.row,t->pos_cache.col);
	}

	/* try and produce first token */
	tmp=token_produce_any(t,getCar(seq),t->flags&STRIP_TERMINALS);

	if(tmp) {
		/* try and produce rest of list */
		rec=_produce_seq_rec(t,getCdr(seq));
		if(rec) {
			update_pos_cache(t);
			if(isAtom(rec)) {
				assert(!strcmp(Value(rec),"eos"));
				delete_node(rec);
				//return newPair(tmp,NULL,t->pos_cache.row,t->pos_cache.col);
				return tmp;
			} else {
				//return newPair(tmp,rec,t->pos_cache.row,t->pos_cache.col);
				return Append(tmp,rec);
			}
		} else {
			/* FIXME : delay deletions until final cache flush */
			//delete_node(tmp);
			return NULL;
		}
	}
	return NULL;
}



ast_node_t  token_produce_seq(token_context_t*t,ast_node_t seq) {
	ast_node_t ret;

	/* try and produce seq */
	ret=_produce_seq_rec(t,seq);
/*	if(ret) {
		return newPair(ret,NULL,0,0);
	} else {
		return NULL;
	}*/
	return ret;
}



ast_node_t  token_produce_alt(token_context_t*t,ast_node_t alt) {
	ast_node_t tmp;

	/* if alt is Nil, fail */
	if(!alt) {
		return NULL;
	}

	tmp=token_produce_any(t,getCar(alt),t->flags&STRIP_TERMINALS);
	if(tmp) {
		/* select first matching alternative */
		//update_pos_cache(t);
		//return newPair(tmp,NULL,t->pos_cache.row,t->pos_cache.col);
		return tmp;
	} else {
		return token_produce_alt(t,getCdr(alt));
	}
}




#define OP_EOF 1
#define OP_RE  2
#define OP_T   3
#define OP_RTR 4
#define OP_ROP 5
#define OP_NT  6
#define OP_SEQ 7
#define OP_ALT 8



ast_node_t  token_produce_any(token_context_t*t,ast_node_t expr,int strip_T) {
	static int rec_lvl=0;
	char*tag;
	char*key=NULL;
	ast_node_t ret=NULL;
	int typ=0;
	int r,c;
	size_t dummy;
	int row,col;
	ast_node_t nt, re;

	if(!expr) {
		return NULL;
	}
	tag=node_tag(expr);

	row = t->pos_cache.row;
	col = t->pos_cache.col;

	if(!strcmp(tag,"Seq")) {
		typ = OP_SEQ;
	} else if(!strcmp(tag,"Alt")) {
		typ = OP_ALT;
	} else if(!strcmp(tag,"RE")) {
		typ = OP_RE;
//		key = Value(Car(Cdr(expr)));
	} else if(!strcmp(tag,"T")) {
		typ = OP_T;
//		key = Value(Car(Cdr(expr)));
	} else if(!strcmp(tag,"NT")) {
		typ = OP_NT;
		key = Value(Car(Cdr(expr)));
	} else if(!strcmp(tag,"TransientRule")) {
		typ = OP_RTR;
//		key = node_tag(Cdr(expr));
	} else if(!strcmp(tag,"OperatorRule")) {
		typ = OP_ROP;
//		key = node_tag(Cdr(expr));
	} else if(!strcmp(tag,"EOF")) {
		typ = OP_EOF;
	}

//	debug_write("--debug[% 4.4i]-- produce %s ",rec_lvl,tag);
//	dump_node(getCdr(expr));
//	printf("\n");
//	fprintf(stderr,"\tsource = %10.10s%s\n",t->source+t->ofs,t->ofs<(strlen(t->source)-10)?"...":"");



//*
	if(key&&node_cache_retrieve(t->cache, row, col, key, &ret,&t->ofs)) {
//		printf("found %s at %i:%i %s\n",key,row, col,tinyap_serialize_to_string(ret));
		update_pos_cache(t);
		return ret;
	}
//*/

	rec_lvl+=1;

	token_context_push(t);

	switch(typ) {
	case OP_SEQ:
		ret=token_produce_seq(t,getCdr(expr));
		break;
	case OP_ALT:
		ret=token_produce_alt(t,getCdr(expr));
		break;
	case OP_RE:		
		re = getCar(getCdr(expr));
		if(!re->raw._p2) {
			/* take advantage of unused atom field to implement regexp cache */
			re->raw._p2=token_regcomp(Value(re));
			/* FIXME : need call to regfree() on delete, should implement that in token_regcomp */
		}
		key = Value(re);
		ret=token_produce_re(t,re->raw._p2);
		break;
	case OP_T:
		ret=token_produce_str(t,Value(getCar(getCdr(expr))));
//		debug_write("### -=< term %s >=- ###\n",ret?"OK":"failed");
		if(ret) {
			if(strip_T) {
				delete_node(ret);
				ret=newPair(newAtom("strip.me",0,0),NULL,0,0);
//			} else {
//				update_pos_cache(t);
//				ret=newPair(ret,NULL,t->pos_cache.row,t->pos_cache.col);
			}
		}
		break;
	case OP_ROP:
		expr=getCdr(expr);	/* shift the operator tag */
//		dump_node(expr);
		update_pos_cache(t);
		r=t->pos_cache.row;
		c=t->pos_cache.col;
		tag=node_tag(expr);

		ret=token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
		if(ret) {
			ret=newPair(newPair(newAtom(tag,r,c),ret,r,c),NULL,r,c);
//			debug_write("Produce OperatorRule ");
//			dump_node(expr);
//			dump_node(ret);
//			fputc('\n',stdout);
//			printf("add to cache [ %li:%li:%s ] %p\n", t->pos_cache.row, t->pos_cache.col, tag, ret);
//			node_cache_add(t->cache, t->pos_cache.row, t->pos_cache.col, tag, ret);

		}
		break;
	case OP_RTR:
		expr=getCdr(expr);	/* shift the operator tag */
//		dump_node(expr);
		tag=node_tag(expr);

		ret=token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
		break;
	case OP_NT:
		tag = Value(getCar(getCdr(expr)));
		if(!node_cache_retrieve(t->cache,0,0,tag,&nt,&dummy)) {
			nt=find_nterm(t->grammar,tag);
			node_cache_add(t->cache,0,0,tag,nt,0);
		}
		
		if(!nt) {
			/* error, fail */
			debug_write("FAIL-- couldn't find non-terminal `%s'\n", tag);
			ret=NULL;
		} else {
			ret=token_produce_any(t,nt,0);
		}
		break;
	case OP_EOF:
		_filter_garbage(t);
		//if(*(t->source+t->ofs)&&t->ofs!=t->length) {
		if(t->ofs<t->size) {
			//debug_writeln("EOF not matched at #%u (against #%u)",t->ofs,t->size);
			ret=NULL;
		} else {
			//debug_writeln("EOF matched at #%u (against #%u)",t->ofs,t->size);
			update_pos_cache(t);
			ret=newPair(newAtom("strip.me",t->pos_cache.row,t->pos_cache.col),NULL,t->pos_cache.row,t->pos_cache.col);
		}
		break;
	};

	rec_lvl-=1;
	if(ret) {
		/* add to node cache */
		if(key) {
//			printf("add to cache [ %i:%i:%s ] %s\n", row, col, key, tinyap_serialize_to_string(ret));
			node_cache_add(t->cache,row,col,key,ret,t->ofs);
		}
		//dump_node(ret);
		token_context_validate(t);
		return ret;
	} else {
		token_context_pop(t);
		return NULL;
	}
}


//	if(!strcmp(tag,"Seq")) {
//	/* case seq : try and produce every subexpr, or fail. return whole cons'd list */
//		ret=token_produce_seq(t,getCdr(expr));
////		debug_write("### -=< seq %s >=- ###\n",ret?"OK":"failed");
////		if(ret) {
//			//debug_write("Produce Seq ");
//			//dump_node(expr);
//			//dump_node(ret);
////		}
//
//	} else if(!strcmp(tag,"Alt")) {
//	/* case alt : try and produce each subexpr, return the first whole production or fail */
//		ret=token_produce_alt(t,getCdr(expr));
////		debug_write("### -=< alt %s >=- ###\n",ret?"OK":"failed");
////		if(ret) {
////			debug_write("Produce Alt ");
////			dump_node(expr);
////			dump_node(ret);
////		}
//
//	} else if(!strcmp(tag,"RE")) {
//	/* case regex : call and return token_produce_re */
//		ast_node_t  re = getCar(getCdr(expr));
//		if(!re->raw._p2) {
//			/* take advantage of unused atom field to implement regexp cache */
//			re->raw._p2=token_regcomp(Value(re));
//			/* FIXME : need call to regfree() on delete, should implement that in token_regcomp */
//		}
//		key = Value(re);
//		//ret=newPair(token_produce_re(t,re->raw._p2),NULL);
//		ret=token_produce_re(t,re->raw._p2);
////		debug_write("### -=< regex %s >=- ###\n",ret?"OK":"failed");
//
//	} else if(!strcmp(tag,"T")) {
//	/* case term : call and return token_produce_str */
//		ret=token_produce_str(t,Value(getCar(getCdr(expr))));
////		debug_write("### -=< term %s >=- ###\n",ret?"OK":"failed");
//		if(ret) {
//			if(strip_T) {
//				delete_node(ret);
//				ret=newPair(newAtom("strip.me",0,0),NULL,0,0);
////			} else {
////				update_pos_cache(t);
////				ret=newPair(ret,NULL,t->pos_cache.row,t->pos_cache.col);
//			}
//		}
//
//	} else if(!strcmp(tag,"OperatorRule")) {
//	/* case operator rule : try and produce rule, return tagged parse tree */
//		expr=getCdr(expr);	/* shift the operator tag */
////		dump_node(expr);
//		update_pos_cache(t);
//		r=t->pos_cache.row;
//		c=t->pos_cache.col;
//		tag=node_tag(expr);
//
//		ret=token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
//		if(ret) {
//			ret=newPair(newPair(newAtom(tag,r,c),ret,r,c),NULL,r,c);
//			debug_write("Produce OperatorRule ");
//			dump_node(expr);
//			dump_node(ret);
//			fputc('\n',stdout);
////			printf("add to cache [ %li:%li:%s ] %p\n", t->pos_cache.row, t->pos_cache.col, tag, ret);
////			node_cache_add(t->cache, t->pos_cache.row, t->pos_cache.col, tag, ret);
//
//		}
//
//	} else if(!strcmp(tag,"TransientRule")) {
//	/* case transient rule : try and produce rule, return raw parse tree */
//		expr=getCdr(expr);	/* shift the operator tag */
////		dump_node(expr);
//		tag=node_tag(expr);
//
//		ret=token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
//
////		printf("add to cache [ %li:%li:%s ] %p\n", t->pos_cache.row, t->pos_cache.col, tag, ret);
////		node_cache_add(t->cache, t->pos_cache.row, t->pos_cache.col, tag, ret);
//
//	} else if(!strcmp(tag,"NT")) {
//	/* case nterm : call and return token_produce_nterm */
//		tag = Value(getCar(getCdr(expr)));
//		if(!node_cache_retrieve(t->cache,0,0,tag,&nt)) {
//			nt=find_nterm(t->grammar,tag);
//			node_cache_add(t->cache,0,0,tag,nt);
//		}
//		
//		if(!nt) {
//			/* error, fail */
//			debug_write("FAIL-- couldn't find non-terminal `%s'\n", tag);
//			ret=NULL;
//		} else {
//			ret=token_produce_any(t,nt,0);
//		}
////		debug_write("### -=< nterm %s %s >=- ###\n",Value(getCar(getCdr(expr))),ret?"OK":"failed");
//
//	} else if(!strcmp(tag,"EOF")) {
//		_filter_garbage(t);
//		//if(*(t->source+t->ofs)&&t->ofs!=t->length) {
//		if(t->ofs<t->size) {
//			//debug_writeln("EOF not matched at #%u (against #%u)",t->ofs,t->size);
//			ret=NULL;
//		} else {
//			//debug_writeln("EOF matched at #%u (against #%u)",t->ofs,t->size);
//			update_pos_cache(t);
//			ret=newPair(newAtom("strip.me",t->pos_cache.row,t->pos_cache.col),NULL,t->pos_cache.row,t->pos_cache.col);
//		}
////		debug_write("### -=< EOF %s >=- ###\n",ret?"OK":"failed");
//	}





ast_node_t clean_ast(ast_node_t t) {
	if(!t) {
		return NULL;
	}
	if(isAtom(t)) {
		if(strcmp(Value(t),"strip.me")) {
			return t;
		} else {
			delete_node(t);
			return NULL;
		}
	} else if(isPair(t)) {
		t->pair._car=clean_ast(t->pair._car);
		t->pair._cdr=clean_ast(t->pair._cdr);
		//if(t->pair._car==NULL&&t->pair._cdr==NULL) {
		if(t->pair._car==NULL) {
			ast_node_t cdr=t->pair._cdr;
			t->pair._cdr=NULL;
			delete_node(t);
			return cdr;
		}
	}
	return t;
}



void update_pos_cache(token_context_t*t) {
	int ln=1;		/* line number */
	size_t ofs=0;
	size_t end=t->ofs;
	size_t last_nlofs=0;

	if(t->ofs<t->pos_cache.last_ofs) {
		ln=1;
		ofs=0;
		last_nlofs=0;
	} else {
		ofs=t->pos_cache.last_ofs;
		ln=t->pos_cache.row;
		last_nlofs=t->pos_cache.last_nlofs;
	}

	while(ofs<end) {
		if(t->source[ofs]=='\n') {
			ln+=1;
			last_nlofs=ofs+1;
		}
		ofs+=1;
	}

	t->pos_cache.row=ln;
	t->pos_cache.col=1+end-last_nlofs;
	t->pos_cache.last_ofs=end;
	t->pos_cache.last_nlofs=last_nlofs;
}


const char* parse_error(token_context_t*t) {
	static char err_buf[1024];
	size_t last_nlofs=0;
	size_t next_nlofs=0;

	t->ofs=t->farthest;
	update_pos_cache(t);
	last_nlofs=t->ofs-t->pos_cache.col+1;
	
	next_nlofs=last_nlofs;
	while(t->source[next_nlofs]&&t->source[next_nlofs]!='\n') {
		next_nlofs+=1;
	}
	
//	sprintf(err_buf,"parse error at line %i :\n%*.*s\n%*.*s^\n",
	sprintf(err_buf,"%*.*s\n%*.*s^\n",
		(int)(next_nlofs-last_nlofs),
		(int)(next_nlofs-last_nlofs),
		t->source+last_nlofs,
		(int)(t->farthest-last_nlofs),
		(int)(t->farthest-last_nlofs),
		""
	);
	return err_buf;
}

int parse_error_column(token_context_t*t) {
	t->ofs=t->farthest;
	update_pos_cache(t);
	return t->pos_cache.col;
}

int parse_error_line(token_context_t*t) {
	t->ofs=t->farthest;
	update_pos_cache(t);
	return t->pos_cache.row;
}

