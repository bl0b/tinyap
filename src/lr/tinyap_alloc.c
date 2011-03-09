/* TinyaML
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

#include "tinyap_alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>

struct _memorybloc {
	struct _alloc_unit node;
	void* reserve;
	unsigned long reserve_size;
};


#define BLOC_DATA(_mb) ((void*) ( ((char*)(_mb)) + sizeof(struct _memorybloc) ))
#define BLOC_ALLOC_SIZE(_n) (sizeof(struct _memorybloc)+_n)
#define BLOC_RESERVE_INCR(_mb, _sz) (_mb->reserve = ( ((char*)_mb->reserve) + item_size ))

static inline struct _memorybloc* _alloc_bloc(unsigned long item_size, unsigned long max_allocs) {
	struct _memorybloc* mb = (struct _memorybloc*) malloc(BLOC_ALLOC_SIZE(item_size*max_allocs));
	mb->reserve = BLOC_DATA(mb);
	mb->reserve_size = max_allocs;
	return mb;
}

static inline void* bloc_alloc(struct _memorybloc* mb, unsigned long item_size) {
	void* ret;
	if(!mb->reserve_size) {
		return NULL;
	}
	ret = mb->reserve;
	--mb->reserve_size;
	BLOC_RESERVE_INCR(mb, item_size);
	return ret;
}


#define ALLOCA_INIT(_sz) { _sz, 0, {NULL}, {NULL}, PTHREAD_MUTEX_INITIALIZER }

struct __allocator
	_alloca_1 = ALLOCA_INIT(W(1)),
	_alloca_2 = ALLOCA_INIT(W(2)),
	_alloca_4 = ALLOCA_INIT(W(4)),
	_alloca_8 = ALLOCA_INIT(W(8)),
	_alloca_16 = ALLOCA_INIT(W(16)),
	_alloca_32 = ALLOCA_INIT(W(32)),
	_alloca_64 = ALLOCA_INIT(W(64))
;


void _term_allocator(struct __allocator*A) {
	struct _alloc_unit* b = A->blocs.head, *tmp;
	while(b) {
		tmp = b;
		b = b->next;
		free(tmp);
	}
}

void* _alloc(struct __allocator*A) {
	struct _memorybloc* current_bloc = (struct _memorybloc*)A->blocs.head;
	struct _alloc_unit* x;
	/*printf("_alloc %u bytes", A->size);*/
	/*fflush(stdout);*/
	if(A->free.head) {
		x = A->free.head;
		/*printf(" : had free item\n");*/
	/*fflush(stdout);*/
		A->free.head = A->free.head->next;
		return x;
	} else if(current_bloc&&current_bloc->reserve_size) {
		/*printf(" : from reserve\n");*/
	/*fflush(stdout);*/
		return bloc_alloc(current_bloc, A->size);
	} else {
		/*printf(" : new bloc\n");*/
	/*fflush(stdout);*/
		x = (struct _alloc_unit*)_alloc_bloc(A->size, (1<<16)-1);
		x->next = A->blocs.head;
		/*x->prev=NULL;*/
		A->blocs.head = x;
		/*if(!A->blocs.tail) {*/
			/*A->blocs.tail=x;*/
		/*}*/
		return bloc_alloc((struct _memorybloc*)x, A->size);
	}
}

void _free(struct __allocator*A, void* ptr) {
	struct _alloc_unit*n = (struct _alloc_unit*)ptr;
	/*n->prev=NULL;*/
	n->next=A->free.head;
	A->free.head = n;
}

