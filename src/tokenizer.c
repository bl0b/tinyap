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
#include "tinyap.h"
#include "ast.h"
#include "tokenizer.h"
#include "tinyap_alloc.h"
#include "string_registry.h"

void ast_serialize(const ast_node_t ast,char**output);
void unescape_chr(char**src,char**dest, int context);
void delete_node(ast_node_t n);
ast_node_t copy_node(ast_node_t);


ast_node_t PRODUCTION_OK_BUT_EMPTY = (union _ast_node_t[]){{ {0, 0, 0, 0, 0} }};

ast_node_t SafeAppend(ast_node_t a, ast_node_t b) {
	return a==PRODUCTION_OK_BUT_EMPTY
		? b
		: b==PRODUCTION_OK_BUT_EMPTY
			? a
			: Append(a, b);
}

int dump_node(const ast_node_t n) {
	const char*ptr=tinyap_serialize_to_string(n);
	debug_writeln("%s", ptr);
	free((char*)ptr);
	return 0;
}


void __fastcall update_pos_cache(token_context_t*t);


#define _RE   2
#define _T    4


/* prepends a ^ to reg_expr and returns the compiled extended POSIX regexp */
regex_t*token_regcomp(const char*reg_expr) {
	static char buf[1024];
	regex_t*initiatur=tinyap_alloc(regex_t);
	sprintf(buf,"^%s",reg_expr);
	regcomp(initiatur,buf,REG_EXTENDED);
	return initiatur;
}

void escape_ncpy(char**dest, char**src, int count, int context) {
	const char* base=*src;
	while( (*src-base) < count) {
		unescape_chr(src,dest, context);
	}
}

char* match2str_rpl(const char*repl, const char* match, int n_tok, regmatch_t* tokens) {
	char* rbuf = _stralloc(strlen(repl)+strlen(match));
	char*dest=rbuf;
	char*src=(char*)repl,*subsrc;

	while(*src) {
		if(*src=='\\'&& *(src+1)>='0' && *(src+1)<='9') {
			int n = *(src+1)-'0';
			subsrc = (char*) (match+tokens[n].rm_so);
			escape_ncpy(&dest,&subsrc, tokens[n].rm_eo-tokens[n].rm_so, _RE);
			src+=2;
		} else {
			unescape_chr(&src,&dest, -1);
		}
	}
	*dest=0;
	/*return _strdup(rbuf);*/
	return rbuf;
	/*return regstr(rbuf);*/
}



char*match2str(const char*src,const size_t start,const size_t end) {
	char* buf = _stralloc(end-start+1);
	char* rd = (char*)src+start;
	char* wr = buf;
	size_t sz=end-start-1,ofs=0;

	if(end>start) {
//	printf("match2str orig = \"%*.*s\" sz=%li\n",(int)(end-start),(int)(end-start),rd,sz);
//		memset(buf,0,end-start);
//	printf("              => \"%s\"\n",buf);
		while(ofs<sz) {
			unescape_chr(&rd, &wr, _RE);
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
//		printf("t->garbage = \"%s\"\n",garbage_regex);
		t->garbage=token_regcomp(garbage_regex);
	} else {
//		printf("t->garbage = NULL\n");
		t->garbage=NULL;
	}
	t->grammar=greuh;			/* grou la grammaire */
	t->farthest=0;
	t->farthest_stack = new_stack();
	t->node_stack = new_stack();
	t->pos_cache.last_ofs=0;
	t->pos_cache.last_nlofs=0;
	t->pos_cache.row=1;
	t->pos_cache.col=1;

	node_cache_init(t->cache);

	return t;
}



static inline size_t token_context_peek(const token_context_t*t) {
	if(!t->ofsp)
		return 0;
	return t->ofstack[t->ofsp-1];
}



static inline void token_context_push(token_context_t*t, const char*tag) {
	t->ofstack[t->ofsp]=t->ofs;
	t->ofsp+=1;
	push(t->node_stack,(void*)tag);
}



static inline void token_context_validate(token_context_t*t) {
	/* TODO : implement node caching here */
	t->ofsp-=1;		/* release space on stack, don't update t->ofs */
	if(t->farthest<t->ofs) {
		t->farthest=t->ofs;
		free_stack(t->farthest_stack);
		t->farthest_stack = stack_dup(t->node_stack);
	}
	_pop(t->node_stack);
}



static inline void token_context_pop(token_context_t*t) {
	_pop(t->node_stack);
	if(!t->ofsp)
		return;
	t->ofsp-=1;
	t->ofs=t->ofstack[t->ofsp];
}



void token_context_free(token_context_t*t) {
	if(t->garbage) {
		regfree(t->garbage);
		tinyap_free(regex_t, t->garbage);
	}
	delete_node(t->grammar);
	free_stack(t->node_stack);
	free_stack(t->farthest_stack);
	node_cache_flush(t->cache);
	free(t->source);
	free(t);
}


/*
 * basic production rule from regexp : [garbage]token_regexp
 * return NULL on no match
 */

ast_node_t __fastcall token_produce_re(token_context_t*t,const regex_t*expr) {
	regmatch_t token;
	char*lbl;
	int r,c;
	ast_node_t ret=NULL;
	/* perform some preventive garbage filtering */
	/*_filter_garbage(t);*/
	/*update_pos_cache(t);*/
	r=t->pos_cache.row;
	c=t->pos_cache.col;
	if(regexec(expr,t->source+t->ofs,1,&token,0)!=REG_NOMATCH&&token.rm_so==0) {
		assert(token.rm_so==0);
		lbl=match2str(t->source+t->ofs,0,token.rm_eo);
		t->ofs+=token.rm_eo;
//		debug_write("debug-- matched token [%s]\n",lbl);
		update_pos_cache(t);
		ret = newPair(newAtom(lbl,t->pos_cache.row,t->pos_cache.col),NULL,r,c);
		_strfree(lbl);
		//return newAtom(lbl,t->pos_cache.row,t->pos_cache.col);
//	} else {
//		debug_write("debug-- no good token\n");
	}
	return ret;
}


/*
 * basic production rule from regexp+replacement : [garbage]token_regexp
 * return NULL on no match
 */

ast_node_t __fastcall token_produce_rpl(token_context_t*t,const regex_t*expr, const char*rplc) {
	regmatch_t tokens[10];
	char*lbl;
	int r,c;
	ast_node_t ret=NULL;
	/* perform some preventive garbage filtering */
	/*_filter_garbage(t);*/
	/*update_pos_cache(t);*/
	r=t->pos_cache.row;
	c=t->pos_cache.col;
	if(regexec(expr,t->source+t->ofs,10,tokens,0)!=REG_NOMATCH&&(*tokens).rm_so==0) {
		lbl=match2str_rpl(rplc,t->source+t->ofs,10,tokens);
		ret = newPair(newAtom(lbl,t->pos_cache.row,t->pos_cache.col),NULL,r,c);
		/*debug_write("debug-- replaced to %s [%s]\n",rplc,lbl);*/
		_strfree(lbl);
		t->ofs+=(*tokens).rm_eo;
		update_pos_cache(t);
		//return newAtom(lbl,t->pos_cache.row,t->pos_cache.col);
//	} else {
//		debug_write("debug-- no good token\n");
	}
	return ret;
}


ast_node_t __fastcall token_produce_str(token_context_t*t,const char*token) {
	int r,c;
	size_t slen;
	/*_filter_garbage(t);*/
	/*update_pos_cache(t);*/
	r=t->pos_cache.row;
	c=t->pos_cache.col;
	slen=strlen(token);
	if(!strncmp(t->source+t->ofs,token,slen)) {
		t->ofs+=slen;
		update_pos_cache(t);
		return t->flags&STRIP_TERMINALS
			? PRODUCTION_OK_BUT_EMPTY
			: newPair(newAtom(token,t->pos_cache.row,t->pos_cache.col),NULL,r,c);
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



ast_node_t  __fastcall token_produce_any(token_context_t*t,ast_node_t expr,int strip_T);




#define node_tag(_x) Value(Car(_x))
#define node_cdr(_x) Value(Car(Cdr(_x)))



ast_node_t __fastcall find_nterm(const ast_node_t ruleset,const char*ntermid) {
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


ast_node_t __fastcall _produce_seq_rec(token_context_t*t,ast_node_t seq) {
	ast_node_t tmp,rec, _cdr;

	/* if seq is Nil, don't fail */
	if(!seq) {
		return PRODUCTION_OK_BUT_EMPTY;
	}

	/* try and produce first token */
	tmp=token_produce_any(t,getCar(seq),t->flags&STRIP_TERMINALS);

	if(tmp) {
		/* try and produce rest of list */
		/*printf("seq:: start seq %s ; first production is %s\n", tinyap_serialize_to_string(seq), tinyap_serialize_to_string(tmp));*/
		if(tmp!=PRODUCTION_OK_BUT_EMPTY) {
			_cdr = tmp;
			while(Cdr(_cdr)) { _cdr = Cdr(_cdr); }
		} else {
			_cdr = NULL;
		}
		seq = Cdr(seq);

		while(seq&&(rec=token_produce_any(t, Car(seq), t->flags&STRIP_TERMINALS))) {
			if(rec!=PRODUCTION_OK_BUT_EMPTY) {
				update_pos_cache(t);
				if(_cdr) {
					_cdr->pair._cdr = rec;
				} else {
					tmp = rec;
					_cdr = rec;
				}
				while(_cdr->pair._cdr) { _cdr = _cdr->pair._cdr; }
			}
			seq = Cdr(seq);
			/*printf("seq:: now tmp=%p _cdr=%p seq=%p\n", tmp, _cdr, seq);*/
		}

		if(seq) {
			/*printf("seq:: ended with remaining %s ; had produced %s\n", tinyap_serialize_to_string(seq), tinyap_serialize_to_string(tmp));*/
			/*abort();*/
			/*delete_node(tmp);*/
			return NULL;
		} else {
			return tmp;
		}
		
#if 0
		rec=_produce_seq_rec(t,getCdr(seq));
		if(rec) {
			update_pos_cache(t);
			if(isAtom(rec)) {
				assert(!TINYAP_STRCMP(Value(rec),STR_eos));
				(void)(rec?delete_node(rec):0);
				//return newPair(tmp,NULL,t->pos_cache.row,t->pos_cache.col);
				return tmp;
			} else {
				//return newPair(tmp,rec,t->pos_cache.row,t->pos_cache.col);
				return SafeAppend(tmp,rec);
			}
		} else {
			/* FIXME : delay deletions until final cache flush */
			//delete_node(tmp);
			return NULL;
		}
#endif
	}
	return NULL;
}



ast_node_t  __fastcall token_produce_seq(token_context_t*t,ast_node_t seq) {
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



ast_node_t  __fastcall token_produce_alt(token_context_t*t,ast_node_t alt) {
	ast_node_t tmp;
	while(alt&&!(tmp=token_produce_any(t,getCar(alt),t->flags&STRIP_TERMINALS))) {
		alt = getCdr(alt);
	};
	return tmp;
}






/*
 * détection des récursions à gauche simples :
 * si la règle est une alternative
 * et
 * si le premier item (en descendant dans alt et seq) de la règle est (NT règle) 
 * et
 * si la règle est une alternative
 * et
 * s'il y a au moins une alternative non left-réentrante :
 * 	- on teste les autres alternatives d'abord (postpone).
 * 	- sinon :
 *	 	- on teste si le reste de la règle parse au niveau de récurrence 0.
 * 		- si oui, on teste au niveau 1, et ainsi de suite jusqu'à foirer.
 *	- on retourne le dernier résultat non foiré (s'il en est)
 * RESTRICTION :
 * 	la règle doit être de la forme (Alt (Seq (NT règle) ...) (?)) avec (?) ne commençant pas par (NT règle)
 *
 * il faudrait détecter toutes les récurrences, surtout celles pas gérées, avec une recherche dans une pile d'ops.
 */
ast_node_t blacklisted=NULL, replacement=NULL;

ast_node_t __fastcall token_produce_leftrec(token_context_t*t,ast_node_t expr,int strip_T,int isOp) {
	const char*tag = node_tag(Cdr(expr));
	ast_node_t
		tmp = Cdr(Car(Cdr(Cdr(expr)))),
		alt1 = Car(tmp),
		alt2 = Car(Cdr(tmp));

	/*printf("alt1 = %s\nalt2 = %s\n",tinyap_serialize_to_string(alt1),tinyap_serialize_to_string(alt2));*/
	tmp = token_produce_any(t,alt2,strip_T);
	if(tmp&&isOp) {
		tmp=newPair(newPair(newAtom(tag,0,0),tmp,0,0),NULL,0,0);
	}

	if(tmp) {
		blacklisted=Car(Cdr(alt1));
		do {
			replacement = tmp;
			tmp = token_produce_any(t,alt1,strip_T);
			if(tmp&&isOp) {
				tmp=newPair(newPair(newAtom(tag,0,0),tmp,0,0),NULL,0,0);
			}
			/*printf("prout %s\n",tinyap_serialize_to_string(tmp));*/
		} while(tmp);
		tmp = replacement;
		blacklisted=NULL;
	}
	return tmp;
}




int __fastcall check_trivial_left_rec(ast_node_t node) {
	static ast_node_t last=NULL;
	const char*tag=node_tag(Cdr(node));
	ast_node_t alt;

	if(node==last) {
		// don't re-detect, it's being handled
		return 0;
	}
	last=node;

/* 	la règle doit être de la forme (Alt (Seq (NT règle) ...) (?)) avec (?) ne commençant pas par (NT règle) */
	//printf("check lefty %s\n",tinyap_serialize_to_string(node));
	ast_node_t elems=Car(Cdr(Cdr(node)));

	if(isAtom(elems)) {
		return 0;
	}

	//printf("\t%s\n",node_tag(elems));
	if(!TINYAP_STRCMP(node_tag(elems),STR_Alt)) {
		alt=elems;
		elems=Cdr(elems);
		//printf("\t%s\n",node_tag(Car(elems)));
		if(!TINYAP_STRCMP(node_tag(Car(elems)),STR_Seq)) {
			elems=Cdr(Car(elems));
			//printf("\t%s\n",node_tag(Car(elems)));
			if(!TINYAP_STRCMP(node_tag(Car(elems)),STR_NT)) {
				//printf("\t%s %s\n",node_tag(Cdr(Car(elems))),tag);
				if(!TINYAP_STRCMP(node_tag(Cdr(Car(elems))),tag)) {
					/* get second part of alternative */
					elems=Cdr(Cdr(alt));
					if(elems) {
						elems=Cdr(elems);
						//printf("!elems => %i\n",!elems);
						return !elems;/* 0 if more than 2 parts in alternative, 1 if exactly 2 */
					}
				}
			}
		}
	}
	return 0;
}



#define OP_EOF 1
#define OP_RE  2
#define OP_T   3
#define OP_RTR 4
#define OP_ROP 5
#define OP_PREFX 6
#define OP_NT  7
#define OP_SEQ 8
#define OP_ALT 9
#define OP_POSTFX 10
#define OP_RPL 11
#define OP_REP_0N 12
#define OP_REP_01 13
#define OP_REP_1N 14


int max_rec_level = 0;


ast_node_t __fastcall token_produce_any(token_context_t*t,ast_node_t expr,int strip_T) {
//	static int prit=0;
	static int rec_lvl=0;
	char*tag;
	char*key=NULL;
	char*err_tag=NULL;
	ast_node_t ret=NULL, pfx=NULL, tmp=NULL;
	int typ=0;
	int r,c;
	size_t dummy;
	int row,col;
	ast_node_t nt, re;

	if(!expr) {
		return NULL;
	}

	// trivial left-recursion handling
	if(expr==blacklisted) {
		//return newPair(newAtom("strip.me",0,0),NULL,0,0);
		return replacement;
	}

	_filter_garbage(t);
	update_pos_cache(t);

	row = t->pos_cache.row;
	col = t->pos_cache.col;

	if(isAtom(expr)) {
		if(!TINYAP_STRCMP(Value(expr),STR_Epsilon)) {
			/*return newPair(newAtom(STR_strip_me,0,0),NULL,row,col);*/
			return PRODUCTION_OK_BUT_EMPTY;
		} else if(!TINYAP_STRCMP(Value(expr),STR_EOF)) {
			_filter_garbage(t);
			if(t->ofs<t->size) {
				//debug_writeln("EOF not matched at #%u (against #%u)",t->ofs,t->size);
				ret=NULL;
			} else {
				//debug_writeln("EOF matched at #%u (against #%u)",t->ofs,t->size);
				update_pos_cache(t);
				/*ret=newPair(newAtom(STR_strip_me,t->pos_cache.row,t->pos_cache.col),NULL,t->pos_cache.row,t->pos_cache.col);*/
				ret = PRODUCTION_OK_BUT_EMPTY;
			}
			return ret;
		/*} else {*/
			/*printf("%s\n",tinyap_serialize_to_string(expr));*/
		}
	}

	tag=node_tag(expr);

	if(!TINYAP_STRCMP(tag,STR_Seq)) {
		typ = OP_SEQ;
	} else if(!TINYAP_STRCMP(tag,STR_Rep0N)) {
		typ = OP_REP_0N;
	} else if(!TINYAP_STRCMP(tag,STR_Rep1N)) {
		typ = OP_REP_1N;
	} else if(!TINYAP_STRCMP(tag,STR_Rep01)) {
		typ = OP_REP_01;
	} else if(!TINYAP_STRCMP(tag,STR_Alt)) {
		typ = OP_ALT;
	} else if(!TINYAP_STRCMP(tag,STR_RE)) {
		typ = OP_RE;
		err_tag = Value(Car(Cdr(expr)));
	} else if(!TINYAP_STRCMP(tag,STR_RPL)) {
		typ = OP_RPL;
		err_tag = Value(Car(Cdr(expr)));
		key = Value(Car(Cdr(expr)));
	} else if(!TINYAP_STRCMP(tag,STR_T)) {
		typ = OP_T;
		err_tag = Value(Car(Cdr(expr)));
//		key = Value(Car(Cdr(expr)));
	} else if(!TINYAP_STRCMP(tag,STR_NT)) {
		typ = OP_NT;
		key = Value(Car(Cdr(expr)));
	} else if(!TINYAP_STRCMP(tag,STR_Prefix)) {
		typ = OP_PREFX;
	} else if(!TINYAP_STRCMP(tag,STR_Postfix)) {
		typ = OP_POSTFX;
	} else if(!TINYAP_STRCMP(tag,STR_TransientRule)) {
		typ = OP_RTR;
//		key = node_tag(Cdr(expr));
	} else if(!TINYAP_STRCMP(tag,STR_OperatorRule)) {
		typ = OP_ROP;
		err_tag = node_tag(Cdr(expr));
//		key = node_tag(Cdr(expr));
	} else if(!TINYAP_STRCMP(tag,STR_EOF)) {
		typ = OP_EOF;
	} else if(!TINYAP_STRCMP(tag,STR_Epsilon)) {
		/*return newPair(newAtom(STR_strip_me,0,0),NULL,row,col);*/
		return PRODUCTION_OK_BUT_EMPTY;
	}

	/*debug_write("--debug[% 4.4i]-- produce %s ",rec_lvl,tag);*/
	/*dump_node(getCdr(expr));*/
	/*printf("\n");*/
	/*fprintf(stderr,"\tsource = %10.10s%s\n",t->source+t->ofs,t->ofs<(strlen(t->source)-10)?"...":"");*/



//*
	if(key&&node_cache_retrieve(t->cache, row, col, key, &ret,&t->ofs)) {
		/*fprintf(stderr,"found %s at %i:%i %s\n",key,row, col,tinyap_serialize_to_string(ret));*/
		update_pos_cache(t);
		return ret==PRODUCTION_OK_BUT_EMPTY?ret:copy_node(ret);	/* keep PROD.. a singleton */
	}
//*/

	rec_lvl+=1;
	max_rec_level = rec_lvl>max_rec_level?rec_lvl:max_rec_level;

	token_context_push(t,err_tag);

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
//			assert(isAtom(re));
//			printf("prit %i %s\n",prit+=1,Value(re));
			re->raw._p2=token_regcomp(Value(re));
		}
		key = Value(re);
		ret=token_produce_re(t,re->raw._p2);
		break;
	case OP_RPL:
		re = getCar(getCdr(expr));
		if(!re->raw._p2) {
			/* take advantage of unused atom field to implement regexp cache */
			re->raw._p2=token_regcomp(Value(re));
		}
		key = Value(re);
		/*printf("match \"%s\" / replace \"%s\"\n",key,Value(Car(Cdr(Cdr(expr)))));*/
		ret=token_produce_rpl(t,re->raw._p2,Value(getCar(getCdr(getCdr(expr)))));
		break;
	case OP_T:
		ret=token_produce_str(t,Value(getCar(getCdr(expr))));
//		debug_write("### -=< term %s >=- ###\n",ret?"OK":"failed");
		break;
	case OP_ROP:
/*
 * détection des récursions à gauche simples :
 * si la règle est une alternative
 * et
 * si le premier item (en descendant dans alt et seq) de la règle est (NT règle) 
 * et
 * si la règle est une alternative
 * et
 * s'il y a au moins une alternative non left-réentrante :
 * 	- on teste les autres alternatives d'abord (postpone).
 * 	- sinon :
 *	 	- on teste si le reste de la règle parse au niveau de récurrence 0.
 * 		- si oui, on teste au niveau 1, et ainsi de suite jusqu'à foirer.
 *	- on retourne le dernier résultat non foiré (s'il en est)
 * RESTRICTION :
 * 	la règle doit être de la forme (Alt (Seq (NT règle) ...) (?)) avec (?) ne commençant pas par (NT règle)
 *
 * il faudrait détecter toutes les récurrences, surtout celles pas gérées, avec une recherche dans une pile d'ops.
 */
		if(check_trivial_left_rec(expr)) {
			ret = token_produce_leftrec(t,expr,t->flags,1);
			tag=node_tag(Cdr(expr));
		} else {
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
		}
		break;
	case OP_RTR:
		if(check_trivial_left_rec(expr)) {
			ret = token_produce_leftrec(t,expr,t->flags,0);
		} else {
			expr=getCdr(expr);	/* shift the operator tag */
	//		dump_node(expr);
			tag=node_tag(expr);

			ret=token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
		}
		break;
	case OP_REP_01:
		pfx = token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
		if(pfx!=NULL) {
			ret = pfx;
		} else {
			/*ret = newPair(newAtom(STR_strip_me,0,0), NULL,t->pos_cache.row,t->pos_cache.col);*/
			ret = PRODUCTION_OK_BUT_EMPTY;
		}
		break;
	case OP_REP_1N:
		pfx = token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
		if(pfx!=NULL&&pfx!=PRODUCTION_OK_BUT_EMPTY) {
			unsigned long last_ofs = t->ofs;
			/*char*stmp = (char*) tinyap_serialize_to_string(expr);*/
			/*char*stmp2 = (char*)tinyap_serialize_to_string(pfx);*/
			update_pos_cache(t);
			/*printf("got prefix for rep 1,N for expr %s at %i,%i : %s\n",stmp,t->pos_cache.row,t->pos_cache.col,stmp2);*/
			/*free(stmp);*/
			/*free(stmp2);*/
			ret = pfx;
			while( (tmp = token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS))
						&&
					tmp != PRODUCTION_OK_BUT_EMPTY
						&&
					last_ofs!=t->ofs ) {
				last_ofs=t->ofs;
				/*stmp = (char*) tinyap_serialize_to_string(tmp);*/
				/*printf("    continue for rep 1,N at %i,%i : %s\n",t->pos_cache.row,t->pos_cache.col,stmp);*/
				/*free(stmp);*/
				while(pfx->pair._cdr) {
					pfx=pfx->pair._cdr;
				}
				pfx->pair._cdr = tmp;
				pfx = tmp;
			}
		}
		break;
	case OP_REP_0N:
		pfx = token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
		if(pfx!=NULL&&pfx!=PRODUCTION_OK_BUT_EMPTY) {
			unsigned long last_ofs = t->ofs;
			ret = pfx;
			while( (tmp = token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS))
						&&
					tmp != PRODUCTION_OK_BUT_EMPTY
						&&
					last_ofs!=t->ofs  ) {
				last_ofs=t->ofs;
				while(pfx->pair._cdr) {
					pfx=pfx->pair._cdr;
				}
				pfx->pair._cdr = tmp;
				pfx = tmp;
			}
		} else {
			ret = PRODUCTION_OK_BUT_EMPTY;
		}
		break;
	case OP_PREFX:
		expr=getCdr(expr);	/* shift the operator tag */
//		dump_node(expr);
		//tag=node_tag(expr);

		pfx=token_produce_any(t,getCar(expr),t->flags&STRIP_TERMINALS);
		if(pfx!=NULL) {
			/*printf("have prefix %s\n",tinyap_serialize_to_string(pfx));*/
			ret=token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
			if(ret==PRODUCTION_OK_BUT_EMPTY) {
				return pfx;
			}
			if(pfx==PRODUCTION_OK_BUT_EMPTY) {
				return ret;
			}
			if(ret&&ret->pair._car) {
				ast_node_t tail;
				/* these copies are necessary because of structural hack.
				 * Not copying botches the node cache.
				 */
				ret=copy_node(ret);
				pfx=copy_node(pfx);

				tail=pfx;
				/*printf("have expr %s\n",tinyap_serialize_to_string(ret));*/
				//ret->pair._car->pair._cdr = Append(pfx,ret->pair._car->pair._cdr);
				/* FIXME ? Dirty hack. */
				while(tail->pair._cdr) {
					tail = tail->pair._cdr;
				}
				tail->pair._cdr = ret->pair._car->pair._cdr;
				ret->pair._car->pair._cdr = pfx;
				/*printf("\nhave merged into %s\n\n",tinyap_serialize_to_string(ret));*/
			}
		}
		break;
	case OP_POSTFX:
		expr=getCdr(expr);	/* shift the operator tag */
//		dump_node(expr);
		//tag=node_tag(expr);

		pfx=token_produce_any(t,getCar(expr),t->flags&STRIP_TERMINALS);
		if(pfx!=NULL) {
			//printf("have postfix %s\n",tinyap_serialize_to_string(pfx));
			ret=token_produce_any(t,getCar(getCdr(expr)),t->flags&STRIP_TERMINALS);
			if(ret&&ret->pair._car) {
				//printf("have expr %s\n",tinyap_serialize_to_string(ret));
				//ret->pair._car->pair._cdr = Append(pfx,ret->pair._car->pair._cdr);
				ret->pair._car = SafeAppend(ret->pair._car,pfx);

				//printf("have merged into %s\n",tinyap_serialize_to_string(ret));
			}
		}
		break;
	case OP_NT:
		tag = Value(getCar(getCdr(expr)));
		if(!(TINYAP_STRCMP(tag, STR_Space) && TINYAP_STRCMP(tag, STR_NewLine) && TINYAP_STRCMP(tag, STR_Indent) && TINYAP_STRCMP(tag, STR_Dedent))) {
			/*ret=newPair(newAtom(STR_strip_me,t->pos_cache.row,t->pos_cache.col),NULL,t->pos_cache.row,t->pos_cache.col);*/
			ret = PRODUCTION_OK_BUT_EMPTY;
		} else {
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
			/*ret=newPair(newAtom(STR_strip_me,t->pos_cache.row,t->pos_cache.col),NULL,t->pos_cache.row,t->pos_cache.col);*/
			ret = PRODUCTION_OK_BUT_EMPTY;
		}
		break;
	};

	rec_lvl-=1;
	if(ret) {
		/* add to node cache */
		if(key) {
			/*fprintf(stderr,"add to cache [ %i:%i:%s ] %s\n", row, col, key, tinyap_serialize_to_string(ret));*/
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


//	if(!TINYAP_STRCMP(tag,"Seq")) {
//	/* case seq : try and produce every subexpr, or fail. return whole cons'd list */
//		ret=token_produce_seq(t,getCdr(expr));
////		debug_write("### -=< seq %s >=- ###\n",ret?"OK":"failed");
////		if(ret) {
//			//debug_write("Produce Seq ");
//			//dump_node(expr);
//			//dump_node(ret);
////		}
//
//	} else if(!TINYAP_STRCMP(tag,"Alt")) {
//	/* case alt : try and produce each subexpr, return the first whole production or fail */
//		ret=token_produce_alt(t,getCdr(expr));
////		debug_write("### -=< alt %s >=- ###\n",ret?"OK":"failed");
////		if(ret) {
////			debug_write("Produce Alt ");
////			dump_node(expr);
////			dump_node(ret);
////		}
//
//	} else if(!TINYAP_STRCMP(tag,"RE")) {
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
//	} else if(!TINYAP_STRCMP(tag,"T")) {
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
//	} else if(!TINYAP_STRCMP(tag,"OperatorRule")) {
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
//	} else if(!TINYAP_STRCMP(tag,"TransientRule")) {
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
//	} else if(!TINYAP_STRCMP(tag,"NT")) {
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
//	} else if(!TINYAP_STRCMP(tag,"EOF")) {
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





ast_node_t __fastcall clean_ast(ast_node_t t) {
	if(!t) {
		return NULL;
	}
	if(isAtom(t)) {
		if(TINYAP_STRCMP(Value(t),STR_strip_me)) {
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



void __fastcall update_pos_cache(token_context_t*t) {
	int ln=t->pos_cache.row;		/* line number */
	size_t ofs=t->pos_cache.last_ofs;
	size_t end=t->ofs;
	size_t last_nlofs=t->pos_cache.last_nlofs;

	if(t->ofs<t->pos_cache.last_ofs) {
		while(ofs>end) {
			if(t->source[ofs]=='\n') {
				--ln;
			}
			--ofs;
		}
		while(ofs>0&&t->source[ofs]!='\n') {
			--ofs;
		}
		last_nlofs=ofs+(!!ofs);	/* don't skip character if at start of buffer */
	} else {
		while(ofs<end) {
			if(t->source[ofs]=='\n') {
				++ln;
				last_nlofs=ofs+1;
			}
			++ofs;
		}

	}

	if(ln>t->pos_cache.row) {
		t->pos_cache.row=ln;
		t->pos_cache.col=1+end-last_nlofs;
		t->pos_cache.last_ofs=end;
		t->pos_cache.last_nlofs=last_nlofs;
		/*node_cache_clean(t->cache, &t->pos_cache);*/
	}
	t->pos_cache.row=ln;
	t->pos_cache.col=1+end-last_nlofs;
	t->pos_cache.last_ofs=end;
	t->pos_cache.last_nlofs=last_nlofs;
}


const char* parse_error(token_context_t*t) {
	static char err_buf[4096];
	size_t last_nlofs=0;
	size_t next_nlofs=0;
	size_t tab_adjust=0;
	int i;
	char*sep,*k;

	t->ofs=t->farthest;
	update_pos_cache(t);
	last_nlofs=t->ofs-t->pos_cache.col+1;
	
	next_nlofs=last_nlofs;
	while(t->source[next_nlofs]&&t->source[next_nlofs]!='\n') {
		if(t->source[next_nlofs]=='\t') {
			tab_adjust+=8-((next_nlofs-last_nlofs)&7);	/* snap to tabsize 8 */
		}
		next_nlofs+=1;
	}

	err_buf[0]=0;

	if((long)t->farthest_stack->sp>=0) {
		sep = " In context ";

		for(i=0;i<=(long)t->farthest_stack->sp;i+=1) {
			k=(char*)t->farthest_stack->stack[i];
			if(k) {
				strcat(err_buf,sep);
				strcat(err_buf,k);
				sep=".";
			}
		}

		strcat(err_buf,",\n");
	}
	
//	sprintf(err_buf,"parse error at line %i :\n%*.*s\n%*.*s^\n",
	sprintf(err_buf+strlen(err_buf),"%*.*s\n%*.*s^\n",
		(int)(next_nlofs-last_nlofs),
		(int)(next_nlofs-last_nlofs),
		t->source+last_nlofs,
		(int)(t->farthest-last_nlofs+tab_adjust-1),
		(int)(t->farthest-last_nlofs+tab_adjust-1),
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

