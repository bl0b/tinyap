#ifndef _TINYAP_LR_BASE_H_
#define _TINYAP_LR_BASE_H_


#include <ext/hash_map>
#include <set>
#include <map>
#include "hashtab.h"
#include <string>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <list>

#include <iostream>

namespace ext = __gnu_cxx;

extern "C" {
#include "tinyap_alloc.h"
#include "string_registry.h"
#include "token_utils.h"
#include "trie.h"
}

#include "ast.h"

namespace lr {
	template <class C>
		struct ptr_less {
			bool operator()(const C*a, const C*b) const {
				return *a < *b;
			}
	};

	template <class C>
		struct ptr_eq {
			bool operator()(const C*a, const C*b) const {
				return a == b;
			}
	};

	struct hash_an {
		size_t operator()(const ast_node_t x) const {
			return (size_t)x;
		}
	};
}

extern "C" {
	typedef unsigned long counter;
	extern counter tinyap_allocs;
	extern counter tinyap_reallocs;
	extern counter tinyap_frees;
	extern counter tinyap_ram_size;
	extern counter gss_allocs;
	extern counter gss_reallocs;
	extern counter gss_frees;
	extern counter gss_ram_size;
	extern counter gss_shifts;
	extern counter gss_reduces;
	extern counter states_count;
}

namespace grammar {
	class Grammar;

	namespace item {
		/*template <class C> class iterator;*/
		class iterator;
		class base;

		extern ext::hash_map<
					const ast_node_t,
					grammar::item::base*,
					lr::hash_an, lr::ptr_eq<_ast_node_t>
				> registry;

		class registry_t : public std::vector<base*> {
			public:
				registry_t() : std::vector<base*>() {}
				static registry_t& instance() {
					static registry_t _;
					return _;
				}
				~registry_t();
		};

		template<class I> I* gc(I* x) {
			registry_t::instance().push_back(x);
			return x;
		}

		namespace token {
			class Re;
			class Str;
			class T;
			class Comment;
			class Nt;
			class Bow;
			class AddToBag;
			class Epsilon;
			class Eof;
		}
		namespace combination {
			class Rep01;
			class Rep0N;
			class Rep1N;
			class Prefix;
			class Postfix;
			class Seq;
			class RawSeq;
			class Alt;
		}
	}
	namespace rule {
		class base;
		class Operator;
		class Transient;
		class Prefix;
		class Postfix;
	}
	class Grammar;

	namespace visitors {
		class visitor {
			public:
				virtual void visit(item::token::Str*) = 0;
				virtual void visit(item::token::Re*) = 0;
				virtual void visit(item::token::Epsilon*) = 0;
				virtual void visit(item::token::Eof*) = 0;
				virtual void visit(item::token::Comment*) = 0;
				virtual void visit(item::token::T*) = 0;
				virtual void visit(item::token::Nt*) = 0;
				virtual void visit(item::token::Bow*) = 0;
				virtual void visit(item::token::AddToBag*) = 0;

				virtual void visit(item::combination::Rep01*) = 0;
				virtual void visit(item::combination::Rep0N*) = 0;
				virtual void visit(item::combination::Rep1N*) = 0;
				virtual void visit(item::combination::Prefix*) = 0;
				virtual void visit(item::combination::Postfix*) = 0;
				virtual void visit(item::combination::Seq*) = 0;
				virtual void visit(item::combination::RawSeq*) = 0;
				virtual void visit(item::combination::Alt*) = 0;

				virtual void visit(rule::Transient*) = 0;
				virtual void visit(rule::Operator*) = 0;
				virtual void visit(rule::Prefix*) = 0;
				virtual void visit(rule::Postfix*) = 0;

				virtual void visit(Grammar*) = 0;
		};

		template <typename O> class evaluator;
		class debugger;
		class filter;
		class token_filter;
		class item_rewriter;
		class rmember_rewriter;
	}

	/*typedef item::iterator<item::base> item_iterator;*/
	/*typedef item::iterator<rule::base> rule_iterator;*/
}



namespace lr0 {
	struct transition;
	class state;
	class gss;
	class automaton;
}


#endif

