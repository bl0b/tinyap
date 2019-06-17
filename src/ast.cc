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
#include <iostream>
#include <iomanip>
#include <set>
namespace ext = __gnu_cxx;

#include "ast.h"
#include "tinyap_alloc.h"
#include "string_registry.h"

#include "static_init.h"

#include "registry.h"

extern "C" {
    /*ast_node_t PRODUCTION_OK_BUT_EMPTY = (union _ast_node_t[]){{ {ast_Nil, 0, 0, 0} }};*/

    volatile int depth=0;

    volatile int _node_alloc_count=0;
    volatile int _node_dealloc_count=0;
    volatile int delete_node_count=0;
    volatile int newAtom_count=0;
    volatile int newPair_count=0;

    volatile ast_node_t node_pool = NULL;
}

/*namespace std {*/
    /*template <>*/
    /*struct hash<ast_node_t> {*/
        /*std::hash<unsigned int> hui;*/
        /*std::hash<void*> hp;*/
        /*size_t operator () (const ast_node_t& a) const*/
        /*{*/
            /*return a ? (1 + a->type + (ptrdiff_t)a->raw.p1) * ((ptrdiff_t)a->raw.p2 + a->pos.offset + 1) : 0;*/
        /*}*/
    /*};*/
/**/
    /*template <>*/
    /*struct equal_to<ast_node_t> {*/
        /*bool operator() (const ast_node_t& a, const ast_node_t& b) const*/
        /*{*/
            /*return !(a->type != b->type || a->pos.offset != b->pos.offset || a->raw.p1 != b->raw.p1 || (a->type == ast_Pair && b->type == ast_Pair && a->raw.p2 != b->raw.p2));*/
        /*}*/
    /*};*/
/*}*/

extern "C" ast_node_t node_alloca();
extern "C" void node_dealloc(ast_node_t);


struct ast_node_registry_type {
    std::unordered_map<const char*, std::unordered_map<int, ast_node_t>> m_atoms;
    std::unordered_map<ast_node_t, std::unordered_map<ast_node_t, ast_node_t>> m_pairs;
    std::unordered_map<ast_node_t, size_t> m_refs;

    static inline
    ast_node_t make_nil()
    {
        ast_node_t _ = node_alloca();
        _->type = ast_Nil;
        return _;
    }

    inline
    ast_node_t make_atom(char* str, int ofs)
    {
        ++newAtom_count;
        str = regstr(str);
        ast_node_t& ret = m_atoms[str][ofs];
        if (!ret) {
            ret = node_alloca();
            ret->type = ast_Atom;
            ret->atom._str = str;
            ret->pos.offset = ofs;
            m_refs[ret] = 0;
        }
        return ret;
    }

    inline
    ast_node_t make_pair(ast_node_t car, ast_node_t cdr)
    {
        ++newPair_count;
        ast_node_t& ret = m_pairs[car][cdr];
        if (!ret) {
            ret = node_alloca();
            ret->type = ast_Pair;
            ret->pair._car = car;
            ref(car);
            ret->pair._cdr = cdr;
            ref(cdr);
            m_refs[ret] = 0;
        }
        return ret;
    }

    inline
    ast_node_t ref(ast_node_t a) { if (a && a != PRODUCTION_OK_BUT_EMPTY) { m_refs[a]++; } return a; }

    inline
    void unref(ast_node_t a)
    {
        if (!a || a == PRODUCTION_OK_BUT_EMPTY) { return; }
        auto it = m_refs.find(a);
        if (it == m_refs.end() || it->second == 0) {
            return;
            std::cout << "WARNING trying to unref a node that has no ref: " << a << std::endl;
        }
        ++delete_node_count;
        if (it->second <= 1) {
            if (a->type == ast_Atom) {
                m_atoms[a->atom._str].erase(a->pos.offset);
                if (!m_atoms[a->atom._str].size()) {
                    m_atoms.erase(a->atom._str);
                }
                unregstr(a->atom._str);
            } else if (a->type == ast_Pair) {
                m_pairs[a->pair._car].erase(a->pair._cdr);
                if (!m_pairs[a->pair._car].size()) {
                    m_pairs.erase(a->pair._car);
                }
                unref(Car(a));
                unref(Cdr(a));
            }
            node_dealloc(a);
            m_refs.erase(it);
        } else {
            m_refs[a]--;
        }
    }

    void clear()
    {
        m_atoms.clear();
        m_pairs.clear();
        for (const auto& kv: m_refs) {
            node_dealloc(kv.first);
        }
        m_refs.clear();
    }

    size_t ref_count(ast_node_t a) const
    {
        if (!a) { return 0; }
        auto i = m_refs.find(a);
        return i == m_refs.end() ? 0 : i->second;
    }

    void dump(std::ostream& os) const
    {
        os << "Registry:" << std::endl;
        for (const auto& kv: m_refs) { os << " * " << kv.first << "   #" << kv.second << std::endl; }
    }

    size_t size() const { return m_refs.size(); }
};


struct node_registry_alloc {
    ast_node_t own(ast_node_t a)
    {
        std::cout << "alloc one ast node" << std::endl;
        ast_node_t ret = node_alloca();
        memcpy(ret, a, sizeof(_ast_node_t));
        return ret;
    }

    void disown(ast_node_t n)
    {
        std::cout << "dealloc one ast node" << std::endl;
        node_dealloc(n);
    }
};


/*typedef registry<ast_node_t, node_registry_alloc> ast_node_registry_type;*/

static ast_node_registry_type ast_node_registry;

ast_node_t ref(ast_node_t a)
{
    return ast_node_registry.ref(a);
    /*if (!a) { return NULL; }*/
    /*ast_node_registry.ref(a);*/
    /*return a;*/
}

void unref(ast_node_t a)
{
    return ast_node_registry.unref(a);
    /*if (a) {*/
        /*ast_node_registry.unref(a);*/
    /*}*/
}

size_t ref_count(ast_node_t a)
{
    return ast_node_registry.ref_count(a);
    /*return a ? ast_node_registry.ref_count(a) : 0;*/
}


extern "C" {
#include "config.h"
#include <regex.h>
#include "stack.h"


#define _node(_tag,_contents) Cons(Atom(_tag),(_contents))

/*static tinyap_stack_t node_stack;*/


void node_pool_init() {
	/*node_stack = new_stack();*/
    ast_node_registry.clear();
}


size_t node_pool_size() {
#if 0
	size_t ret = 0;
	ast_node_t n = node_pool;
	while(n) {
		ret += 1;
		n = n->pool.next;
	}
	return ret;
#endif
	return ast_node_registry.size();
}

ast_node_t node_alloca() {
	++_node_alloc_count;
#if 0
	ast_node_t ret = node_pool;
	if(!ret) {
		ret = tinyap_alloc(union _ast_node_t);
		_node_alloc_count+=1;
		/*push(node_stack,ret);*/
	} else {
		node_pool = node_pool->pool.next;
	}
	return ret;
#else
	return (ast_node_t) _alloc(_select_alloca(sizeof(union _ast_node_t)));
#endif
}

void node_dealloc(ast_node_t node) {
	++_node_dealloc_count;
#if 0
	if(node&&node->type!=ast_Pool) {
		node->type = ast_Pool;
		node->pool.next = node_pool;
		node_pool = node;
	}
	/*(void) (node&&node->type!=ast_Pool ? node->type = ast_Pool, node->pool.next = node_pool, node_pool = node : 0);*/
#else
	tinyap_free(union _ast_node_t, node);
#endif
}

void delete_node(ast_node_t);

void node_pool_flush() {
#if 0
	ast_node_t n;

	tinyap_stack_t tmp_stack = new_stack();

/*
	while(node_pool) {
		n = node_pool;
		node_pool = node_pool->pool.next;
		free(n);
		_node_alloc_count-=1;
	}
*/
	while(not_empty(node_stack)) {
		//node_dealloc(pop(ast_node_t,node_stack));
		n = pop(ast_node_t,node_stack);
		if(n->type!=ast_Pool) {
			delete_node(n);
		}
		push(tmp_stack,n);
		//free(n);
		_node_alloc_count-=1;
	}

	/* freeing slogw things down */
	while(not_empty(tmp_stack)) {
		tinyap_free(union _ast_node_t, _pop(tmp_stack));
	}

	free_stack(tmp_stack);

//	printf("after node pool flush, %i nodes remain\n",_node_alloc_count);
#endif
}


void node_pool_term() {
	/*printf("node_pool_term\n"); fflush(stdout);*/
	/*node_pool_flush();*/
	/*free_stack(node_stack);*/
	/*node_stack = NULL;*/
}


static atom_registry_t& atom_registry = _static_init.atom_registry;

static pair_registry_t& pair_registry = _static_init.pair_registry;


ast_node_t newAtom_impl(const char*data, int offset) {
    return ast_node_registry.make_atom(const_cast<char*>(data), offset);
    /*std::cout << "calling newAtom " << data << ' ' << offset << std::endl;*/
	/*char* reg = regstr(data);*/
    /*newAtom_count++;*/
    /*union _ast_node_t n {ast_Atom, (int) offset, reg, NULL, NULL};*/
    /*ast_node_t ret = const_cast<ast_node_t>(ast_node_registry.find(&n));*/
    /*std::cout << "new atom @str=\"" << data << "\"/\"" << reg << "\" " << ((void*) ret) << ' ' << ret << std::endl;*/
    /*return ret;*/
}


ast_node_t newPair_impl(const ast_node_t a,const ast_node_t d) {
    return ast_node_registry.make_pair(const_cast<ast_node_t>(a), const_cast<ast_node_t>(d));
    /*std::cout << "calling newPair " << a << ' ' << d << std::endl;*/
    /*newPair_count++;*/
    /*if (a) {*/
        /*std::cout << "[newPair] ref car" << std::endl;*/
        /*ref(a);*/
        /*std::cout << "--" << std::endl;*/
    /*}*/
    /*if (d) {*/
        /*std::cout << "[newPair] ref cdr" << std::endl;*/
        /*ref(d);*/
        /*std::cout << "--" << std::endl;*/
    /*}*/
    /*union _ast_node_t n {ast_Pair, 0, a, d, NULL};*/
    /*std::cout << "[newPair] create/find" << std::endl;*/
    /*ast_node_t ret = const_cast<ast_node_t>(ast_node_registry.find(&n));*/
    /*std::cout << "--" << std::endl;*/
    /*std::cout << "new pair " << ret << " : " << a << ", " << d << std::endl;*/
    /*return ret;*/
}



#include "static_init.h"
static std::set<ast_node_t>& still_has_refs = _static_init.still_has_refs;

void delete_node(ast_node_t n) {
    ast_node_registry.unref(n);
	/*static int rec_lvl=0;*/
	/*++delete_node_count;*/
	/*if(!(n && n != PRODUCTION_OK_BUT_EMPTY)) return;*/
    /*if (n->type == ast_Pair) {*/
        /*if (Car(n)) {*/
            /*delete_node(Car(n));*/
        /*}*/
        /*if (Cdr(n)) {*/
            /*delete_node(Cdr(n));*/
        /*}*/
    /*}*/
    /*unref(n);*/
}

void dump_ast_reg() { ast_node_registry.dump(std::cout); }

ast_node_t PRODUCTION_OK_BUT_EMPTY = ast_node_registry.make_nil();
}/* extern "C" */

