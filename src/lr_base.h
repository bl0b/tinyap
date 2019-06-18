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

#include "string_registry.h"

extern "C" {
#include "tinyap_alloc.h"
#include "token_utils.h"
#include "trie.h"
}

#include "ast.h"

/*! \brief Automaton, GSS, and a few helper classes */
namespace lr {
	/*! \brief compare two pointed-to data by value (less-than) */
	template <class C>
		struct ptr_less {
			bool operator()(const C*a, const C*b) const {
				return *a < *b;
			}
	};

	/*! \brief compare two pointed-to data by value (equality) */
	template <class C>
		struct ptr_eq {
			bool operator()(const C*a, const C*b) const {
				return a == b;
			}
	};

	/*! \brief hash an ast_node_t (returns identity because ast_node_t are unique */
	struct hash_an {
		size_t operator()(const ast_node_t x) const {
			return (size_t)x;
		}
	};
}

typedef unsigned long counter;
extern "C" {
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

/*! \brief Intermediate representation to easily compute the LR(0) sets and provide helpers during parse (such as token recognition). */
namespace grammar {
	/*! \brief The main class, representing the top-level AST node (Grammar ...rules...). */
	class Grammar;

	/*! \brief all grammar items (tokens, combinations, rules) and helpers (iterators, comparators) fit in here */
	namespace item {
		/*! \brief the item iterator facade */
		class iterator;
		/*! \brief the base for all grammar items */
		class base;

		/*! \brief this class ensures grammar::items are unique when created from an AST node */
		//extern ext::hash_map<
		//			const ast_node_t,
		//			grammar::item::base*,
		//			lr::hash_an, lr::ptr_eq<_ast_node_t>
		//		>& registry;

		/*! \brief A simple container for garbage-collecting, made a singleton.
		 * TODO make this a std::list to speed up registration
		 */
		class registry_t : public std::vector<base*> {
			public:
				registry_t() : std::vector<base*>() {}
				static registry_t& instance() {
					static registry_t _;
					return _;
				}
				~registry_t();
		};

		/*! \brief the garbage-collector implementation */
		template<class I> I* gc(I* x) {
			registry_t::instance().push_back(x);
			return x;
		}

		/*! \brief Grammar tokens reside here. */
		namespace token {
			/*! \brief Regular expression */
			class Re;
			/*! \brief Delimited string */
			class Str;
			/*! \brief String constant (AKA terminal) */
			class T;
			/*! \brief Comment. Unused and actually dismissed in the LR context */
			class Comment;
			/*! \brief Reference to a rule (AKA non-terminal) */
			class Nt;
			/*! \brief Bag of Words : a suffix tree (called a trie as in re-trie-ve) */
			class Bow;
			/*! \brief A regular expression that also inserts the matched string into a trie */
			class AddToBag;
			/*! \brief The infamous "do nothing and succeed" token */
			class Epsilon;
			/*! \brief A remnant of the previous implementation, matches end of file/buffer; slighly useless with the GLR algorithm, but some may like using it. */
			class Eof;
		}
		/*! \brief Combination of grammar items reside here. */
		namespace combination {
			/*! \brief The operator \em "?": the contents may appear once, or not at all. */
			class Rep01;
			/*! \brief The operator \em "*": the contents may appear any number of times, or not at all. */
			class Rep0N;
			/*! \brief The operator \em "+": the contents must appear at least once, and can be repeated any number of times. */
			class Rep1N;
			/*! \brief The construct \em "[ contents ] some_NT": the contents must be followed by a reduction of \em some_NT and will be inserted at the beginning of its contents. */
			class Prefix;
			/*! \brief The construct \em "{ contents } some_NT": the contents must be followed by a reduction of \em some_NT and will be inserted at the end of its contents. */
			class Postfix;
			/*! \brief A sequence of grammar items. */
			class Seq;
			/*! \brief This combination actually behaves like a meta-token: it may contain a sequence of T, Re, Bow, AddToBag or Str that will be recognized without skipping any whitespace character in between them. */
			class RawSeq;
			/*! \brief A collection of alternative elements. */
			class Alt;
		}
	}
	/*! \brief All grammar rule types reside here. */
	namespace rule {
		/*! \brief The base class for all rules. */
		class base;
		/*! \brief The tinyap OperatorRule : reduces \tt (contents) into \tt ((mytag contents)). */
		class Operator;
		/*! \brief The tinyap TransientRule : reduces \tt (contents) into \tt (contents). */
		class Transient;
		/*! \brief The result of the expansion of a Prefix token. */
		class Prefix;
		/*! \brief The result of the expansion of a Postfix token. */
		class Postfix;
	}
	class Grammar;

	/*! \brief Grammar item visitors. */
	namespace visitors {
		/*! \brief Visitor interface. */
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

		/*! \brief Evaluator interface; this is for visitors that return a value (of type \em O). */
		template <typename O> class evaluator;
		/*! \brief Prettyprint an element to some std::ostream. */
		class debugger;
		/*! \brief A filter is an evaluator returning a \em "grammar::item::base*". */
		class filter;
		/*! \brief Filter out anything that is not a token (RawSeq is considered a token). */
		class token_filter;
		/*! \brief perform expansion of complex items. */
		class item_rewriter;
		/*! \brief perform expansion of right member (behaves differently from \tt item_rewriter for few items such as Alt). */
		class rmember_rewriter;
	}

	/*typedef item::iterator<item::base> item_iterator;*/
	/*typedef item::iterator<rule::base> rule_iterator;*/
}


#endif

