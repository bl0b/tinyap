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
#include "string_registry.h"

#include "3rd_party/fnv.h"

#define FNV_HASH(_x, _s) fnv_32a_buf(_x, _s, FNV1_32A_INIT)

static long int total_collisions = 0;
static long int max_collisions = 0;
static long int cache_collisions = 0;
static long int cache_popu = 0;
static long int max_popu = 0;
static long int cache_dup_keys = 0;
static long int cache_max_depth = 0;


#define INC_POPU do { cache_popu+=1; max_popu = cache_popu>max_popu ? cache_popu : max_popu; } while(0)
#define DEC_POPU do { cache_popu-=1; } while(0)

#define INC_COLL do { total_collisions+=1; cache_collisions+=1; max_collisions = cache_collisions>max_collisions ? cache_collisions : max_collisions; } while(0)
#define DEC_COLL(_v) do { cache_collisions-=(_v&&_v->next); } while(0)

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
	max_popu = 0;
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

void node_cache_clean(node_cache_t cache, struct _pos_cache_t* pos) {
	int i;
	node_cache_entry_t q, p;
	int threshold = pos->row-20;
//	printf("node_cache_flush\n");
	for(i=0;i<NODE_CACHE_SIZE;i+=1) {
		while(cache[i]&&cache[i]->k_l<threshold) {
			q = cache[i];
			cache[i]=q->next;
			tinyap_free(struct _node_cache_entry_t, q);
#ifdef NODE_CACHE_STATS
			DEC_POPU;
			DEC_COLL(cache[i]);
#endif
		}
		if(cache[i]) {
			p=cache[i];
			q=p->next;
			while(q) {
				if(q->k_l<threshold) {
					p->next = q->next;
					tinyap_free(struct _node_cache_entry_t, q);
#ifdef NODE_CACHE_STATS
					DEC_POPU;
					DEC_COLL(cache[i]);
#endif
					q = p->next;
				} else {
					q = q->next;
				}
			}
		}
	}
}

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
			tinyap_free(struct _node_cache_entry_t, q);
		}
	}
	if(max_popu) {
		i = 1000 * (max_popu-max_collisions) / max_popu;
		j = 1000 * (max_popu-max_collisions) / NODE_CACHE_SIZE;
		fprintf(stderr,"node cache statistics :\n - maximum population : %li (final %li, total space %i)\n - collision count : %li\n - duplicate keys : %li\n - maximum search depth : %li\n - optimal access probability : %3i.%1.1i%%\n - cache usage : %3i.%1.1i%%\n", max_popu,cache_popu,NODE_CACHE_SIZE, max_collisions, cache_dup_keys, cache_max_depth, i/10,i%10,j/10,j%10);
	}
}




#include <stdint.h>     /* defines uint32_t etc */

uint32_t hashlittle( const void *key, size_t length, uint32_t initval);

#define _h_(_accum,_ch) ( (_ch) ^ (((_accum)<<6) + (_accum)) )
/*#define _h_(_accum,_ch) ( (_ch) + (_accum<<6) + (_accum<<16) + _accum )*/

#define _reduce(_x, _bsz) ((((_x)>>(_bsz))^(_x)) & ((1<<(_bsz))-1))

#define _r2(_x) _reduce(_x, 2)
#define _r4(_x) _reduce(_x, 4)
#define _r8(_x) _reduce(_x, 8)
#define _r16(_x) _reduce(_x, 16)
#define rowcolhash(_x) do { _x = _r16(_x); _x = _r8(_x); _x = _r4(_x); } while(0)

static inline unsigned long hash_bytes(unsigned char *str, unsigned int len, unsigned long hash)
{
	/*unsigned long hash = 5381;*/
	while(len) {
	        int c = *str;
		/*counter = permut[(unsigned char)(counter+c)];*/
                /*hash = (hash*33) ^ c;*/
                hash = ((hash << 6) + hash) ^ c;
		/*hash = (hash << 5) | (hash >> 27);*/
                ++str;
		--len;
		/*counter&=0xFF;*/
        }
	return hash;
}




/* sdbm function : hash(i) = hash(i - 1) * 65599 + str[i] */
/* djb2 function : hash(i) = hash(i - 1) * 33 ^ str[i] */
static inline size_t cache_hash(int l, int c, const char*n) {
	/*struct _ch {*/
		/*unsigned char c;*/
		/*unsigned char l;*/
		/*ast_node_t e;*/
		/*unsigned short int n;*/
		/*const char* str;*/
		/*char str[256];*/
	/*} buf = { l, c, "" };*/
	/*} buf = { c, l, (((unsigned int)n)>>2)&0xFFFF };*/
	/*} buf = { l, c, expr };*/
	/*size_t ret = *(size_t*)&buf;*/
	/*ret = _r16(ret);*/
	/*ret = _r8(ret);*/
	/*return ret;*/
	/*size_t ret;*/
	/*strncpy(buf.str, n, 255);*/
	/*ret = FNV_HASH(&buf, sizeof(struct _ch));*/
	/*ret = FNV_HASH(&buf, sizeof(int)+sizeof(int)+strlen(buf.str));*/
	/*return (ret^(ret>>(NODE_CACHE_BITSIZE)))&NODE_CACHE_MASK;*/
	/*return ret%NODE_CACHE_MOD;*/
	/*return ret&NODE_CACHE_MASK;*/
	/*size_t ret = hashlittle(n, strlen(n), (0xdeadbeef*l*c)^(0xdeadbeef+l+c));*/
	size_t ret = hash_bytes((char*)n, strlen(n),0xdeadb33f);
	/*size_t ret = hashlittle(n, strlen(n),0xdeadb33f);*/
	/*size_t ret = FNV_HASH(n, strlen(n))^l^c;*/
	/*ret *= (ret>>16);*/
	ret ^= (0xBC7F3F1F*l)^((~0xBC7F3F1F)*c);
	/*ret += (ret >> ((8*sizeof(void*))-NODE_CACHE_BITSIZE));*/
	return ret;
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
#if 1
	size_t ofs = cache_hash(l,c,rule)&((1<<NODE_CACHE_BITSIZE)-1);
	node_cache_entry_t nce = cache[ofs];
	while(nce) {
		/*if(l==nce->k_l&&c==nce->k_c&&(!strcmp(rule,nce->k_rule))) {*/
		if(l==nce->k_l&&c==nce->k_c&&(!TINYAP_STRCMP(rule,nce->k_rule))) {
//			*node_p = copy_node(nce->v_node);
			*node_p = nce->v_node;
			/*printf("At %i:%i for %s, node_cache_retrieve has found %s\n",nce->k_l,nce->k_c,nce->k_rule,tinyap_serialize_to_string(*node_p));*/
			*ofs_p = nce->v_ofs;
			return 1;
		}
		nce = nce->next;
	}
#endif
	return 0;
}

void node_cache_add(node_cache_t cache, int l, int c, const char* expr_op, ast_node_t node, size_t ofs_p) {
#if 1
	node_cache_entry_t nce;
	size_t ofs = cache_hash(l,c,expr_op)&((1<<NODE_CACHE_BITSIZE)-1);
	/*nce = (node_cache_entry_t) malloc(sizeof(struct _node_cache_entry_t));*/
	nce = tinyap_alloc(struct _node_cache_entry_t);
#  ifdef NODE_CACHE_ALLOW_COLLISION
	nce->next = cache[ofs];

#    ifdef NODE_CACHE_STATS

	if(nce->next) {
		int n=0;
		node_cache_entry_t tmp=nce;
		nce = cache[ofs];
		/*while(nce&&!(l==nce->k_l&&c==nce->k_c&&(!strcmp(expr_op,nce->k_rule)))) {*/
		/*while(nce&&!(l==nce->k_l&&c==nce->k_c&&(!TINYAP_STRCMP(expr_op,nce->k_rule)))) {*/
		while(nce&&!(l==nce->k_l&&c==nce->k_c&&expr_op==nce->k_rule)) {
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
		fprintf(stderr,"hash collision %i:%i:\"%s\" hashed to %i, but %i:%i:\"%s\" in slot\n",l,c,expr_op,ofs, cache[ofs]->k_l,cache[ofs]->k_c,cache[ofs]->k_rule);
		INC_COLL;
	}
	INC_POPU;

#    endif

#  else /* NODE_CACHE_ALLOW_COLLISION */

	if(cache[ofs]) {
		tinyap_free(struct _node_cache_entry_t, cache[ofs]);
	}

	nce->next = NULL;

#  endif

	nce->k_rule=expr_op;
	nce->k_l = l;
	nce->k_c = c;
	nce->v_node = node;
	nce->v_ofs = ofs_p;
	cache[ofs] = nce;
#endif
}

