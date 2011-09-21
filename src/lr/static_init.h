#ifndef __TINYAP_STATIC_INIT_H__
#define __TINYAP_STATIC_INIT_H__

#include "lr_base.h"
#include "lr_grammar.h"
#include "trie.h"

extern "C" {


extern void tinyap_init();
extern void tinyap_terminate();


typedef std::pair<const char*, size_t> atom_key;
typedef std::pair<ast_node_t, ast_node_t> pair_key;

struct comp_atom {
	bool operator()(const atom_key&a, const atom_key&b) const {
		/*return !strcmp(a.first, b.first) && a.second==b.second;*/
		return a.first==b.first && a.second==b.second;
	}
};

struct hash_atom {
	/*ext::hash<const char*> hs;*/
	/*ext::hash<size_t> ho;*/
	size_t operator()(const atom_key&a) const {
		/*return hs(a.first)^ho(a.second);*/
		return ((size_t)a.first)+((((size_t)a.first)<<16)^((size_t)a.second));
	}
};

struct comp_pair {
	bool operator()(const pair_key&a, const pair_key&b) const {
		return a.first==b.first && a.second==b.second;
	}
};

struct hash_pair {
	ext::hash<ast_node_t> h;
	size_t operator()(const pair_key&a) const {
		return ((size_t)a.first)+((((size_t)a.first)<<16)^((size_t)a.second));
	}
};
typedef ext::hash_map<atom_key, ast_node_t, hash_atom, comp_atom> atom_registry_t;
typedef ext::hash_map<pair_key, ast_node_t, hash_pair, comp_pair> pair_registry_t;


struct tinyap_static_init {
	std::set<ast_node_t> still_has_refs;
	atom_registry_t atom_registry;
	pair_registry_t pair_registry;
	ext::hash_map<const ast_node_t, grammar::item::base*, lr::hash_an, lr::ptr_eq<_ast_node_t> > grammar_registry;
	ext::hash_map<const char*, grammar::item::token::Nt*> nt_registry;
	ext::hash_map<const char*, trie_t> trie_registry;
	tinyap_static_init() { tinyap_init(); }
	~tinyap_static_init() { tinyap_terminate(); }
};

}

extern struct tinyap_static_init _static_init;

#endif

