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
#include "tinyap_alloc.h"
#include "string_registry.h"


static unsigned long int cache_collisions = 0;
static unsigned long int cache_popu = 0;
static unsigned long int cache_dup_keys = 0;
static unsigned long int cache_max_depth = 0;


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
	cache_popu = 0;
	cache_dup_keys = 0;
	cache_collisions = 0;
	cache_max_depth = 0;
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
	int i,j;
	node_cache_entry_t q;
//	printf("node_cache_flush\n");
	for(i=0;i<NODE_CACHE_SIZE;i+=1) {
		while(cache[i]) {
			q=cache[i];
			cache[i]=q->next;
			//delete_node(cache,q->v_node);
			/*free(q);*/
			_tinyap_free(struct _node_cache_entry_t, q);
		}
	}
	if(cache_popu) {
		i = 1000 * (cache_popu-cache_collisions) / cache_popu;
		j = 1000 * (cache_popu-cache_collisions) / NODE_CACHE_SIZE;
		fprintf(stderr,"node cache statistics :\n - maximum population : %lu\n - collision count : %lu\n - duplicate keys : %lu\n - maximum search depth : %li\n - optimal access probability : %3i.%1.1i%%\n - cache usage : %3i.%1.1i%%\n",cache_popu, cache_collisions,cache_dup_keys,cache_max_depth,i/10,i%10,j/10,j%10);
	}
}




#include <stdint.h>     /* defines uint32_t etc */

uint32_t hashlittle( const void *key, size_t length, uint32_t initval);

#define _h_(_accum,_ch) ( (_ch) ^ (((_accum)<<6) + (_accum)) )
/*#define _h_(_accum,_ch) ( (_ch) + (_accum<<6) + (_accum<<16) + _accum )*/

/* sdbm function : hash(i) = hash(i - 1) * 65599 + str[i] */
/* djb2 function : hash(i) = hash(i - 1) * 33 ^ str[i] */
size_t cache_hash(int l, int c, const char*n) {
	unsigned long int slen = strlen(n);
	unsigned long int accum=0;
	/*accum = hashlittle(n,strlen(n),accum);*/
	/*c=~(0x7FFFFFFFl/(1+c));*/
	/*l=~(0x7FFFFFFFl/(1+l));*/
	/*accum = hashlittle(&l,4,accum);*/
	/*accum = hashlittle(&c,4,accum);*/
	/*accum = hashlittle(n,slen,accum);*/
	accum = hashlittle(n,slen,accum);
	/*accum = hashlittle(n,slen,accum);*/
	return (accum^(l*0x10l)^(c*0x101l))&0xFFFF;
	/*return accum&0xFFFF;*/
	/*return accum%NODE_CACHE_SIZE;*/
	/*accum = hashlittle(n,strlen(n),hashword(&l,1,hashword(&c,1,0)));*/
	/*accum = hashlittle(n,strlen(n),accum);*/
	/*return hashlittle(n,strlen(n),accum)&0xFFFF;*/
	/*return accum&(NODE_CACHE_SIZE-1);*/
	/*return accum%NODE_CACHE_SIZE;*/
	/*unsigned long int accum=0;*/
/*	unsigned char*k=(unsigned char*)n;
	unsigned long int ch;
	l+=1;
	c+=1;

	while((ch=*k)) {
		accum = ch + (accum<<6) + (accum<<16) - accum;
		accum = _h_(accum,ch);
		k += 1;
	}

	accum = _h_(accum,'_');
	// simulate char keys for l and c, as in <l>_<c>_<n>
	while(l) { accum = _h_(accum,(l&3)+31); l>>=2; }
	accum = _h_(accum,'_');

	while(c) { accum = _h_(accum,(c&3)+37); c>>=2; }
// */

	/*accum^=hash_int(c);*/
	/*accum+=(l<<13)-l+c;*/
//	printf("hashed %i:%i:\"%s\" to %i\n",l,c,n,((l<<7)+c+accum)%NODE_CACHE_SIZE);
	/*accum ^= (l+1)*65599;*/
	/*accum ^= (l+1)*6599;*/
	/*accum ^= (c+1)*53747;*/
	/*accum ^= (c+1)*573;*/
	/*return accum%NODE_CACHE_SIZE;*/
	/*return (accum>>16)^(accum&0xFFFF);*/
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
		/*if(l==nce->k_l&&c==nce->k_c&&(!strcmp(rule,nce->k_rule))) {*/
		if(l==nce->k_l&&c==nce->k_c&&(!TINYAP_STRCMP(rule,nce->k_rule))) {
//			*node_p = copy_node(nce->v_node);
			*node_p = nce->v_node;
			/*printf("At %i:%i for %s, node_cache_retrieve has found %s\n",nce->k_l,nce->k_c,nce->k_rule,tinyap_serialize_to_string(*node_p));*/
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
	/*nce = (node_cache_entry_t) malloc(sizeof(struct _node_cache_entry_t));*/
	nce = _tinyap_alloc(struct _node_cache_entry_t);
	nce->next = cache[ofs];

#ifdef NODE_CACHE_STATS

	if(nce->next) {
		int n=0;
		node_cache_entry_t tmp=nce;
		cache_collisions+=1;
		nce = cache[ofs];
		/*while(nce&&!(l==nce->k_l&&c==nce->k_c&&(!strcmp(expr_op,nce->k_rule)))) {*/
		while(nce&&!(l==nce->k_l&&c==nce->k_c&&(!TINYAP_STRCMP(expr_op,nce->k_rule)))) {
			nce = nce->next;
			n+=1;
		}
		if(nce) {
			cache_dup_keys += 1;
		}
		while(nce) { nce=nce->next; n+=1; }
		if(n>cache_max_depth) {
			cache_max_depth=n;
		}
		nce=tmp;
		/*fprintf(stderr,"hash collision %i:%i:\"%s\" hashed to %i, but %i:%i:\"%s\" in slot\n",l,c,expr_op,ofs,*/
			/*cache[ofs]->k_l,cache[ofs]->k_c,cache[ofs]->k_rule);*/
	}
	cache_popu+=1;

#endif

	nce->k_rule=expr_op;
	nce->k_l = l;
	nce->k_c = c;
	nce->v_node = node;
	nce->v_ofs = ofs_p;
	cache[ofs] = nce;
}

