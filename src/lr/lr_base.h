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

namespace grammar {
	class Grammar;

	namespace item {
		/*template <class C> class iterator;*/
		class iterator;
		class base;

		namespace token {
			class Re;
			class Str;
			class T;
			class Comment;
			class Nt;
			class Bow;
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

