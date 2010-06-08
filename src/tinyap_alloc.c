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
	GenericListNode node;
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


#define ALLOCA_INIT(_sz) { _sz, 0, {NULL, NULL, 0}, {NULL, NULL, 0}, PTHREAD_MUTEX_INITIALIZER }

struct __allocator
	_alloca_4 = ALLOCA_INIT(W(4)),
	_alloca_8 = ALLOCA_INIT(W(8)),
	_alloca_16 = ALLOCA_INIT(W(16)),
	_alloca_32 = ALLOCA_INIT(W(32)),
	_alloca_64 = ALLOCA_INIT(W(64))
;


void* _alloc(struct __allocator*A) {
	struct _memorybloc* current_bloc = (struct _memorybloc*)A->blocs.head;
	GenericListNode* x;
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
		x = (GenericListNode*)_alloc_bloc(A->size, (1<<10)-1);
		x->next = A->blocs.head;
		x->prev=NULL;
		A->blocs.head = x;
		if(!A->blocs.tail) {
			A->blocs.tail=x;
		}
		return bloc_alloc((struct _memorybloc*)x, A->size);
	}
}

void* _free(struct __allocator*A, void* ptr) {
	GenericListNode*n = (GenericListNode*)ptr;
	n->prev=NULL;
	n->next=A->free.head;
	A->free.head = n;
}

#if 0



pthread_mutex_t tinyap_allocmutex4;
pthread_mutex_t tinyap_allocmutex8;
pthread_mutex_t tinyap_allocmutex16;

typedef unsigned char _qw[16];
typedef unsigned char _ow[32];
typedef unsigned char _para_t[64];

typedef _qw* qw[4];
typedef _ow* ow[8];
typedef _para_t* para_t[16];

static void* quawords=NULL;
static size_t qw_tinyap_free=0;
static size_t qw_total=0;
static void* octwords=NULL;
static size_t ow_tinyap_free=0;
static size_t ow_total=0;
static void* paragraphs=NULL;
static size_t para_tinyap_free=0;
static size_t para_total=0;

GenericList qWordBufs={NULL,NULL,0},oWordBufs={NULL,NULL,0},paraBufs={NULL,NULL,0};

#define _offset(_p,_o,_sz) (((char*)(_p))+(_o)*(_sz))

static volatile long tinyap_alloc_is_init=0;

void term_tinyap_alloc() {
	GenericListNode* gln;
	if(!tinyap_alloc_is_init) {
		return;
	}
	/*vm_printf("freeing alloc blocs.\n");*/
	while(qWordBufs.head) {
		gln=qWordBufs.head->next;
		free(qWordBufs.head);
		qWordBufs.head=gln;
	}
	while(oWordBufs.head) {
		gln=oWordBufs.head->next;
		free(oWordBufs.head);
		oWordBufs.head=gln;
	}
	while(paraBufs.head) {
		gln=paraBufs.head->next;
		free(paraBufs.head);
		paraBufs.head=gln;
	}
	tinyap_alloc_is_init=0;
}



static inline void* __new_buf(size_t size,size_t countPerBuf,GenericList*l,void**first,size_t*total,size_t*free__) {
	char*ptr;
	void*p;
	long i;
//	vm_printf("__new_buf(%i,%i)\n",size,countPerBuf);
	listAddTail(*l,(GenericListNode*)*first);
	ptr=((char*)*first)+sizeof(GenericListNode);
	*first=(void*)(ptr+size);
	*total+=countPerBuf;
	--countPerBuf;
	*free__+=countPerBuf;
	p=*first;
	--countPerBuf;
	for(i=0;i<countPerBuf;i++) {
//		vm_printf("%p, ",_offset(p,i+1,size));
		*(void**)_offset(p,i,size)=_offset(p,i+1,size);
//		vm_printf("%p, ",*(void**)_offset(p,i,size));
	}
//	vm_printf("NULL.\n");
	*(void**)_offset(p,i,size)=NULL;
//	vm_printf(" [%p]\n",ptr);fflush(stdout);
	return (void*)ptr;
}


static inline void* _ta_malloc(size_t size, size_t countPerBuf, GenericList*l, void**first, size_t*total, size_t*free__) {
	return (*first=malloc(countPerBuf*size+sizeof(GenericListNode)))
			? __new_buf(size,countPerBuf,l,first,total,free__)
			: NULL;
}

static inline void* _ta_fast(void* ptr, void**first, size_t*free__) {
	*first=**(void***)first;
	--*free__;
	return ptr;
}

static inline void* __tinyap_allocate_(pthread_mutex_t*mtx,size_t size,size_t countPerBuf,GenericList*l,void**first,size_t*total,size_t*free__) {
	char*ptr;
	/*pthread_mutex_lock(mtx);*/
	ptr = *first	? (ptr=(char*)*first)	? _ta_fast(ptr, first, free__)
						: NULL
			: _ta_malloc(size, countPerBuf, l, first, total, free__);
	/*pthread_mutex_unlock(mtx);*/
	return (void*)ptr;
}

static inline void __collect_(pthread_mutex_t*mtx,void*ptr,GenericList*l,void**first) {
	if(!ptr) return;
	/*pthread_mutex_lock(mtx);*/
//	vm_printf("collect %p (first=%p, next=%p)\n",ptr,*first,*(void**)*first);
	*(void**)ptr=*first;
	*first=ptr;
//	vm_printf("after collecting %p : first=%p, next=%p\n",ptr,*first,*(void**)*first);
	/*pthread_mutex_unlock(mtx);*/
}


unsigned long tinyap_allocBlocSize=1048576;

#define SIZE(n) (n*sizeof(void*))
#define COUNT(n) ((tinyap_allocBlocSize-sizeof(GenericListNode))/SIZE(n))

void*_tinyap_alloc_4w() { return __tinyap_allocate_(&tinyap_allocmutex4,SIZE(4),COUNT(4),&qWordBufs,&quawords,&qw_total,&qw_tinyap_free); }
void*_tinyap_alloc_8w() { return __tinyap_allocate_(&tinyap_allocmutex8,SIZE(8),COUNT(8),&oWordBufs,&octwords,&ow_total,&ow_tinyap_free); }
void*_tinyap_alloc_16w() { return __tinyap_allocate_(&tinyap_allocmutex16,SIZE(16),COUNT(16),&paraBufs,&paragraphs,&para_total,&para_tinyap_free); }

void _tinyap_free_4w(void*p) { __collect_(&tinyap_allocmutex4,p,&qWordBufs,&quawords); }
void _tinyap_free_8w(void*p) { __collect_(&tinyap_allocmutex8,p,&oWordBufs,&octwords); }
void _tinyap_free_16w(void*p) { __collect_(&tinyap_allocmutex16,p,&paraBufs,&paragraphs); }


void init_tinyap_alloc() {
	if(tinyap_alloc_is_init) {
		return;
	}
	tinyap_alloc_is_init=1;
	pthread_mutex_init(&tinyap_allocmutex4,NULL);
	/*pthread_mutex_trylock(&allocmutex4);*/
	/*pthread_mutex_unlock(&allocmutex4);*/
	pthread_mutex_init(&tinyap_allocmutex8,NULL);
	/*pthread_mutex_trylock(&allocmutex8);*/
	/*pthread_mutex_unlock(&allocmutex8);*/
	pthread_mutex_init(&tinyap_allocmutex16,NULL);
	/*pthread_mutex_trylock(&allocmutex16);*/
	/*pthread_mutex_unlock(&allocmutex16);*/

	atexit(term_tinyap_alloc);
}


#endif
