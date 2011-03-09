#ifndef _TINYAP_LR_H_
#define _TINYAP_LR_H_

#include "parse_context.h"
#include "token_utils.h"
#include "lr_base.h"
#include "lr_grammar.h"
#include "lr_visitors.h"




namespace lr {
	class item {
		private:
			const grammar::rule::base* rule_;
			grammar::item::iterator dot_;
		public:
			/*item(const grammar::rule::base* r, grammar::item::iterator& rmb)*/
				/*: rule_(r), dot_(grammar::item::iterator::create(*rmb))*/
			/*{}*/
			item(const grammar::rule::base* r, grammar::item::iterator rmb)
				: rule_(r), dot_(grammar::item::iterator::create(*rmb))
			{}
			bool at_start() const { return dot_.at_start(); }
			bool at_end() const { return dot_.at_end(); }
			item& operator ++() {
				++dot_;
				return *this;
			}
			item& operator --() {
				--dot_;
				return *this;
			}
			item next() const {
				item i = *this;
				++i;
				return i;
			}
			item prev() const {
				item i = *this;
				--i;
				return i;
			}
			const grammar::rule::base* rule() const { return rule_; }
			const grammar::item::iterator& dot() const { return dot_; }
			const grammar::item::base* operator*() const { return *dot_; }
			bool operator ==(const item& i) const {
				return rule_==i.rule_ && dot_==i.dot_;
			}
			bool operator !=(const item& i) const {
				return rule_!=i.rule_ || dot_!=i.dot_;
			}
			bool operator<(const item&i) const {
				if(rule_==i.rule_) {
					return dot_ < i.dot_;
				}
				return rule_ < i.rule_;
			}
	};

	static inline std::ostream& operator<<(std::ostream&o, item&i) {
		grammar::visitors::debugger d(o);
		item tmp = i;
		while(!tmp.at_start()) { --tmp; }
		o << '[' << i.rule()->tag();
		/*o << ' '; d << *i;*/
		o << " ->";
		while(tmp!=i) {
			if(*tmp) { o << ' '; d << *tmp; }
			++tmp;
		}
		/*std::cerr << "(tmp==i?"<<(tmp==i)<<", *tmp?"<<(!!*tmp)<<')';*/
		o << " ⋅";
		while(!tmp.at_end()) {
			if(*tmp) { o << ' '; d << *tmp; }
			++tmp;
		}
		if(*tmp) { d << *tmp; }
		o << ']';
		return o;
	}



	typedef std::set<item> item_set;

	
	
	static inline std::ostream& operator<<(std::ostream&o, item_set&S) {
		lr::item_set::iterator i=S.begin(), j=S.end();
		if(i==j) {
			o << "<empty>";
		}
		for(;i!=j;++i) {
			lr::item tmp = *i;
			o << " :: " << tmp << std::endl;
		}
		return o;
	}

	static inline std::ostream& operator<<(std::ostream&o, item_set*S) {
		if(S) {
			o << *S;
		} else {
			o << "<null>";
		}
		return o;
	}


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



	
	template <> struct ptr_less<grammar::item::base> {
		bool operator()(const grammar::item::base* a,
						const grammar::item::base* b) const {
			return a->is_less(b);
		}
	};


	
	typedef std::set<grammar::item::base*, ptr_less<grammar::item::base> > token_set;


	
	struct state;

	struct hash_gitb {
		size_t operator()(const grammar::item::base* x) const {
			return (size_t)x;
		}
	};
	
	typedef ext::hash_map<
			const grammar::item::base*,
			state*,
			hash_gitb,
			ptr_eq<grammar::item::base> > follow_set;

	typedef ext::hash_map<
			const grammar::item::base*,
			item_set,
			hash_gitb,
			ptr_eq<grammar::item::base> > follow_set_builder;



	struct state {
		item_set items;
		follow_set transitions;
		state(item_set& s) : items(s), transitions() {}
		bool operator<(const state& b) const {
			return items < b.items;
		}
	};


	typedef std::set<state*, ptr_less<state> > state_set;


	class gss {
		public:
			struct node {
				state* s;
				ast_node_t prod;

				node(state* s_, ast_node_t prod_, node* pred)
					: s(s_), prod(prod_), preds(), succs()
				{
					preds.push_back(pred);
				}

				private:
					friend class gss;
					std::list<node*> preds;
					std::list<node*> succs;
					bool remove_succ(node*succ) { succs.remove(succ); return !succs.size(); }
					void add_succ(node*succ) { succs.push_front(succ); }
					bool remove_pred(node*pred) { preds.remove(pred); return !preds.size(); }
					void add_pred(node*pred) { preds.push_front(pred); }
			};
	};

	struct process {
		gss::node* gss_node;
		unsigned long offset;
	};

	/*
	 * epsilon-closure(T) :
push all states of T onto stack;
initialize E- closure(T) to T;
while ( stack is not empty ) {
pop t, the top element, off stack;
for ( each state u with an edge from t to u labeled e )
if ( u is not in e-closure(T) ) {
add u to e-closure(T);
push u onto stack
	 */

	class automaton {
		public:
			struct token {
				typedef grammar::item::token::Nt Nt;
			};
			struct rule {
				typedef grammar::rule::base base;
			};
			grammar::Grammar* G;
			std::set<state*> S;
			state* S0;
			state_set states;

			automaton(grammar::Grammar* _)
				: G(_), S(), states()
			{
				items();
			}

			void closure(item_set& I, item_set& C) const {
				item_set::iterator  i, j;
				C = I;
				/*std::cout << "C starts with " << C.size() << " elements" << std::endl;*/
				std::vector<item> stack(I.begin(), I.end());
				while(stack.size()>0) {
					item i = stack.back();
					stack.pop_back();
					if(i.at_end()) {
						continue;
					}
					const token::Nt* nt = dynamic_cast<const token::Nt*>(*i);
					if(nt) {
						/*std::cout << "have NT " << nt->tag() << std::endl;*/
						grammar::Grammar::iterator S = G->find(nt->tag());
						rule::base* r = (S==G->end()) ? NULL : S->second;
						if(!r) {
							std::cout << "couldn't find rule !" << std::endl;
							continue;
						}
						/* and we add an iterator to each variant of the rule */
						grammar::item::iterator ri = grammar::item::iterator::create(r);
						while(!ri.at_end()) {
							item c(r, ri);
							if(C.find(c)==C.end()) {
								C.insert(c);
								stack.push_back(c);
								/*std::cout << "C has now " << C.size() << " elements" << std::endl;*/
							/*} else {*/
								/*std::cout << "C already contains " << c << std::endl;*/
							}
							++ri;
						}
					}
				}
			}

			bool is_initial_item(const item& i) const {
				static const std::string start("_start");
				const grammar::rule::base* R = i.rule();
				return R && start==R->tag();
			}

			void kernel(item_set& I, item_set& K) const {
				item_set::iterator i, j = I.end();
				for(i=I.begin();i!=j;++i) {
					if(is_initial_item(*i)||!(*i).at_start()) {
						K.insert(*i);
					}
				}
			}

			void productible(const item_set&I, item_set& P) const {
				/*grammar::visitors::producer_filter f;*/
				grammar::visitors::token_filter f;
				item_set::iterator i, j = I.end();
				for(i=I.begin();i!=j;++i) {
					if(f((grammar::item::base*)**i)) {
						P.insert(*i);
					}
				}
			}

			/*void transitions(const item_set&I, const transition_map&T) const {*/
			/*}*/

			void first(const item_set& I, token_set& F) const {
				item_set::iterator i, j=I.end();
				grammar::item::token::base* x;
				grammar::visitors::token_filter f;
				for(i=I.begin();i!=j;++i) {
					/* cast to unconstify **i so we can pass it to the filter */
					x = dynamic_cast<grammar::item::token::base*>(f((grammar::item::base*)**i));
					if(x) { F.insert(x); }
				}
			}

			void follow(const item_set& I, follow_set& F) const {
				item_set::iterator i, j=I.end();
				token_set T;
				for(i=I.begin();i!=j;++i) {
					item x = *i;
					item_set f, C; 
					if(!x.at_end()) {
						++x;
						f.insert(x);
						closure(f, C);
						first(C, T);
					}
				}
			}

			std::pair<state*, bool> commit(item_set& s) {
				state* tmp = new state(s);
				std::pair<state_set::iterator, bool> ret = states.insert(tmp);
				if(ret.second!=true) {
					delete tmp;
				}
				return std::pair<state*, bool>(*(ret.first), ret.second);
			}

			state* items_commit(item_set&s, std::vector<state*>& stack) {
				std::pair<state*, bool> ret = commit(s);
				if(ret.second) {
					stack.push_back(ret.first);
					/*std::cout << "  committed new set :" << std::endl << stack.back()->items;*/
				}
				return ret.first;
			}


			void compute_transitions(item_set& items, follow_set_builder& transitions) {
				typedef grammar::item::base item_base;
				item_set prods;
				productible(items, prods);
				item_set::iterator i, j=prods.end();
				for(i=prods.begin();i!=j;++i) {
					const item_base* t = **i;
					if(!t) {
						item x = *i;
						std::cout << "COIN " << x << " " << *x << " " << typeid(**i).name() << std::endl;
						throw "COIN";
					}
					item tmp = (*i).next();
					/*std::cout << "  transiting to " << tmp << std::endl;*/
					std::pair<item_set::iterator, bool> ret = transitions[t].insert(tmp);
					if(!ret.second) {
						item x = *ret.first;
						/*std::cout << "COIN transition pas ajoutée " << x << std::endl;*/
					}
					/*grammar::visitors::debugger d(std::cout);*/
					/*std::cout << "  => transitions[";*/
					/*((item_base*)t)->accept(&d);*/
					/*std::cout << "] = " << transitions[t] << std::endl;*/
				}
			}


			void items() {
				grammar::rule::base* rule = (*G)["_start"];
				grammar::item::iterator iter = grammar::item::iterator::create(rule);
				item_set s0, tmp;
				while(!iter.at_end()) {
					item i0(rule, iter);
					tmp.insert(i0);
					++iter;
				}
				closure(tmp, s0);
				std::vector<state*> stack;
				S0 = items_commit(s0, stack);
				/*std::cout << "initial state is : " << std::endl << S0->items << std::endl;*/
				follow_set_builder FSB;
				while(stack.size()>0) {
					state* S = stack.back();
					stack.pop_back();
					/*std::cout << "Now computing transitions of " << std::endl << S->items << std::endl;*/
					FSB.clear();
					compute_transitions(S->items, FSB);
					follow_set_builder::iterator fi, fj=FSB.end();
					for(fi=FSB.begin();fi!=fj;++fi) {
						item_set tmp;
						closure((*fi).second, tmp);
						S->transitions[(*fi).first] = items_commit(tmp, stack);
						/*std::cout << "finally, retain S->transitions[";*/
						/*grammar::visitors::debugger d;*/
						/*((grammar::item::base*)(*fi).first)->accept(&d);*/
						/*std::cout << "] = " << S->transitions[(*fi).first]->items << std::endl;*/
					}
				}
			}

			void dump_states() const {
				std::cout << "automaton has " << states.size() << " states." << std::endl;
				state_set::iterator i, j = states.end();
				for(i=states.begin();i!=j;++i) {
					std::cout << (*i)->items << std::endl;
				}
			}
	};

}



#endif

