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
#ifndef _TINYAP_AST_H__
#define _TINYAP_AST_H__

#ifdef __cplusplus
#include <iostream>
extern "C" {
#endif

#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bootstrap.h"

extern volatile int depth;

#define debug_enter() do { depth+=3; } while(0)
#define debug_leave() do { depth-=3; } while(0)
#define debug_indent() do { fprintf(stdout,"%*.*s",depth,depth,""); } while(0)
#define debug_write(_fmt ,arg...) do { debug_indent(); fprintf(stdout,_fmt ,##arg ); fflush(stdout); } while(0)
#define debug_writeln(_fmt ,arg...) do { debug_indent(); fprintf(stdout,_fmt ,##arg ); fputc('\n',stdout); fflush(stdout); } while(0)


/*! \brief AST Node Types
 */
typedef enum { ast_Nil=0, ast_Pool=42, ast_Atom=0x6106, ast_Pair=0x8108 } ast_type_t;


union _ast_node_t {
	/* first union member is used for static initializations */
	struct _ant_sz {
		ast_type_t type;
        int offset;
		void* p1;
		void* p2;
        void* _;
	} raw;
	void*__align[4];
	/* direct access to type field */
	ast_type_t type;
	/* position in source text */
	struct {
		ast_type_t _res_type;
		int offset;
	} pos;
	/* an atom : type_Atom + token */
	struct {
		ast_type_t _res_type;
		int offset;
		char*_str;
	} atom;
	/* a pair : type_Pair + car_ptr + cdr_ptr */
	struct {
		ast_type_t _res_type;
		int _res_offset;
		union _ast_node_t* _car;
		union _ast_node_t* _cdr;
	} pair;
};



#if 0
// OLD IMPL
union _ast_node_t {
	/* first union member is used for static initializations */
	struct _ant_sz {
		ast_type_t _;
		void*_p1;
		void*_p2;
		unsigned int _offset;
		int _flags;
		int ref;
	} raw;
	void*__align[8];
	struct {
		ast_type_t _;
		char __reserve[sizeof(struct _ant_sz)-sizeof(ast_type_t)-sizeof(ast_node_t)];
		ast_node_t next;
	} pool;
	/* position in source text */
	struct {
		ast_type_t _res_at;
		void*__res[2];
		unsigned int offset;
	} pos;
	/* direct access to type field */
	ast_type_t type;
	/* an atom : type_Atom + token */
	struct {
		ast_type_t _;
		char*_str;
	} atom;
	/* a pair : type_Pair + car_ptr + cdr_ptr */
	struct {
		ast_type_t _;
		union _ast_node_t* _car;
		union _ast_node_t* _cdr;
	} pair;
};
#endif
#define node_flags raw._flags
#define RULE_IS_LEFTREC 1
#define RULE_IS_LEFTREC_COMPUTED 2
#define ATOM_IS_NOT_STRING 4
#define IS_FOREST	0x8000


#define _atom(__s,_r,_c, _f) (union _ast_node_t[]){{{ast_Atom,__s,NULL,_r,_c, _f}}}
#define _pair(__a,__d,_r,_c, _f) (union _ast_node_t[]){{{ast_Pair,__a,__d,_r,_c, _f}}}


/*! \par __n an AST node
 *  \return the corresponding character string if __n is an atom
 */
#define getAtom(__n) (ast_type_check((__n),ast_Atom,__FILE__,__LINE__)->atom._str)

/*! \par __n an AST node
 *  \return car(__n) if __n is a pair
 */
#define getCar(__n) (ast_type_check((__n),ast_Pair,__FILE__,__LINE__)->pair._car)

/*! \par __n an AST node
 *  \return cdr(__n) if __n is a pair
 */
#define getCdr(__n) (ast_type_check((__n),ast_Pair,__FILE__,__LINE__)->pair._cdr)

static inline int getOffset(ast_node_t n) {
	if(!n) return 0;
	return n->pos.offset;
}

/*static inline int getCol(ast_node_t n) {*/
	/*if(!n) return 0;*/
	/*return n->pos.col;*/
/*}*/
/**/
/*static inline void setRow(ast_node_t n,int r) {*/
	/*if(!n) return;*/
	/*n->pos.row=r;*/
/*}*/
/**/
/*static inline void setCol(ast_node_t n,int c) {*/
	/*if(!n) return;*/
	/*n->pos.col=c;*/
/*}*/
/**/

#define isAtom(__n) ((__n)&&(__n)->type==ast_Atom)
#define isPair(__n) ((__n)&&(__n)->type==ast_Pair)
#define isNil(__n) ((__n)==NULL)


#define Atom	_atom
#define Cons	_pair
#define Car	getCar
#define Cdr	getCdr
#define Value	getAtom

#define Cddr(_x) Cdr(Cdr(_x))
#define Cdddr(_x) Cdr(Cddr(_x))
#define Cddddr(_x) Cdr(Cdddr(_x))

#define Cadr(_x) Car(Cdr(_x))
#define Caddr(_x) Car(Cddr(_x))
#define Cadddr(_x) Car(Cdddr(_x))
#define Caddddr(_x) Car(Cddddr(_x))

extern volatile int _node_alloc_count;

#define _ast_node_type_to_str(__t) (__t==ast_Atom?"Atom":__t==ast_Pair?"Pair":__t?"(unknown)":"#nil")

int dump_node(const ast_node_t n);

#ifdef TINYAP_SAFEMODE
static inline ast_node_t _atc_fail(const ast_node_t n,const ast_type_t expected,const char*_f,const int _l) {
	debug_writeln("%s:%d: wrong node type %s [%X] : expected %s ; \n",
		_f, _l,
		n?_ast_node_type_to_str(n->type):"(nil)", n?n->type:0,_ast_node_type_to_str(expected));
	if(n) dump_node(n);
	/* die */
	abort();
	//exit(-1);
	return NULL;
}

static inline ast_node_t ast_type_check(const ast_node_t n,const ast_type_t expected,const char*_f,const int _l) {
	return n	? n->type==expected
					? n
					: _atc_fail(n, expected, _f, _l)
				: NULL;
	/*if((!n)&&expected==ast_Nil) {*/
		/*return NULL;*/
	/*} else if((!n)||n->type!=expected) {*/
		/* spawn error */
		/*_atc_fail(n, expected, _f, _l);*/
	/*}*/
	/*return (ast_node_t )n;*/
}
#else
#	define ast_type_check(_n, _e, _f, _l) (_n)
#endif

ast_node_t ref(ast_node_t);
void unref(ast_node_t);
size_t ref_count(ast_node_t);

ast_node_t newAtom_debug(const char*data, int offset, const char*F, size_t L);
ast_node_t newPair_debug(const ast_node_t a,const ast_node_t d, const char*F, size_t L);
ast_node_t newAtom_impl(const char*data, int offset);
ast_node_t newPair_impl(const ast_node_t a,const ast_node_t d);
#ifdef DEBUG_ALLOCS
#define newAtom(_d, _o) newAtom_debug(_d, _o, __FILE__, __LINE__)
#define newPair(_a, _d) newPair_debug(_a, _d, __FILE__, __LINE__)
#else
#define newAtom(_d, _o) newAtom_impl(_d, _o)
#define newPair(_a, _d) newPair_impl(_a, _d)
#endif
void delete_node(ast_node_t n);

void print_pair(ast_node_t n);


extern ast_node_t PRODUCTION_OK_BUT_EMPTY;

const char* ast_serialize_to_string(const ast_node_t ast, int show_offset);

void dump_ast_reg();
#ifdef __cplusplus
}

static inline std::ostream& operator<<(std::ostream&os, const ast_node_t n) {
	const char* tmp = ast_serialize_to_string(n, 1);
	os << tmp;
	free((char*)tmp);
	return os;
};

struct Ast {
	Ast(const ast_node_t _)
		: n(ref(_))
	{}
	Ast(const Ast& a)
		: n(ref(a))
	{}
	Ast() : n(0) {}
	~Ast() { unref(n); }
	Ast& operator=(const ast_node_t x) {
        if (x == n) { return *this; }
		unref(n);
		n=x;
        ref(n);
		return *this;
	}
	operator ast_node_t() const { return n; }
	ast_node_t operator->() const { return n; }
private:
	ast_node_t n;
};

#endif

#endif


