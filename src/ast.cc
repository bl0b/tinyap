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
#include <ext/hash_map>
#include <iostream>
#include <iomanip>
#include <set>
namespace ext = __gnu_cxx;

#include "ast.h"
#include "tinyap_alloc.h"
#include "string_registry.h"

#include "static_init.h"

extern "C" {
#include "config.h"
#include <regex.h>
#include "stack.h"

ast_node_t PRODUCTION_OK_BUT_EMPTY = (union _ast_node_t[]){{ {ast_Nil, 0, 0, 0} }};

volatile int depth=0;

volatile int _node_alloc_count=0;
volatile int _node_dealloc_count=0;
volatile int delete_node_count=0;
volatile int newAtom_count=0;
volatile int newPair_count=0;

volatile ast_node_t node_pool = NULL;

#define _node(_tag,_contents) Cons(Atom(_tag),(_contents))

/*static tinyap_stack_t node_stack;*/



void node_pool_init() {
	/*node_stack = new_stack();*/
}


size_t node_pool_size() {
#if 0
	size_t ret = 0;
	ast_node_t n = node_pool;
	while(n) {
		ret += 1;
		n = n->pool.next;
	}
	return ret;
#endif
	return 0;
}

ast_node_t node_alloca() {
	++_node_alloc_count;
#if 0
	ast_node_t ret = node_pool;
	if(!ret) {
		ret = tinyap_alloc(union _ast_node_t);
		_node_alloc_count+=1;
		/*push(node_stack,ret);*/
	} else {
		node_pool = node_pool->pool.next;
	}
	return ret;
#else
	return (ast_node_t) _alloc(_select_alloca(sizeof(union _ast_node_t)));
#endif
}

void node_dealloc(ast_node_t node) {
	++_node_dealloc_count;
#if 0
	if(node&&node->type!=ast_Pool) {
		node->type = ast_Pool;
		node->pool.next = node_pool;
		node_pool = node;
	}
	/*(void) (node&&node->type!=ast_Pool ? node->type = ast_Pool, node->pool.next = node_pool, node_pool = node : 0);*/
#else
	tinyap_free(union _ast_node_t, node);
#endif
}

void delete_node(ast_node_t);

void node_pool_flush() {
#if 0
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
#endif
}


void node_pool_term() {
	/*printf("node_pool_term\n"); fflush(stdout);*/
	/*node_pool_flush();*/
	/*free_stack(node_stack);*/
	/*node_stack = NULL;*/
}


static atom_registry_t& atom_registry = _static_init.atom_registry;

static pair_registry_t& pair_registry = _static_init.pair_registry;

#ifdef DEBUG_ALLOCS
ast_node_t newAtom_debug(const char*data,size_t offset, const char* f, size_t l) {
	char* reg = regstr(data);
	ast_node_t& ret = atom_registry[atom_key(reg, offset)];
	if(!ret) {
		ret = (ast_node_t)_alloc_debug(_select_alloca(sizeof(union _ast_node_t)), f, l, "newAtom");
		/*std::cout << "new atom " << ret << " \"" << data << '"';*/
		ret->atom._str=reg;
		atom_registry[atom_key(ret->atom._str, offset)] = ret;
		ret->type=ast_Atom;
		ret->raw._p2=NULL;	/* useful for regexp cache hack */
		ret->pos.offset = offset;
		ret->raw.ref = 0;
	/*} else {*/
		/*std::cout << "reuse alloc'd atom " << ret;*/
	}
	/*std::cout << "  => " << ret->atom._str << std::endl;*/
	ret->raw.ref++;

	return ret;
}

ast_node_t newPair_debug(const ast_node_t a,const ast_node_t d, const char* f, size_t l) {
	pair_key k(a, d);
	ast_node_t& ret = pair_registry[k];
	if(!ret) {
		ret = (ast_node_t)_alloc_debug(_select_alloca(sizeof(union _ast_node_t)), f, l, "newPair");
		/*std::cout << "new pair " << ret << " : " << a << ", " << d << std::endl;*/
		/*pair_registry[pair_key(a, d)] = ret;*/
		ret->type=ast_Pair;
		ret->pair._car=(ast_node_t )a;
		ret->pair._cdr=(ast_node_t )d;
		if(a) { ((ast_node_t)a)->raw.ref++; }
		if(d) { ((ast_node_t)d)->raw.ref++; }
		ret->raw.ref=0;
	/*} else {*/
		/*std::cout << "reuse alloc'd pair " << ret << std::endl;*/
	}
	/*ret->raw.ref++;*/
	return ret;
}
#endif

ast_node_t newAtom_impl(const char*data,size_t offset) {
	++newAtom_count;
	char* reg = regstr(data);
	ast_node_t& ret = atom_registry[atom_key(reg, offset)];
	if(!ret) {
		ret = node_alloca();
		/*std::cout << "new atom " << ret << " \"" << data << '"';*/
		ret->atom._str=reg;
		/*atom_registry[atom_key(ret->atom._str, offset)] = ret;*/
		ret->type=ast_Atom;
		ret->raw._p2=NULL;	/* useful for regexp cache hack */
		ret->pos.offset = offset;
		ret->raw.ref = 0;
	/*} else {*/
		/*std::cout << "reuse alloc'd atom " << ret;*/
		/*ret->raw.ref++;*/
	}
	/*std::cerr << "newAtom: " << ret << std::endl;*/

	return ret;
}


ast_node_t newPair_impl(const ast_node_t a,const ast_node_t d) {
	++newPair_count;
	pair_key k(a, d);
	ast_node_t& ret = pair_registry[k];
	if(!ret) {
		ret = node_alloca();
		/*std::cout << "new pair " << ret << " : " << a << ", " << d << std::endl;*/
		/*pair_registry[pair_key(a, d)] = ret;*/
		ret->type=ast_Pair;
		ret->pair._car=(ast_node_t )a;
		ret->pair._cdr=(ast_node_t )d;
		if(a) { ((ast_node_t)a)->raw.ref++; }
		if(d) { ((ast_node_t)d)->raw.ref++; }
		ret->raw.ref=0;
	/*} else {*/
		/*std::cout << "reuse alloc'd pair " << ret << std::endl;*/
		/*ret->raw.ref++;*/
	}
	/*std::cerr << "newPair: " << ret << std::endl;*/
	return ret;
}


#include "static_init.h"
static std::set<ast_node_t>& still_has_refs = _static_init.still_has_refs;

void delete_node(ast_node_t n) {
	static int rec_lvl=0;
	++delete_node_count;
//	static int prout=0;
	if(!(n && n!=PRODUCTION_OK_BUT_EMPTY)) return;
	n->raw.ref--;
	if(n->raw.ref>0) {
		/*std::cout << std::setw(rec_lvl) << "Node " << n << " still has " << n->raw.ref << " references." << std::endl;*/
		/*if(still_has_refs.find(n)==still_has_refs.end()) {*/
			/*still_has_refs.insert(n);*/
		/*}*/
		return;
	}
	/*else {*/
		/*still_has_refs.erase(still_has_refs.lower_bound(n), still_has_refs.upper_bound(n));*/
	/*}*/
	++rec_lvl;
	switch(n->type) {
	case ast_Atom:
		/*std::cout << std::setw(rec_lvl) << "deleting an atom \"" << n->atom._str << '"' << std::endl;*/
		assert(n->atom._str);
		/*free(n->atom._str);*/
		atom_registry.erase(atom_key(n->atom._str, n->pos.offset));
		/*if(!(n->node_flags&ATOM_IS_NOT_STRING)) {*/
		unregstr(n->atom._str);
		/*}*/
		/*if(n->raw._p2) {*/
			/* regex cache hack */
//			printf("prout %i\n",prout+=1);
			/*regfree(n->raw._p2);*/
			/*tinyap_free(regex_t, n->raw._p2);*/
//			n->raw._p2=NULL;

			/* FIXME ! */
			/*pcre_free(n->raw._p2);*/
		/*}*/
		break;
	case ast_Pair:
		/*std::cout << std::setw(rec_lvl) << "deleting a pair (" << n->pair._car << ", " << n->pair._cdr << ')' << std::endl;*/
		delete_node(n->pair._car);
		delete_node(n->pair._cdr);
		pair_registry.erase(pair_key(n->pair._car, n->pair._cdr));
		break;
	case ast_Nil:;	/* so that -Wall won't complain */
	case ast_Pool:;
		--rec_lvl;
		return;
	};
	/*node_dealloc(n);*/
//	free(n);
	++_node_dealloc_count;
	tinyap_free(union _ast_node_t, n);
	--rec_lvl;
}

}/* extern "C" */

