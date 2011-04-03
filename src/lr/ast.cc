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
namespace ext = __gnu_cxx;

extern "C" {
#include "config.h"
#include "ast.h"
#include "tinyap_alloc.h"
#include <regex.h>
#include "stack.h"
#include "string_registry.h"

ast_node_t PRODUCTION_OK_BUT_EMPTY = (union _ast_node_t[]){{ {ast_Nil, 0, 0, 0, 0} }};

volatile int depth=0;

volatile int _node_alloc_count=0;

volatile ast_node_t node_pool = NULL;

#define _node(_tag,_contents) Cons(Atom(_tag),(_contents))

static tinyap_stack_t node_stack;



void node_pool_init() {
	/*node_stack = new_stack();*/
}


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
		/*push(node_stack,ret);*/
	} else {
		node_pool = node_pool->pool.next;
	}
	return ret;
}

void node_dealloc(ast_node_t node) {
	if(node&&node->type!=ast_Pool) {
		node->type = ast_Pool;
		node->pool.next = node_pool;
		node_pool = node;
	}
	/*(void) (node&&node->type!=ast_Pool ? node->type = ast_Pool, node->pool.next = node_pool, node_pool = node : 0);*/
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

typedef std::pair<const char*, size_t> atom_key;
typedef std::pair<ast_node_t, ast_node_t> pair_key;

struct comp_atom {
	bool operator()(const atom_key&a, const atom_key&b) const {
		return !strcmp(a.first, b.first) && a.second==b.second;
	}
};

struct hash_atom {
	ext::hash<const char*> hs;
	ext::hash<size_t> ho;
	size_t operator()(const atom_key&a) const {
		return hs(a.first)^ho(a.second);
	}
};

struct comp_pair {
	bool operator()(const pair_key&a, const pair_key&b) const {
		return a.first==b.first && a.second==b.second;
	}
};

struct hash_pair {
	ext::hash<ast_node_t> h;
	size_t operator()(const pair_key&a) const {
		return (((size_t)a.first)<<16)^((size_t)a.second);
	}
};

typedef ext::hash_map<atom_key, ast_node_t, hash_atom, comp_atom> atom_registry_t;
typedef ext::hash_map<pair_key, ast_node_t, hash_pair, comp_pair> pair_registry_t;

ast_node_t newAtom(const char*data,size_t offset) {
	static atom_registry_t atom_registry;
	char* reg = regstr(data);
	ast_node_t ret = atom_registry[atom_key(reg, offset)];
	if(!ret) {
		ret = node_alloca();
		/*std::cout << "new atom " << ret << " \"" << data << '"';*/
		ret->atom._str=reg;
		atom_registry[atom_key(ret->atom._str, offset)] = ret;
		ret->type=ast_Atom;
		ret->raw._p2=NULL;	/* useful for regexp cache hack */
		ret->pos.offset = offset;
		/*ret->pos.col=0;*/
		ret->node_flags=0;
		ret->raw.ref = 0;
	/*} else {*/
		/*std::cout << "reuse alloc'd atom " << ret;*/
	}
	/*std::cout << "  => " << ret->atom._str << std::endl;*/
	ret->raw.ref++;

	return ret;
}


ast_node_t newPair(const ast_node_t a,const ast_node_t d) {
	static pair_registry_t pair_registry;
	ast_node_t ret = pair_registry[pair_key(a, d)];
	if(!ret) {
		ret = node_alloca();
		/*std::cout << "new pair " << ret << " : " << a << ", " << d << std::endl;*/
		pair_registry[pair_key(a, d)] = ret;
		ret->type=ast_Pair;
		ret->pair._car=(ast_node_t )a;
		ret->pair._cdr=(ast_node_t )d;
		if(a) { ((ast_node_t)a)->raw.ref++; }
		if(d) { ((ast_node_t)d)->raw.ref++; }
		ret->pos.offset=0;
		/*ret->pos.col=0;*/
		ret->node_flags=0;
		ret->raw.ref=0;
	/*} else {*/
		/*std::cout << "reuse alloc'd pair " << ret << std::endl;*/
	}
	ret->raw.ref++;
	return ret;
}



void delete_node(ast_node_t n) {
//	static int prout=0;
	if(!n) return;
	n->raw.ref--;
	if(n->raw.ref) {
		return;
	}
	switch(n->type) {
	case ast_Atom:
		/*assert(n->atom._str);*/
		/*free(n->atom._str);*/
		if(!(n->node_flags&ATOM_IS_NOT_STRING)) {
			/*unregstr(n->atom._str);*/
		}
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

}/* extern "C" */

