#ifndef _HASH_H
#define _HASH_H

#include <stdlib.h>

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




static __inline int hi_incr(htab_iterator_t hi) {
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


static __inline void hi_init(htab_iterator_t hi,hashtab_t t) {
	hi->tab=t;
	hi->row=(size_t)-1;
	hi->entry=NULL;
	hi_incr(hi);
}


static __inline int hi_is_valid(htab_iterator_t hi) { return hi->entry!=NULL; }

static __inline hash_key hi_key(htab_iterator_t hi) { return hi->entry->key; }

static __inline hash_elem hi_value(htab_iterator_t hi) { return hi->entry->e; }

static __inline htab_entry_t hi_entry(htab_iterator_t hi) { return hi->entry; }










static __inline void init_hashtab(hashtab_t tab,hash_func hash,compare_func cmp) {
	int i;
	for(i=0;i<HASH_SIZE;i++)
		tab->table[i]=(htab_entry_t) HASH_NOVAL;
	tab->hash=hash;
	tab->keycmp=cmp;
}



static __inline void hash_addelem(hashtab_t tab,hash_key key,hash_elem elem) {
	size_t i=tab->hash(key);
/* 	TRACE(&quot;HASH_ADDELEM [%X]`%s' [%X] hashed as %u\n&quot;,key,key,elem,i); */
	/*htab_entry e=(htab_entry )alloc_node(1);*/
	htab_entry_t e=(htab_entry_t)malloc(sizeof(struct _htab_entry_struct));
/* 	TRACE(&quot;  %ssuccessfully alloc'ed node\n&quot;,e?&quot;&quot;:&quot;un&quot;); */
	if(!e) return;
	e->next=tab->table[i];
	e->key=key;
	e->e=elem;
	tab->table[i]=e;
}



static __inline htab_entry_t hash_find_e(hashtab_t tab,hash_key key) {
	int i=tab->hash(key);
	htab_entry_t s=tab->table[i];
/* 	TRACE(&quot;      HASH_FINDELEM [%X]`%s' hashed as %u [%X]\n&quot;,key,key,i,s); */
	if(s) while(tab->keycmp(s->key,key)&&(s=s->next));
	return s;
}

static __inline hash_elem hash_find(hashtab_t tab,hash_key key) {
	htab_entry_t s;
/* 	TRACE(&quot;   calling hash_find with [%X]`%s'\n&quot;,key,key); */
	s=hash_find_e(tab,key);
	if(s) return s->e;
	return HASH_NOVAL;
}


static __inline void hash_delelem(hashtab_t tab,hash_key key) {
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
		free(s);
	}
}

static __inline void clean_hashtab(hashtab_t tab,void(*callback)(htab_entry_t)) {
	int i;
	htab_entry_t s,p;
	for(i=0;i<HASH_SIZE;i++) {
		s=tab->table[i];
		while(s) {
			p=s;
			s=s->next;
			/*free_node((nep_mem_node*)p,1);*/
			if(callback) callback(p);
			free(p);
		}
		tab->table[i]=(htab_entry_t)HASH_NOVAL;
	}
}

#endif

