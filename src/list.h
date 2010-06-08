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

#ifndef __TINYAP__LIST_H__
#define __TINYAP__LIST_H__

/*
 * structs defined as macros to allow precise typing of data in nodes (i.e. avoid using void* that could be easily miscast to some type)
 * routines defined as macros to avoir rewriting trivial stuff for each type
 *
 */

#define NODE_P(__type) \
	struct NodeOfType_##__type {\
		struct NodeOfType_##__type *prev,*next;\
		struct ListOfType_##__type*list;\
		__type* data;\
	}

#define NODE(__type) \
	struct NodeOfType_##__type {\
		struct NodeOfType_##__type *prev,*next;\
		struct ListOfType_##__type*list;\
		__type data;\
	}

#define LIST(__type) \
	struct ListOfType_##__type {\
		struct NodeOfType_##__type *head,*tail;\
		long count;\
	}

#define listInit(__l) do { (__l).head=NULL; (__l).tail=NULL; (__l).count=0; } while(0)


#define TypeDefList(__id,__type)\
	typedef LIST(__type) __id;\
	typedef NODE(__type) __id##Node;

#define TypeDefPList(__id,__type)\
	typedef LIST(__type) __id;\
	typedef NODE_P(__type) __id##Node;

TypeDefPList(GenericList,void);

#define listInsertAfter(__a,__n) do {	__n->prev=__a;\
					__n->list=__a->list;\
					if(__a->next) {\
						__n->next=__a->next;\
						__n->next->prev=__n;\
					} else {\
						__n->next=NULL;\
						__n->list->tail=__n;\
					}\
					__a->next=__n;\
					__n->list->count++;\
				} while(0)



#define listInsertBefore(__a,__n) do {	__n->next=__a;\
					__n->list=__a->list;\
					if(__a->prev) {\
						__n->prev=__a->prev;\
						__n->prev->next=__n;\
					} else {\
						__n->prev=NULL;\
						__n->list->head=__n;\
					}\
					__a->prev=__n;\
					__n->list->count++;\
				} while(0)




#define listAddHead(_l,_a)	do {	if((_l).head) {\
						listInsertBefore((_l).head,(_a));\
					} else {\
						(_a)->prev=NULL;\
						(_a)->next=NULL;\
						(_a)->list=&(_l);\
						(_l).head=(_a);\
						(_l).tail=(_a);\
						(_l).count=1;\
					}\
				} while(0)



#define listAddTail(_l,_a)	do {	if((_l).tail) {\
						listInsertAfter((_l).tail,(_a));\
					} else {\
						(_a)->prev=NULL;\
						(_a)->next=NULL;\
						(_a)->list=&(_l);\
						(_l).head=(_a);\
						(_l).tail=(_a);\
						(_l).count++;\
					}\
				} while(0)



#define listRemove(__l,__a)	do {	if((__a)->prev) (__a)->prev->next=(__a)->next;\
					if((__a)->next) (__a)->next->prev=(__a)->prev;\
					if((__l).tail==(__a)) (__l).tail=(__a)->prev;\
					if((__l).head==(__a)) (__l).head=(__a)->next;\
					(__l).count--;\
				} while(0)



#define listHead(__l,__a) 	do { __a=(__l).head; } while(0)
#define listTail(__l,__a) 	do { __a=(__l).tail; } while(0)



#define Prev(__a) do { __a=(__a)->prev; } while(0)
#define Next(__a) do { __a=(__a)->next; } while(0)



#endif

