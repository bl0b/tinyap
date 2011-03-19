#ifndef _TINYAP_LR_H_
#define _TINYAP_LR_H_

#include "parse_context.h"
#include "token_utils.h"
#include "lr_base.h"
#include "lr_grammar.h"
#include "lr_visitors.h"

#include <algorithm>
#include <queue>
#include <iomanip>



extern "C" {
ast_node_t  ast_unserialize(const char*input);
const char* ast_serialize_to_string(const ast_node_t ast);
void ast_serialize_to_file(const ast_node_t ast,FILE*f);
}



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
		grammar::visitors::lr_item_debugger d(o);
		item tmp = i;
		while(!tmp.at_start()) { --tmp; }
		o << '[' << i.rule()->tag();
		/*o << ' '; d << *i;*/
		o << " ->";
		if(tmp.at_end()) {
			/* epsilon */
			o << " ⋅ ]";
			return o;
		}
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

	
	
	static inline std::ostream& operator<<(std::ostream&o, const item_set&S) {
		lr::item_set::iterator i=S.begin(), j=S.end();
		if(i==j) {
			o << "<empty>" << std::endl;
		}
		for(;i!=j;++i) {
			lr::item tmp = *i;
			o << ' ' << tmp << std::endl;
		}
		return o;
	}

	static inline std::ostream& operator<<(std::ostream&o, const item_set*S) {
		if(S) {
			o << *S;
		} else {
			o << "<null>";
		}
		return o;
	}


	
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
			ptr_eq<grammar::item::base> > follow_set_text;

	typedef ext::hash_map<
			const char*,
			state*> follow_set_stack;

	typedef ext::hash_map<
			const grammar::item::base*,
			item_set,
			hash_gitb,
			ptr_eq<grammar::item::base> > follow_set_builder;



	struct state {
		int id;
		item_set items;
		struct transitions_ {
			follow_set_text from_text;
			follow_set_stack from_stack;
			transitions_() : from_text(), from_stack() {}
		} transitions;
		item_set reductions;
		state(item_set& s) : id(0), items(s), transitions(), reductions() {}
		bool operator<(const state& b) const {
			return items < b.items;
		}
	};


	static inline std::ostream& operator<<(std::ostream&o, const follow_set_text::value_type& f) {
		grammar::visitors::lr_item_debugger d(o);
		((grammar::item::base*)f.first)->accept(&d);
		o << " => " << f.second->id << std::endl;
		return o;
	}

	static inline std::ostream& operator<<(std::ostream&o, const follow_set_stack::value_type& f) {
		o << f.first << " => " << f.second->id << std::endl;
		return o;
	}

	static inline std::ostream& operator<<(std::ostream&o, const state* S) {
		o << "===(" << std::setw(4) << S->id << ")======================================================" << std::endl;
		o << S->items;
		o << "-- Token transitions ------------------------------------------" << std::endl;
		follow_set_text::const_iterator fti, ftj = S->transitions.from_text.end();
		for(fti = S->transitions.from_text.begin();fti!=ftj;++fti) {
			if((*fti).second) { o << (*fti); }
		}
		o << "-- Non-terminal transitions -----------------------------------" << std::endl;
		follow_set_stack::const_iterator fsi, fsj = S->transitions.from_stack.end();
		fsj = S->transitions.from_stack.end();
		for(fsi = S->transitions.from_stack.begin();fsi!=fsj;++fsi) {
			if((*fsi).second) { o << (*fsi); }
		}
		o << "-- Reductions -------------------------------------------------" << std::endl << S->reductions;
		return o;
	}

	typedef std::set<state*, ptr_less<state> > state_set;

	struct state_id_less {
		bool operator()(const state* a, const state* b) const {
			return a->id < b->id;
		}
	};
	typedef std::set<state*,  state_id_less> state_set_dumper;

	struct process;
}

#include "lr_gss.h"

namespace lr {
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
			unsigned int furthest;

			struct error {
				unsigned int line, column;
			};

			automaton(grammar::Grammar* _)
				: G(_), S(), states()
			{
				items();
				/*grammar::visitors::nt_remover nr(G);*/
				/*nr.process(G);*/
			}

			~automaton() {
				state_set::iterator i, j=states.end();
				for(i=states.begin();i!=j;++i) {
					delete *i;
				}
			}

			void closure(item_set& I, item_set& C) const {
				item_set::iterator  i, j;
				C = I;
				std::cout << "C starts with " << C.size() << " elements" << std::endl;
				std::vector<item> stack(I.begin(), I.end());
				while(stack.size()>0) {
					item i = stack.back();
					stack.pop_back();
					if(i.at_end()) {
						continue;
					}
					const token::Nt* nt = dynamic_cast<const token::Nt*>(grammar::visitors::item_rewriter(G).process((grammar::item::base*)*i));
					if(nt) {
						std::cout << "have NT " << nt->tag() << std::endl;
						grammar::Grammar::iterator S = G->find(nt->tag());
						rule::base* r = (S==G->end()) ? NULL : S->second;
						if(!r) {
							std::cout << "couldn't find rule " << nt->tag() << " !" << std::endl;
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

			void reduction_candidates(const item_set&I, item_set& R) const {
				item_set::iterator i, j = I.end();
				for(i=I.begin();i!=j;++i) {
					if((*i).at_end()) {
						R.insert(*i);
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

			void follow(const item_set& I, follow_set_text& F) const {
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
					reduction_candidates(ret.first->items, ret.first->reductions);
					stack.push_back(ret.first);
					ret.first->id = states.size()-1;
					std::cout << "  committed new set :" << std::endl << stack.back()->items;
				} else {
					std::cout << "  didn't commit set :" << std::endl << s;
					std::cout << "  because of set :" << std::endl << ret.first->items;
				}
				return ret.first;
			}

			class transition_dispatcher : public grammar::visitors::dummy {
				private:
					state* S;
					grammar::Grammar* G;
					state* GOTO;
				public:
					transition_dispatcher(state* _, grammar::Grammar* g_) : S(_), G(g_), GOTO(0) {}
					void dispatch(grammar::item::base* tr, state* GOTO_) {
						GOTO = GOTO_;
						tr->accept(this);
					}
					virtual void visit(grammar::item::token::Str* x) { S->transitions.from_text[x] = GOTO; }
					virtual void visit(grammar::item::token::Re* x) { S->transitions.from_text[x] = GOTO; }
					virtual void visit(grammar::item::token::Epsilon* x) {}
					virtual void visit(grammar::item::token::Eof* x) { S->transitions.from_text[x] = GOTO; }
					virtual void visit(grammar::item::token::Comment* x) { S->transitions.from_text[x] = GOTO; }
					virtual void visit(grammar::item::token::T* x) { S->transitions.from_text[x] = GOTO; }
					virtual void visit(grammar::item::token::Nt* x) { S->transitions.from_stack[x->tag()] = GOTO; }
					virtual void visit(grammar::item::token::Bow* x) { S->transitions.from_text[x] = GOTO; }

					virtual void visit(grammar::item::combination::RawSeq* x) { S->transitions.from_text[x] = GOTO; }
			};

			void compute_transitions(item_set& items, follow_set_builder& transitions) {
				typedef grammar::item::base item_base;
				item_set prods;
				productible(items, prods);
				grammar::visitors::token_filter f;
				item_set::iterator i, j=prods.end();
				for(i=prods.begin();i!=j;++i) {
					const item_base* t = f((grammar::item::base*)**i);
					if(!t) {
						item x = *i;
						std::cout << "COIN " << x << " " << *x
							<< " " << typeid(**i).name() << std::endl;
						throw "COIN";
					}
					item tmp = (*i).next();
					std::cout << "  transiting to " << tmp << std::endl;
					std::pair<item_set::iterator, bool> ret = transitions[t].insert(tmp);
					if(!ret.second) {
						item x = *ret.first;
						std::cout << "COIN transition pas ajoutée " << x << std::endl;
					}
					grammar::visitors::debugger d(std::cout);
					std::cout << "  => transitions[";
					((item_base*)t)->accept(&d);
					std::cout << "] = " << transitions[t] << std::endl;
				}
			}


			void items() {
				grammar::rule::base* rule = (*G)["_start"];
				grammar::item::iterator iter = grammar::item::iterator::create(rule);
				item_set s0, tmp;
				do {
					item i0(rule, iter);
					tmp.insert(i0);
					++iter;
				} while(!iter.at_end());
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
					transition_dispatcher text_or_stack(S, G);
					for(fi=FSB.begin();fi!=fj;++fi) {
						item_set tmp;
						closure((*fi).second, tmp);
						text_or_stack.dispatch((grammar::item::base*)(*fi).first, items_commit(tmp, stack));
						/*S->transitions[(*fi).first] = items_commit(tmp, stack);*/
						/*std::cout << "finally, retain S->transitions[";*/
						/*grammar::visitors::debugger d;*/
						/*((grammar::item::base*)(*fi).first)->accept(&d);*/
						/*std::cout << "] = " << S->transitions[(*fi).first]->items << std::endl;*/
					}
				}
			}

			void dump_states() const {
				state_set_dumper ssd(states.begin(), states.end());
				std::cout << "automaton has " << ssd.size() << " states." << std::endl;
				state_set::iterator i, j = ssd.end();
				for(i=ssd.begin();i!=j;++i) {
					/*std::cout << "===============================================================" << std::endl;*/
					/*std::cout << (*i)->items << std::endl;*/
					/*std::cout << "---------------------------------------------------------------" << std::endl;*/
					/*follow_set::iterator fi, fj = (*i)->transitions.from_text.end();*/
					/*for(fi = (*i)->transitions.from_text.begin();fi!=fj;++fi) {*/
						/*std::cout << (*fi) << std::endl;*/
					/*}*/
					/*std::cout << "---------------------------------------------------------------" << std::endl;*/
					/*fj = (*i)->transitions.from_stack.end();*/
					/*for(fi = (*i)->transitions.from_stack.begin();fi!=fj;++fi) {*/
						/*std::cout << (*fi) << std::endl;*/
					/*}*/
					std::cout << *i << std::endl;
				}
			}

			/*node* shift(node* p, grammar::item::base* producer, state* s, ast_node_t ast, unsigned int offset) {*/
			ast_node_t parse(const char* buffer, unsigned int size) {
				grammar::visitors::lr_item_debugger debug;
				std::list<gss::node*> farthest_nodes;
				unsigned int farthest=0;
				gss stack(item((*G)["_start"], grammar::item::iterator::create((*G)["_start"])), size);
				/*int a=0;*/
				stack.shift(NULL, NULL, S0, NULL, 0);
				while(!stack.active.empty()) {
					gss::node* n = stack.consume_active();
					if(n->id.O>=farthest) {
						farthest = n->id.O;
						farthest_nodes.clear();
						farthest_nodes.push_front(n);
					}
					state* S = n->id.S;
					if(!S) {
						throw "COIN";
					}
#define min(a, b) (a<b?a:b)
					char* aststr = (char*)ast_serialize_to_string(n->ast); std::cout << " ===  ACTIVE STATE ===(" << S->id << ") @" << n->id.O << ':' << std::string(buffer+n->id.O, min(buffer+n->id.O+20, buffer+size)) << std::endl << "ast : " << aststr << std::endl; free(aststr);
					item_set::iterator i, j;

					follow_set_text::iterator ti, tj;
					unsigned int ofs = n->id.O;
					ofs = G->skip(buffer, n->id.O, size);

					for(ti=S->transitions.from_text.begin(), tj=S->transitions.from_text.end();ti!=tj;++ti) {
						if(!(*ti).second) {
							std::cout << "null entry in transition table !" << std::endl;
							continue;
						}
						const grammar::item::base* token = (*ti).first;
						std::pair<ast_node_t, unsigned int> ret = token->recognize(buffer, ofs, size);
						std::cout << "follow by "; ((grammar::item::base*)token)->accept(&debug); std::cout << " => " << ret.first << " (" << ret.second << ')' << std::endl;
						if(ret.first) {
							stack.shift(n, (grammar::item::base*)(*ti).first, (*ti).second, ret.first, ret.second);
						}
					}

					for(i=S->reductions.begin(), j=S->reductions.end();i!=j;++i) {
						gss::node*ok = stack.reduce(n, *i, ofs);
						item x = *i;
						if(ok) { std::cout << "reduce by " << x << " => " << ok->id.S->id << std::endl; }
					}
				}
				farthest = G->skip(buffer, farthest, size);
				/* error handling */
				if(farthest!=size) {
					unsigned int nl_before = 0, nl_after = 0, tmp = 0;
					struct {
						unsigned int operator()(const char*buffer, unsigned int ofs) {
							for(;buffer[ofs];++ofs) {
								if(buffer[ofs]=='\r') {
									if(buffer[ofs+1]=='\n') {
										++ofs;
									}
									return ofs+1;
								} else if(buffer[ofs]=='\n') {
									return ofs+1;
								}
							}
							return ofs;
						}
					} find_nl;
					unsigned int line=1, column;
					while((tmp=find_nl(buffer, nl_before))<=farthest) { nl_before = tmp; ++line; }
					nl_after = tmp;
					column = farthest - nl_before + 1;
					std::cout << "parsing stopped at line " << line << ", column " << column << std::endl;
					std::cout << std::string(buffer+nl_before, buffer+nl_after) << std::endl;
					std::cout << std::setw(farthest-nl_before) << "" << '^' << std::endl;
					std::list<gss::node*>::iterator i, j;
					for(i=farthest_nodes.begin(), j=farthest_nodes.end();i!=j;++i) {
						follow_set_text::iterator ti, tj;
						state* S = (*i)->id.S;
						item_set K;
						kernel(S->items, K);
						std::cout << K;
						if(S->transitions.from_text.size()) {
							std::cout << "expected one of ";
							grammar::visitors::debugger d;
							for(ti=S->transitions.from_text.begin(), tj=S->transitions.from_text.end();ti!=tj;++ti) {
								((grammar::item::base*)(*ti).first)->accept(&d); std::cout << ' ';
							}
						} else {
							std::cout << "expected end of text";
						}
					}
					std::cout << std::endl;
				}
				furthest=farthest;
				return stack.accepted;
			}


			static bool test(int testno, const char* gram, const char* txt, const char* expected) {
				std::string grammar;
				grammar = "((Grammar (TransientRule _start (NT X)) ";
				grammar += gram;
				grammar += "))";
				ast_node_t g = gram?Cdr(Car(ast_unserialize(grammar.c_str()))):NULL;
				grammar::Grammar gg(g);
				automaton r2d2(&gg);
				std::cout << "===========================================================" << std::endl;
				std::cout << "===========================================================" << std::endl;
				std::cout << "Grammar : " << grammar << std::endl;
				grammar::visitors::debugger d;
				gg.accept(&d);
				std::cout << "Input : " << txt << std::endl;
				std::cout << "===========================================================" << std::endl;
				r2d2.dump_states();
				ast_node_t ret = r2d2.parse(txt, strlen(txt));
				char* tmp = (char*)(ret?ast_serialize_to_string(ret):strdup("nil"));
				bool ok = false;
				if(!ret) {
					ok = !expected;
				} else {
					ok = !strcmp(tmp, expected);
				}
				if(!ok) {
					std::cout << "[TEST] [automaton] #" << testno << ": With grammar " << gram << " and input \"" << txt << "\", expected --" << (expected?expected:"nil") << "-- and got --" << tmp << "--" << std::endl;
				}
				free(tmp);
				return ok;
			}
	};

}



#endif

