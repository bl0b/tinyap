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

#include "node_cache.h"




struct _node_cache_entry_t {
	struct _node_cache_entry_t* next;
	int k_l, k_c;			// k_* : composite key
	const char* k_rule;
	ast_node_t v_node;		// v_* : value;
	size_t v_ofs;
};




void node_cache_init(node_cache_t cache) {
	int i;
//	printf("node_cache_init\n");
	for(i=0;i<NODE_CACHE_SIZE;i++) {
		cache[i] = NULL;
	}
}

//void node_cache_clear_node(node_cache_t cache, ast_node_t n) {
//	int i;
//	node_cache_entry_t q;
//	for(i=0;i<NODE_CACHE_SIZE;i+=1) {
//		if(cache[i]) {
//			q = cache[i];
//			do {
//				if(q->v_node==n) {
////					printf("   also found at #%i\n",i);
//					q->v_node=NULL;
//				}
//				q = q->next;
//			} while(q);
//		}
//	}
//}


void delete_node(node_cache_t cache, ast_node_t n);

void node_cache_flush(node_cache_t cache) {
	int i;
	node_cache_entry_t q;
//	printf("node_cache_flush\n");
	for(i=0;i<NODE_CACHE_SIZE;i+=1) {
		while(cache[i]) {
			q=cache[i];
			cache[i]=q->next;
			//delete_node(cache,q->v_node);
			free(q);
		}
	}
}

size_t cache_hash(int l, int c, const char*n) {
	unsigned int accum=(l<<7)+c;
	char*k=(char*)n;
	while(*k) {
		accum = (accum^(accum<<8))+*k;
		k += 1;
	}
//	printf("hashed %i:%i:\"%s\" to %i\n",l,c,n,((l<<7)+c+accum)%NODE_CACHE_SIZE);
	return accum%NODE_CACHE_SIZE;
}


ast_node_t copy_node(ast_node_t a) {
	if(isNil(a)) {
		return NULL;
	} else if(isPair(a)) {
		return newPair(copy_node(a->pair._car), copy_node(a->pair._cdr),a->pos.row,a->pos.col);
	} else if(isAtom(a)) {
		return newAtom(a->atom._str,a->pos.row,a->pos.col);
	}
	return NULL;
}


int node_cache_retrieve(node_cache_t cache, int l, int c, const char* rule, ast_node_t* node_p, size_t* ofs_p) {
	size_t ofs = cache_hash(l,c,rule);
	node_cache_entry_t nce = cache[ofs];
	while(nce) {
		if(l==nce->k_l&&c==nce->k_c&&(!strcmp(rule,nce->k_rule))) {
//			*node_p = copy_node(nce->v_node);
			*node_p = nce->v_node;
//			printf("node_cache_retrieve has found %s\n",tinyap_serialize_to_string(*node_p));
			*ofs_p = nce->v_ofs;
			return 1;
		} else {
			nce = nce->next;
		}
	}
	return 0;
}

void node_cache_add(node_cache_t cache, int l, int c, const char* expr_op, ast_node_t node, size_t ofs_p) {
	node_cache_entry_t nce;
	size_t ofs = cache_hash(l,c,expr_op);
	nce = (node_cache_entry_t) malloc(sizeof(struct _node_cache_entry_t));
	nce->next = cache[ofs];
	nce->k_rule=expr_op;
	nce->k_l = l;
	nce->k_c = c;
	nce->v_node = node;
	nce->v_ofs = ofs_p;
	cache[ofs] = nce;
}

