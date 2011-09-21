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

#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <algorithm>


#ifdef DEBUG_ALLOCS

struct alloc_data {
	unsigned long allocs;
	unsigned long frees;
	const char* what;
	alloc_data() : allocs(0), frees(0), what(0) {}
	~alloc_data() {}
	alloc_data(const alloc_data& ad) : allocs(ad.allocs), frees(ad.frees), what(ad.what) {}
	alloc_data& operator=(const alloc_data& ad) {
		allocs = ad.allocs;
		frees = ad.frees;
		what = ad.what;
		return *this;
	}
};

typedef std::map<size_t, alloc_data> alloc_by_line_t;
typedef std::map<const char*, alloc_by_line_t> alloc_by_file_and_line_t;
typedef alloc_data* alloc_counter_t;

alloc_by_file_and_line_t alloc_counters;

std::map<void*, alloc_counter_t> buffer_to_alloc_data;

static inline std::string __fl2str(const char*f, size_t l, const char* what, size_t frees) {
	std::stringstream tmp;
	tmp << frees << '\t';
	if(what) {
		tmp << std::setiosflags(std::ios_base::left) << std::setw(16) << what;
	}
	tmp << f << ':' << l;
	return std::string(tmp.str());
}


extern "C" {

	void* _alloc_debug(struct __allocator* al, const char* f, size_t l, const char* what) {
		void* ret = _alloc(al);
		record_alloc(ret, f, l, what);
		return ret;
	}

	void _free_debug(struct __allocator* al, void* p) {
		/*std::pair<int, int>& ctr = alloc_counters[f][l];*/
		/*ctr.second--;*/
		alloc_counter_t ad = buffer_to_alloc_data[p];
		if(ad) {
			buffer_to_alloc_data.erase(p);
			ad->frees++;
		}
		_free(al, p);
	}

	void record_alloc(void*buf, const char* f, size_t l, const char* what) {
		alloc_data& ad = alloc_counters[f][l];
		ad.allocs++;
		if(!ad.what) {
			ad.what = what;
		}
		buffer_to_alloc_data[buf] = &ad;
	}

	void _dump_allocs() {
		std::multimap<int, std::string> results;
		alloc_by_file_and_line_t::iterator fi, fj;
		alloc_by_line_t::iterator li, lj;
		fi = alloc_counters.begin();
		fj = alloc_counters.end();
		for(;fi!=fj;++fi) {
			const char* f = (*fi).first;
			li = (*fi).second.begin();
			lj = (*fi).second.end();
			for(;li!=lj;++li) {
				size_t l = (*li).first;
				alloc_data& ad = (*li).second;
				results.insert(std::pair<int, std::string>(ad.allocs, __fl2str(f, l, ad.what, ad.frees)));
			}
		}

		std::multimap<int, std::string>::iterator i, j;
		std::cout << "ALLOCs\tFREEs\tLOCATION" << std::endl;
		for(i=results.begin(), j=results.end(); i!=j; ++i) {
			std::cout << (*i).first << '\t' << (*i).second << std::endl;
		}
	}
}

#endif



struct _memorybloc {
	struct _alloc_unit node;
	void* reserve;
	unsigned long reserve_size;
};

extern unsigned int tinyap_allocs;
extern unsigned int tinyap_reallocs;
extern unsigned int tinyap_frees;
extern unsigned int tinyap_ram_size;

#define BLOC_DATA(_mb) ((void*) ( ((char*)(_mb)) + sizeof(struct _memorybloc) ))
#define BLOC_ALLOC_SIZE(_n) (sizeof(struct _memorybloc)+_n)
#define BLOC_RESERVE_INCR(_mb, _sz) (_mb->reserve = ( ((char*)_mb->reserve) + item_size ))

static inline struct _memorybloc* _alloc_bloc(unsigned long item_size, unsigned long max_allocs) {
	size_t mbsz = BLOC_ALLOC_SIZE(item_size*max_allocs);
	struct _memorybloc* mb = (struct _memorybloc*) malloc(mbsz);
	tinyap_ram_size += mbsz;
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

#ifndef USE_MALLOC

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
		tinyap_reallocs += 1;
		return x;
	} else if(current_bloc&&current_bloc->reserve_size) {
		/*printf(" : from reserve\n");*/
	/*fflush(stdout);*/
		tinyap_allocs += 1;
		return bloc_alloc(current_bloc, A->size);
	} else {
		/*printf(" : new bloc\n");*/
	/*fflush(stdout);*/
		x = (struct _alloc_unit*)_alloc_bloc(A->size, (1<<10)-1);
		x->next = A->blocs.head;
		/*x->prev=NULL;*/
		A->blocs.head = x;
		/*if(!A->blocs.tail) {*/
			/*A->blocs.tail=x;*/
		/*}*/
		tinyap_allocs += 1;
		return bloc_alloc((struct _memorybloc*)x, A->size);
	}
}


void _free(struct __allocator*A, void* ptr) {
	struct _alloc_unit*n = (struct _alloc_unit*)ptr;
	/*n->prev=NULL;*/
	tinyap_frees += 1;
	n->next=A->free.head;
	A->free.head = n;
}


#else

void* _alloc(struct __allocator*A) {
	return malloc(A->size);
}

void _free(struct __allocator*A, void* ptr) {
	free(ptr);
}

#endif
