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

#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bootstrap.h"

extern volatile int depth;

#define debug_enter() do { depth+=3; } while(0)
#define debug_leave() do { depth-=3; } while(0)
#define debug_write(_fmt ,arg...) do { fprintf(stdout,"%*.*s",depth,depth,""); fprintf(stdout,_fmt ,##arg ); fflush(stdout); } while(0)
#define debug_writeln(_fmt ,arg...) do { fprintf(stdout,"%*.*s",depth,depth,""); fprintf(stdout,_fmt ,##arg ); fputc('\n',stdout); fflush(stdout); } while(0)


/*! \brief AST Node Types
 */
typedef enum { ast_Nil=0, ast_Pool=42, ast_Atom=0x6106, ast_Pair=0x8108 } ast_type_t;


union _ast_node_t {
	/* first union member is used for static initializations */
	struct _ant_sz {
		ast_type_t _;
		void*_p1;
		void*_p2;
		int _row;
		int _col;
	} raw;
	struct {
		ast_type_t _;
		char __reserve[sizeof(struct _ant_sz)-sizeof(ast_type_t)-sizeof(ast_node_t)];
		ast_node_t next;
	} pool;
	/* position in source text */
	struct {
		ast_type_t _res_at;
		void*__res[2];
		int row;
		int col;
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




#define _atom(__s,_r,_c) (union _ast_node_t[]){{{ast_Atom,__s,NULL,_r,_c}}}
#define _pair(__a,__d,_r,_c) (union _ast_node_t[]){{{ast_Pair,__a,__d,_r,_c}}}


/*! \par __n an AST node
 *  \ret the corresponding character string if __n is an atom
 */
#define getAtom(__n) (ast_type_check((__n),ast_Atom,__FILE__,__LINE__)->atom._str)

/*! \par __n an AST node
 *  \ret car(__n) if __n is a pair
 */
#define getCar(__n) (ast_type_check((__n),ast_Pair,__FILE__,__LINE__)->pair._car)

/*! \par __n an AST node
 *  \ret cdr(__n) if __n is a pair
 */
#define getCdr(__n) (ast_type_check((__n),ast_Pair,__FILE__,__LINE__)->pair._cdr)

static inline int getRow(ast_node_t n) {
	if(!n) return 0;
	return n->pos.row;
}

static inline int getCol(ast_node_t n) {
	if(!n) return 0;
	return n->pos.col;
}

static inline void setRow(ast_node_t n,int r) {
	if(!n) return;
	n->pos.row=r;
}

static inline void setCol(ast_node_t n,int c) {
	if(!n) return;
	n->pos.col=c;
}


#define isAtom(__n) ((__n)&&(__n)->type==ast_Atom)
#define isPair(__n) ((__n)&&(__n)->type==ast_Pair)
#define isNil(__n) ((__n)==NULL)


#define Atom	_atom
#define Cons	_pair
#define Car	getCar
#define Cdr	getCdr
#define Value	getAtom


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
	return n	? n->type==expected	? n
						:_atc_fail(n, expected, _f, _l)
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


ast_node_t newAtom(const char*data,int row,int col);
ast_node_t newPair(const ast_node_t a,const ast_node_t d,const int row,const int col);
//void delete_node(node_cache_t cache, ast_node_t n);

void print_pair(ast_node_t n);



static inline ast_node_t Append(const ast_node_t a,const ast_node_t b) {
	ast_node_t ptr;
	if(!b) {
		return (ast_node_t )a;
	}
	if(!a) {
		return (ast_node_t )b;
	}
	assert(isPair(b));
	assert(isPair(a));
	ptr=(ast_node_t )a;
	//debug_write("Append ");dump_node(b);
	//debug_write("    to ");dump_node(a);
	while(ptr&&isPair(ptr)&&ptr->pair._cdr) ptr=ptr->pair._cdr;
	assert(isPair(ptr));
	assert(ptr->pair._cdr==NULL);
	ptr->pair._cdr=(ast_node_t )b;
	//debug_write("     = ");dump_node(a);
	return (ast_node_t )a;
}



#endif


