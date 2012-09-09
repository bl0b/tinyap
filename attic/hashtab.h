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

#ifndef _HASH_H
#define _HASH_H

#include <stdlib.h>
#include "tinyap_alloc.h"

#define HASH_SIZE 137

typedef struct _htab_entry_struct *htab_entry_t;

typedef struct _htab_iterator *htab_iterator_t;

typedef struct _hashtable* hashtab_t;

typedef void* hash_elem;
typedef void* hash_key;

typedef size_t (*hash_func)(hash_key);
typedef int (*compare_func)(hash_key,hash_key);

struct _htab_entry_struct {
	hash_key key;
	hash_elem e;
	htab_entry_t next;
	size_t __pad;
};



struct _hashtable {
	htab_entry_t table[HASH_SIZE];
	hash_func hash;
	compare_func keycmp;
};



struct _htab_iterator {
	hashtab_t tab;
	size_t row;
	struct _htab_entry_struct*entry;
};

#define HASH_NOVAL ((hash_elem)0)


size_t default_hash_func(const char*);




static inline int hi_incr(htab_iterator_t hi) {
	if(hi->entry&&hi->entry->next)
		hi->entry=hi->entry->next;
	do {
		++hi->row;
		if(hi->row>=HASH_SIZE) {
			hi->entry=NULL;
			return 0;
		}
	} while(!hi->tab->table[hi->row]);
	hi->entry = hi->tab->table[hi->row];
	return 1;
}


static inline void hi_init(htab_iterator_t hi,hashtab_t t) {
	hi->tab=t;
	hi->row=(size_t)-1;
	hi->entry=NULL;
	hi_incr(hi);
}


static inline int hi_is_valid(htab_iterator_t hi) { return hi->entry!=NULL; }

static inline hash_key hi_key(htab_iterator_t hi) { return hi->entry->key; }

static inline hash_elem hi_value(htab_iterator_t hi) { return hi->entry->e; }

static inline htab_entry_t hi_entry(htab_iterator_t hi) { return hi->entry; }










static inline void init_hashtab(hashtab_t tab,hash_func hash,compare_func cmp) {
	int i;
	for(i=0;i<HASH_SIZE;i++)
		tab->table[i]=(htab_entry_t) HASH_NOVAL;
	tab->hash=hash;
	tab->keycmp=cmp;
}



static inline void hash_addelem(hashtab_t tab,hash_key key,hash_elem elem) {
	size_t i=tab->hash(key);
	/*htab_entry_t e=(htab_entry_t)malloc(sizeof(struct _htab_entry_struct));*/
	htab_entry_t e=tinyap_alloc(struct _htab_entry_struct);
	if(!e) return;
	e->next=tab->table[i];
	e->key=key;
	e->e=elem;
	tab->table[i]=e;
}



static inline htab_entry_t hash_find_e(hashtab_t tab,hash_key key) {
	int i=tab->hash(key);
	htab_entry_t s=tab->table[i];
/* 	TRACE(&quot;      HASH_FINDELEM [%X]`%s' hashed as %u [%X]\n&quot;,key,key,i,s); */
	if(s) while(tab->keycmp(s->key,key)&&(s=s->next));
	return s;
}

static inline hash_elem hash_find(hashtab_t tab,hash_key key) {
	htab_entry_t s;
/* 	TRACE(&quot;   calling hash_find with [%X]`%s'\n&quot;,key,key); */
	s=hash_find_e(tab,key);
	if(s) return s->e;
	return HASH_NOVAL;
}


static inline void hash_delelem(hashtab_t tab,hash_key key) {
	htab_entry_t p=NULL;
	size_t i=tab->hash(key);
/* 	TRACE(&quot;CALLING HASH_DELELEM !!\n&quot;); */
	htab_entry_t s=tab->table[i];
	while(s&&tab->keycmp(s->key,key)) {
		p=s;
		s=s->next;
	}
	if(s) {
		if(p) p->next=s->next;
		else tab->table[i]=s->next;
		/*free_node((nep_mem_node*)s,1);*/
		/*free(s);*/
		tinyap_free(struct _htab_entry_struct, s);
	}
}

static inline void clean_hashtab(hashtab_t tab,void(*callback)(htab_entry_t)) {
	int i;
	htab_entry_t s,p;
	for(i=0;i<HASH_SIZE;i++) {
		s=tab->table[i];
		while(s) {
			p=s;
			s=s->next;
			/*free_node((nep_mem_node*)p,1);*/
			if(callback) callback(p);
			tinyap_free(struct _htab_entry_struct, p);
			/*free(p);*/
		}
		tab->table[i]=(htab_entry_t)HASH_NOVAL;
	}
}

#endif

