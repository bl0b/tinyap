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
#include "string_registry.h"
#include "serialize.h"

#define _BIN_SZ 4096

static char* _buf=NULL;
static tinyap_stack_t _buf_st=NULL;
static size_t _buf_sz=0;
static size_t _buf_res=0;

static char
	* _buf_space = " ",
	* _buf_newline = "\n",
	* _buf_indent = "\t";

size_t _buf_indent_lvl = 0;

extern int unrepl_context;

#ifndef HAVE_STRNDUP

static char* strndup(const char* src, size_t n) {
	size_t slen = strlen(src);
	size_t max = slen > n ? slen : n;
	char* ret = (char*) malloc(max+1);
	memcpy(ret, src, max);
	ret[max] = 0;
	return ret;
}

#endif

static int _buf_endswith(char* str) {
	size_t slen = strlen(str);
	if(slen>_buf_sz) {
		return 0;
	}
	return !strcmp(str, _buf+_buf_sz-slen);
}

static void _buf_append(const char* s);

static void _buf_append_space() {
	if(!_buf_endswith(_buf_space)) {
		_buf_append(_buf_space);
	}
}

static void _buf_append_newline() {
	int i;
	_buf_append(_buf_newline);
	for(i=0;i<_buf_indent_lvl;i+=1) {
		_buf_append(_buf_indent);
	}
}

static void _buf_do_indent() {
	_buf_indent_lvl += 1;
	_buf_append(_buf_indent);
}

static void _buf_do_dedent() {
	_buf_indent_lvl -= 1;
	if(_buf_endswith(_buf_indent)) {
		_buf_sz -= strlen(_buf_indent);
	}
}

static void _buf_deinit() {
	if(_buf) {
		free(_buf);
	}
	if(_buf_st) {
		free_stack(_buf_st);
	}
}

static void _buf_init() {
	_buf_deinit();
	_buf_st = new_stack();
	_buf_res = _BIN_SZ;
	_buf_sz = 0;
	_buf = (char*) malloc(_buf_res);
	memset(_buf, 0, _buf_res);
}

static void _buf_realloc() {
	char /* * old_buf = _buf,*/ * new_buf;
	while((_buf_sz+1) >= _buf_res) {
		_buf_res += _BIN_SZ;
	}
	new_buf = (char*) realloc(_buf, _buf_res);
	/*if(new_buf != old_buf) {*/
		/*strcpy(new_buf, old_buf);*/
		_buf = new_buf;
	/*}*/
}

static void _buf_backup() {
	push(_buf_st, (void*)_buf_sz);
}

static void _buf_validate() {
	_pop(_buf_st);
}

static void _buf_restore() {
	_buf_sz = (size_t) _pop(_buf_st);
}

static void _buf_append(const char* s) {
	size_t slen = strlen(s);
	size_t bsz  = _buf_sz;
	_buf_sz += slen;
	if( (_buf_sz+1) >= _buf_res) {
		_buf_realloc();
	}
	strcpy(_buf+bsz, s);
}

static void _buf_append_chr(char c) {
	size_t bsz  = _buf_sz;
	_buf_sz += 1;
	if( (_buf_sz+1) >= _buf_res) {
		_buf_realloc();
	}
	*(_buf+bsz)=c;
}


const char* wi_op(wast_iterator_t wi) {
	return wa_op(tinyap_wi_node(wi));
}

const char* wi_string(wast_iterator_t wi, size_t n) {
	const char* ret = wa_op(wa_opd(tinyap_wi_node(wi), n));
	if(!ret) {
		ret = "";
	}
	return ret;
}


wast_iterator_t wig_goto_rule(wast_iterator_t grammar, char* name) {
	int match;
	wi_reset(grammar);
	wi_down(grammar);
	/*printf("rule name match ? %s / %s (%s)\n", wi_string(grammar, 0), name, wi_has_next(grammar)?"has next":"is last");*/
	match=!strcmp(wi_string(grammar, 0), name);
	while(match==0 && tinyap_wi_has_next(grammar)) {
		tinyap_wi_next(grammar);
		if(strcmp(wi_op(grammar), STR_Comment)) {
			/*printf("rule name match ? %s / %s (%s)\n", wi_string(grammar, 0), name, wi_has_next(grammar)?"has next":"is last");*/
			match=!strcmp(wi_string(grammar, 0), name);
		}
	}
	if(match) {
		wast_iterator_t ret = wi_dup(grammar);
		/*printf("found rule '%s' at %p %s %s\n", name, wi_node(grammar), wi_op(grammar), wi_string(grammar, 0));*/
		/*tinyap_wi_down(ig);*/
		return ret;
	} else {
		printf("didn't find rule '%s'\n", name);
	}
	return NULL;
}


/* Behaviour :
 * start : unproduce(grammar, rule, ast) or fail with rule="_start"
 * unproduce : 
 * - if rule is transient :
 *   	enter = ok
 * - if rule is operator :
 *   	enter = (ast op == rule name)
 *
 *   		
 *   		
 *   	
 *
 */
#define __brv(_op) do { /*printf("DEBUG : " #_op "\n");*/ _buf_##_op(); wi_##_op(expr); wi_##_op(ast); } while(0)

#define BACKUP __brv(backup)
#define RESTORE do { /*printf("BUF WAS : %s\n", _buf);*/ __brv(restore); /*printf("BUF RESTORED TO : %s\n", _buf);*/ } while(0)
#define VALIDATE do { __brv(validate); /*printf("CURRENT BUF : %s\n", _buf);*/ } while(0)

int unproduce(wast_iterator_t grammar, wast_iterator_t expr, wast_iterator_t ast);

int wa_check_lefty(wast_t rule) {
	wast_t expr = wa_opd(rule, 1);
	if(!strcmp(wa_op(expr), "Alt")) {
		wast_t seq = wa_opd(expr, 0);
		if(!(strcmp(wa_op(seq), "Seq")&&strcmp(wa_op(seq), "RawSeq"))) {
			wast_t nt = wa_opd(seq, 0);
			if(!strcmp(wa_op(nt), "NT")) {
				return !strcmp(wa_op(wa_opd(nt, 0)), wa_op(wa_opd(rule, 0)));
			}
		}
	}
	return 0;
}

wast_t wa_dump(wast_t wa) {
	ast_node_t a = make_ast(wa);
	const char* str = tinyap_serialize_to_string(a);
	fputs(str, stdout);
	free((char*)str);
	/*free(a);*/
	delete_node(a);
	return wa;
}

wast_t wa_bl_expr = NULL;
wast_t wa_bl_ast = NULL;

int unproduce_rule(wast_iterator_t grammar, wast_iterator_t expr, wast_iterator_t ast) {
	int status;
	/*int next=0;*/
	/*if(wi_node(ast)==NULL) {*/
		/*return 0;*/
	/*}*/
	/*printf("ON rule %s\n", wi_op(expr));*/
	BACKUP;

	if(!strcmp(wi_op(expr), "TransientRule")) {
		if(wa_check_lefty(wi_node(expr))) {
			/*printf("entering transient L-rec rule (%s).\n", wi_string(expr, 0));*/
			wast_t alt = wa_opd(wi_node(expr), 1);
			wast_t alt1 = wa_opd(alt, 0);
			wast_t alt2 = wa_opd(alt, 1);
			wast_iterator_t wi1, wi2;
			wi2 = wi_new(alt2);
			/*printf(" ## AST now : "); wa_dump(wi_node(ast)); printf("\n");*/
			status = unproduce(grammar, wi2, ast);
			wi_delete(wi2);
			if(status) {
				/*printf("unproduced tail OK.\n");*/
				/*printf(" ## AST now : "); wa_dump(wi_node(ast)); printf("\n");*/
				wa_bl_expr = wa_opd(alt1, 0);
				wi1 = wi_new(alt1);
				wi_backup(wi1);
				while(wi_node(ast)&&unproduce(grammar, wi1, ast)) {
					/*printf(" ## AST now : "); wa_dump(wi_node(ast)); printf("\n");*/
					wi_restore(wi1);
					wi_backup(wi1);
				}
				wi_delete(wi1);
				wa_bl_expr = NULL;
			}
		} else {
			wi_down(expr);
			wi_next(expr);
			status = unproduce(grammar, expr, ast);
		}
	} else if(!strcmp(wi_op(expr), "OperatorRule")) {
		/* Needn't check for L-recs when unproducing thru an operator rule !
		 * the AST itself prevents infinite recursion
		 */
		status = wi_node(ast) && !strcmp(wi_string(expr, 0), wi_op(ast));
		if(status) {
			/*printf("operator match (%s).\n", wi_op(ast));*/
			wi_down(ast);
			wi_down(expr);
			wi_next(expr);
			status = unproduce(grammar, expr, ast);
			wi_up(ast);
			if(status) {
				wi_next(ast);
			}
		/*} else {*/
			/*printf("operator mismatch (%s, %s).\n", wi_string(expr,0), wi_op(ast));*/
		}
	} else {
		fprintf(stderr, "Not supposed to handle '%s' rule type.\n", wi_op(expr));
		/*status = 1/0;*/
		status = 0;
	}
	if(status) {
		VALIDATE;
	} else {
		RESTORE;
	}
	return status;
}


int _str_escape_hlpr(int c, void* context) {
	char** ptr = (char**) context;
	**ptr = (char) c;
	*ptr += 1;
	return 0;
}

/*void escape_chr(char**src,int(*func)(int,void*),void*param, int context);*/

char* str_escape(char* str) {
	static char buffy[4096];
	char* ptr = buffy;
	/*printf("str_escape(%s, %i)\n", str, unrepl_context);*/
	while(*str) {
		escape_chr(&str, _str_escape_hlpr, (void*)&ptr, unrepl_context);
	}
	_str_escape_hlpr(0, &ptr);
	return buffy;
}



char* unrepl(const char* re, const char* repl, const char* token);

#define _RE   2
#define _T    4

int unproduce(wast_iterator_t grammar, wast_iterator_t expr, wast_iterator_t ast/*, int* next*/) {
	static int _rec = 0;
	int status=0;
	int next=0;
	/*ast_node_t a;*/
	/**next = 0;*/
	/*printf("\n[%i] - - unproduce - - expr = ", _rec); wa_dump(wi_node(expr)); printf(" ast = "); wa_dump(wi_node(ast)); printf("\n");*/
	if(wi_node(expr)==NULL) {
		return 0;
	}
	if(wi_node(ast) == wa_bl_ast) {
		wi_next(ast);
	}
	if(wi_node(expr)==wa_bl_expr) {
		return 1;
	}
	if(!strcmp(wi_op(expr), "T")) {
		_buf_append(wi_string(expr,0));
		return 1;
	}
	BACKUP;
	_rec += 1;
	if(!strcmp(wi_op(expr),	"Rep0N")) {
		/*printf("Rep0N\n");*/
		/*_rec += 1;*/
		wi_down(expr);
		while(unproduce(grammar, expr, ast));
		wi_up(expr);
		/*_rec -= 1;*/
		/*wi_next(expr);*/
		status = 1;
	} else if(!strcmp(wi_op(expr),	"Rep01")) {
		wi_down(expr);
		unproduce(grammar, expr, ast);
		wi_up(expr);
		status = 1;
	} else if(!(strcmp(wi_op(expr),	"Seq")&&strcmp(wi_op(expr), "RawSeq"))) {
		wi_down(expr);
		while((status=unproduce(grammar, expr, ast)) && wi_has_next(expr)) {
			wi_next(expr);
			/*printf("seq now on %p (%s)\n", wi_node(expr), wi_node(expr)?wi_op(expr):"null");*/
		}
		/*printf("[%i] after seq : %p %i\n", _rec, wi_node(expr), wi_has_next(expr));*/
		/*if(!status) {*/
			/*printf("[%i] seq failed on ", _rec); wa_dump(wi_node(expr)); printf("\n");*/
		/*}*/
		/*status &= !wi_has_next(expr);*/
		wi_up(expr);
	} else if(!strcmp(wi_op(expr),	"Alt")) {
		wi_down(expr);
		while( (!(status = unproduce(grammar, expr, ast))) && wi_has_next(expr) ) {
			wi_next(expr);
		}
		/*printf("[%i] after alt : status=%i\n", _rec, status);*/
		wi_up(expr);
	} else if(!strcmp(wi_op(expr),	"Epsilon")) {
		/*printf("[%i] produced epsilon.\n", _rec);*/
		status = 1;
	} else if(!strcmp(wi_op(expr),	"EOF")) {
		/*printf("on EOF.... ast = ");*/
		/*wa_dump(wi_node(ast));*/
		/*printf("\n");*/
		status = !wi_node(ast);
	} else if(!strcmp(wi_op(expr),	"NT")) {
		/* first check for special rules */
		if(!strcmp(wi_string(expr, 0), "Space")) {
			_buf_append_space();
			status = 1;
		} else if(!strcmp(wi_string(expr, 0), "NewLine")) {
			_buf_append_newline();
			status = 1;
		} else if(!strcmp(wi_string(expr, 0), "Indent")) {
			_buf_do_indent();
			status = 1;
		} else if(!strcmp(wi_string(expr, 0), "Dedent")) {
			_buf_do_dedent();
			status = 1;
		} else {
			/*int backup = unrepl_context;*/
			/*if(!strcmp(wi_string(expr, 0), "T")) {*/
				/*unrepl_context = _T;*/
			/*} else if(!(strcmp(wi_string(expr, 0), "RE")&&strcmp(wi_string(expr, 0), "RPL"))) {*/
				/*unrepl_context = _RE;*/
			/*}*/
			wast_iterator_t nt_expr = wig_goto_rule(grammar, (char*) wi_string(expr, 0));
			status = unproduce_rule(grammar, nt_expr, ast);
			wi_delete(nt_expr);
			/*unrepl_context = backup;*/
		}
	} else if(wi_node(ast)!=NULL) {
		if(!strcmp(wi_op(expr),	"RE")) {
			if(wi_on_leaf(ast)) {
				_buf_append(str_escape((char*)wi_op(ast)));
				status = 1;
				next = 1;
			}
		} else if(!strcmp(wi_op(expr),	"STR")) {
			if(wi_on_leaf(ast)) {
				const char* tmp = wi_op(ast);
				const char* end = wi_string(expr, 1);
				_buf_append(wi_string(expr, 0));
				while(*tmp) {
					if(*tmp==*end) {
						_buf_append_chr('\\');
					}
					switch(*tmp) {
					case '\n' : _buf_append("\\n"); break;
					case '\r' : _buf_append("\\r"); break;
					case '\t' : _buf_append("\\t"); break;
					default: _buf_append_chr(*tmp);
					};
					tmp+=1;
				}
				_buf_append(end);
				status = 1;
				next = 1;
			}
		} else if(!strcmp(wi_op(expr),	"RPL")) {
			/* FIXME : can't undo a replacement from regexp match */
			if(wi_on_leaf(ast)) {
				char* unrep = unrepl( wi_string(expr, 0), wi_string(expr, 1), wi_op(ast));
				/*_buf_append(str_escape(unrep));*/
				_buf_append(unrep);
				status = 1;
				next = 1;
			}
		} else if(!strcmp(wi_op(expr),	"Prefix")) {
			wast_t backup;
			/* try to unproduce first operand in prefix and first operand in ast */
			status = unproduce(grammar, wi_down(expr), wi_down(ast));
			wi_up(ast);
			if(status) {
				backup = wa_bl_ast;
				/* blacklist wa_opd(wi_node(ast),0) */
				wa_bl_ast = wa_opd(wi_node(ast), 0);
				/* then try to unproduce second operand in prefix and ast */
				status = unproduce(grammar, wi_next(expr), ast);
				/* whitelist wa_opd... */
				wa_bl_ast = backup;
			}
			wi_up(expr);
		} else if(!strcmp(wi_op(expr),	"Postfix")) {
			/* try to unproduce first operand in prefix and first operand in ast */
			wast_t tmp, backup;
			wi_backup(ast);
			wi_down(ast);
			while(wi_has_next(ast)) { wi_next(ast); }
			tmp = wi_node(ast);
			status = unproduce(grammar, wi_down(expr), ast);
			backup = wa_bl_ast;
			wa_bl_ast = tmp;
			/*printf("postfix : blacklist "); wa_dump(wa_bl_ast); printf("\n");*/
			wi_restore(ast);
			if(status) {
				/* blacklist wa_opd(wi_node(ast),0) */
				/* then try to unproduce second operand in prefix and ast */
				status = unproduce(grammar, wi_next(expr), ast);
				/* whitelist wa_opd... */
			}
			wa_bl_ast = backup;
			wi_up(expr);
			/* TODO */
			/*printf("TODO : Postfix\n");*/
			/*status = 0;*/
		} else if(!strcmp(wi_op(expr),	"Rep1N")) {
			status = unproduce(grammar, wi_down(expr), ast);
			/*printf("Rep1N status = %i\n", status);*/
			if(status) {
				while(unproduce(grammar, expr, ast));
			}
			wi_up(expr);
		}
	}
	if(status) {
		VALIDATE;
		/*printf("SUCCESS [%i]\n", _rec);*/
		/*printf("---------- buffer so far ----------\n");*/
		/*printf("%s\n", _buf);*/
		/*printf("-----------------------------------\n");*/
		if(next) {
			wi_next(ast);
		}
	} else {
		RESTORE;
		/*printf("FAILURE [%i]\n", _rec);*/
	}
	_rec -= 1;
	return status;
}



const char* tinyap_unparse(wast_t grammar, wast_t ast) {
	char* ret;
	wast_iterator_t
		igrammar = tinyap_wi_new(grammar),
		ig,
		ia = tinyap_wi_new(ast);
	_buf_init();
	ig = wig_goto_rule(igrammar, STR__start);
	/*wa_dump(ast);*/
	/*printf("\n");*/
	if(unproduce_rule(igrammar, ig, ia)) {
		ret = strndup(_buf, _buf_sz);
	} else {
		ret = NULL;
	}
	tinyap_wi_delete(ig);
	tinyap_wi_delete(igrammar);
	tinyap_wi_delete(ia);
	_buf_deinit();
	return ret;
}

