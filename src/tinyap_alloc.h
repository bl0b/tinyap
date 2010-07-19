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

#ifndef __TINYAP_ALLOC_H__
#define __TINYAP_ALLOC_H__

#include "list.h"
#include <pthread.h>
#include <malloc.h>

struct __allocator {
	unsigned long size;
	unsigned long total;
	GenericList blocs;
	GenericList free;
	pthread_mutex_t mutex;
};

extern struct __allocator _alloca_4, _alloca_8, _alloca_16, _alloca_32, _alloca_64;


#define W(_x) ((_x)*sizeof(unsigned long))

#define _select_alloca(_N) ( \
	_N<=W(4) \
		? &_alloca_4 \
		: _N<=W(8) \
			? &_alloca_8 \
			: _N<=W(16) \
				? &_alloca_16 \
				: _N<=W(32) \
					? &_alloca_32 \
					: _N<=W(64) \
						? &_alloca_64 \
						: NULL)



#define tinyap_alloc(type) (type*) _alloc(_select_alloca(sizeof(type)))
#define tinyap_free(type, ptr) _free(_select_alloca(sizeof(type)), ptr)

void* _alloc(struct __allocator*A);
void _free(struct __allocator*A, void* ptr);

void _term_allocator(struct __allocator*A);

#define init_tinyap_alloc() ((void)0)
#define term_tinyap_alloc() do {\
		_term_allocator(&_alloca_4);\
		_term_allocator(&_alloca_8);\
		_term_allocator(&_alloca_16);\
		_term_allocator(&_alloca_32);\
		_term_allocator(&_alloca_64);\
	} while(0)

#endif


#if 0

#define BLOC_COUNT_4W  ((2048*1024)-2)
#define BLOC_COUNT_8W  ((1024*1024)-1)
#define BLOC_COUNT_16W  ((512*1024)-1)

void init_tinyap_alloc();
void term_tinyap_alloc();

void*_tinyap_alloc_4w();
void _tinyap_free_4w(void*);

void*_tinyap_alloc_8w();
void _tinyap_free_8w(void*);

void*_tinyap_alloc_16w();
void _tinyap_free_16w(void*);

#define _tinyap_alloc(__type) \
	(	sizeof(__type)<=(4*sizeof(void*))?\
	 		(__type*)_tinyap_alloc_4w():\
			sizeof(__type)<=(8*sizeof(void*))?\
				(__type*)_tinyap_alloc_8w():\
				sizeof(__type)<=16*sizeof(void*)?\
					(__type*)_tinyap_alloc_16w():\
					(__type*)malloc(sizeof(__type))\
	)
				
#define _tinyap_free(__type,__p) \
	sizeof(__type)<=(4*sizeof(void*))?\
		_tinyap_free_4w(__p):\
		sizeof(__type)<=(8*sizeof(void*))?\
			_tinyap_free_8w(__p):\
			sizeof(__type)<=(16*sizeof(void*))?\
				_tinyap_free_16w(__p):\
				free(__p)





#endif

