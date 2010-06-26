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
#include "node_cache.h"
#include "tinyap_alloc.h"
#include <regex.h>
#include "stack.h"
#include "string_registry.h"

volatile int depth=0;

volatile int _node_alloc_count=0;

volatile ast_node_t node_pool = NULL;

#define _node(_tag,_contents) Cons(Atom(_tag),(_contents))

static tinyap_stack_t node_stack;

void node_pool_init() {
	node_stack = new_stack();
}


void node_cache_clear_node(node_cache_t cache, ast_node_t n);

size_t node_pool_size() {
	size_t ret = 0;
	ast_node_t n = node_pool;
	while(n) {
		ret += 1;
		n = n->pool.next;
	}
	return ret;
}

ast_node_t node_alloca() {
	ast_node_t ret = node_pool;
	if(!ret) {
		ret = tinyap_alloc(union _ast_node_t);
		_node_alloc_count+=1;
		push(node_stack,ret);
	} else {
		node_pool = node_pool->pool.next;
	}
	return ret;
}

void node_dealloc(ast_node_t node) {
	/*if(node&&node->type!=ast_Pool) {*/
		/*node->type = ast_Pool;*/
		/*node->pool.next = node_pool;*/
		/*node_pool = node;*/
	/*}*/
	(void) (node&&node->type!=ast_Pool ? node->type = ast_Pool, node->pool.next = node_pool, node_pool = node : 0);
}

void delete_node(ast_node_t);

void node_pool_flush() {
	ast_node_t n;

	tinyap_stack_t tmp_stack = new_stack();

/*
	while(node_pool) {
		n = node_pool;
		node_pool = node_pool->pool.next;
		free(n);
		_node_alloc_count-=1;
	}
*/
	while(not_empty(node_stack)) {
		//node_dealloc(pop(ast_node_t,node_stack));
		n = pop(ast_node_t,node_stack);
		if(n->type!=ast_Pool) {
			delete_node(n);
		}
		push(tmp_stack,n);
		//free(n);
		_node_alloc_count-=1;
	}

	/* freeing slogw things down */
	while(not_empty(tmp_stack)) {
		tinyap_free(union _ast_node_t, _pop(tmp_stack));
	}

	free_stack(tmp_stack);

//	printf("after node pool flush, %i nodes remain\n",_node_alloc_count);
}


void node_pool_term() {
	/*printf("node_pool_term\n"); fflush(stdout);*/
	node_pool_flush();
	free_stack(node_stack);
	node_stack = NULL;
}



ast_node_t newAtom(const char*data,int row,int col) {
	ast_node_t ret = node_alloca();
	ret->type=ast_Atom;
	/*ret->atom._str=strdup(data);*/
	ret->atom._str=regstr(data);
	ret->raw._p2=NULL;	/* useful for regexp cache hack */
	ret->pos.row=row;
	ret->pos.col=col;
	ret->node_flags=0;
	return ret;
}


ast_node_t newPair(const ast_node_t a,const ast_node_t d,const int row,const int col) {
	ast_node_t  ret;
/*	if( isAtom(a) && (!strcmp(Value(a),"strip.me")) ) {
		if(isPair(d)) {
			return d;
		} else {
			if(d) {
				return newPair(d,NULL);
			} else {
				return NULL;
			}
		}
	}*/
//	ret=(ast_node_t )malloc(sizeof(union _ast_node_t));
	ret = node_alloca();
	ret->type=ast_Pair;
	ret->pair._car=(ast_node_t )a;
	ret->pair._cdr=(ast_node_t )d;
	ret->pos.row=row;
	ret->pos.col=col;
	ret->node_flags=0;
	return ret;
}



void delete_node(ast_node_t n) {
//	static int prout=0;
	if(!n) return;
	switch(n->type) {
	case ast_Atom:
		/*assert(n->atom._str);*/
		/*free(n->atom._str);*/
		unregstr(n->atom._str);
		if(n->raw._p2) {
			/* regex cache hack */
//			printf("prout %i\n",prout+=1);
			regfree(n->raw._p2);
			tinyap_free(regex_t, n->raw._p2);
//			n->raw._p2=NULL;
		}
		break;
	case ast_Pair:
		delete_node(n->pair._car);
		delete_node(n->pair._cdr);
		break;
	case ast_Nil:;	/* so that -Wall won't complain */
	case ast_Pool:;
		return;
	};
	node_dealloc(n);
//	free(n);
}



