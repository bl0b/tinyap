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

#include "walker.h"

#include "hashtab.h"
#include "tinyap.h"
#include <stdlib.h>
#include <dlfcn.h>

typedef void* (*pilot_init)(void*);

struct {
	hashtab_t cache;
	void* dl_self;
} pilot_mgr;


#define RORSZ 3
#define HASHBITSZ 16


/*! \brief hashes a string
 * \return the hash value
 */
static size_t hash_str(hash_key k) {
//	size_t ror;
	const char* str = (const char*)k;
	size_t ret=*str %HASH_SIZE;
	if(*str) {
		str+=1;
		while(*str) {
/*		ror = ret >> (HASHBITSZ-RORSZ);
		ret<<=RORSZ;
		ret&=(1<<HASHBITSZ)-1;
		ret|=ror;
		ret+=*str;
*/
			ret *= (size_t)(*str);
			ret %= HASH_SIZE;
			str += 1;
		}
	}
	return ret;
}

/*! \brief compare two string keys
 * \return strcmp
 */
static int cmp_str(hash_key a, hash_key b) {
	return strcmp((const char*)a, (const char*)b);
}

volatile int pilot_manager_is_init=0;

void release_pilot(pilot_cache_elem_t pce);

/*! \brief terminate the pilot manager
 */
void term_pilot_manager() {
	struct _htab_iterator hi;
//	hash_key k;
	if(!pilot_manager_is_init) {
		return;
	}
	pilot_manager_is_init=0;
//	printf(" @@ term_pilot_manager\n");
	dlclose(pilot_mgr.dl_self);

	hi_init(&hi,pilot_mgr.cache);
	while(hi_is_valid(&hi)) {
//		k = hi_key(&hi);
		release_pilot((pilot_cache_elem_t)hi_value(&hi));
		hi_incr(&hi);
	}

	clean_hashtab(pilot_mgr.cache,NULL);
	free(pilot_mgr.cache);
}


/*! \brief initialize the pilot manager
 */
void init_pilot_manager() {
	if(!pilot_manager_is_init) {
		pilot_mgr.cache = (hashtab_t) malloc(sizeof(struct _hashtable));
		init_hashtab(pilot_mgr.cache, hash_str, cmp_str);
		pilot_mgr.dl_self = dlopen(NULL, RTLD_LAZY);
		atexit(term_pilot_manager);
		pilot_manager_is_init=1;
	}
}


/*! \brief builds a full pilot method name using a pilot name and a method name
 * \return the full symbol
 */
char* make_mthd(const char*p, const char*m) {
	/* FIXME : static buffer => overflow */
	static char ret[1024];
	//char* ret = (char*) malloc(strlen(p)+strlen(m)+6);
	sprintf(ret,"ape_%s_%s",p,m);
	return ret;
}


/*! \brief instantiate a new pilot from cached descr using init data context
 * \return new pilot handle
 */
static pilot_t new_pilot_from(pilot_cache_elem_t pce,void* init_data) {
	pilot_t ret;
//	printf("new_pilot_from init_data=%p\n",init_data);
	ret = (pilot_t) malloc(sizeof(struct _pilot_t));
	ret->p_type = pce;
	ret->data = pce->init(init_data);

	return ret;
}


void free_pilot(pilot_t p) {
	if(p->p_type->free) {
		p->p_type->free(p->data);
	}
	free(p);
}

pilot_cache_elem_t new_pilot_cache_elem(const char* p_name) {
	pilot_cache_elem_t ret;
	void* handle;
	void* sym;
	char*tmp = (char*)malloc(strlen(p_name)+11);
	sprintf(tmp,"libape_%s.so",p_name);
//	strcpy(tmp,p_name);
	/* try to open ape_[p_name].so */
	handle = dlopen(tmp, RTLD_LAZY);
	/* otherwise use main program */
	if(handle==NULL) {
		handle = pilot_mgr.dl_self;
	}
	free(tmp);

	/* try to fetch the ctor for the pilot */
	sym = dlsym(handle, make_mthd(p_name,"init"));
	if(!sym) {
		fprintf(stderr,"Pilot `%s' retrieval went wrong : %s\n",p_name,dlerror());
		return NULL;
	}
	ret = (pilot_cache_elem_t)malloc(sizeof(struct _pilot_cache_elem_t));
	memset(ret,0,sizeof(struct _pilot_cache_elem_t));

	ret->name = strdup(p_name);
	ret->dl_handle = handle;
	ret->init = (void*(*)(void*)) sym;
	ret->free = (void(*)(pilot_t)) dlsym(handle, make_mthd(p_name,"free"));
	ret->result = (void*(*)(pilot_t)) dlsym(handle, make_mthd(p_name,"result"));
	ret->defaultMethod = (node_visit_method) dlsym(handle, make_mthd(p_name,"default"));

	ret->methods = (hashtab_t) malloc(sizeof(struct _hashtable));

	init_hashtab(ret->methods,hash_str,cmp_str);

//	printf("nouveau pilote connu : %s\n",p_name);
	
	return ret;
}

pilot_t new_pilot(const char* p_name, void*init_data) {
	/* seek for entry in htab */
	pilot_cache_elem_t pce = (pilot_cache_elem_t) hash_find(pilot_mgr.cache, (hash_key)p_name);
//	printf("new_pilot init_data=%p\n",init_data);
	if(!pce) {
		pce = new_pilot_cache_elem(p_name);
		hash_addelem(pilot_mgr.cache,(hash_key)pce->name,pce);
	}
	if(pce) {
		return new_pilot_from(pce,init_data);
	} else {
		return NULL;
	}
}

void free_key(htab_entry_t e) {
	free(e->key);
}

void release_pilot(pilot_cache_elem_t pce) {
	/* TODO */
	if(pce->dl_handle!=pilot_mgr.dl_self) {
		dlclose(pce->dl_handle);
	}

	free((char*)pce->name);

	clean_hashtab(pce->methods,free_key);
	free(pce->methods);
	free(pce);
}

void* get_pilot_data(pilot_t pilot) {
	return pilot->data;
}


node_visit_method get_visit_method(pilot_t p, const char* nodetype) {
	htab_entry_t e = hash_find_e(p->p_type->methods, (hash_key)nodetype);

	if(e) {
//		printf("visit method is cached.\n");
		return (node_visit_method)e->e;
	} else {
		char* tmp = make_mthd(p->p_type->name, nodetype);
		void* sym = dlsym(p->p_type->dl_handle, tmp);
//		printf("visit method %s ain't cached.\n",tmp);
		//free(tmp);
		if(sym==NULL) {
			sym = (void*)p->p_type->defaultMethod;
		}
		hash_addelem(p->p_type->methods,(hash_key)strdup(nodetype),sym);
		return sym;
	}
}

WalkDirection do_visit(pilot_t p, wast_t node) {
	node_visit_method v;
	if(!node) {
		fprintf(stderr,"Warning : can't visit null node.\n");
		return Done;
	}
	v = get_visit_method(p, wa_op(node));
//	printf("do_visit with %s on %s\n",p->p_type->name,wa_op(node));
	return v(node,p->data);
}


