#ifndef _TINYAP_LR_VISITORS_H_
#define _TINYAP_LR_VISITORS_H_

namespace grammar {
	namespace visitors {
		template <typename O>
			class evaluator : public visitor {
				protected:
					O output;
				public:
					O process(item::base* i) {
						if(i) {
							i->accept(this);
							return output;
						} else {
							return NULL;
						}
					}

					virtual void visit(item::token::Str* i) { output = eval(i); }
					virtual void visit(item::token::Re* i) { output = eval(i); }
					virtual void visit(item::token::Epsilon* i) { output = eval(i); }
					virtual void visit(item::token::Eof* i) { output = eval(i); }
					virtual void visit(item::token::Comment* i) { output = eval(i); }
					virtual void visit(item::token::T* i) { output = eval(i); }
					virtual void visit(item::token::Nt* i) { output = eval(i); }
					virtual void visit(item::token::Bow* i) { output = eval(i); }
					virtual void visit(item::token::AddToBag* i) { output = eval(i); }

					virtual void visit(item::combination::Rep01* i) { output = eval(i); }
					virtual void visit(item::combination::Rep0N* i) { output = eval(i); }
					virtual void visit(item::combination::Rep1N* i) { output = eval(i); }
					virtual void visit(item::combination::Prefix* i) { output = eval(i); }
					virtual void visit(item::combination::Postfix* i) { output = eval(i); }
					virtual void visit(item::combination::Seq* i) { output = eval(i); }
					virtual void visit(item::combination::RawSeq* i) { output = eval(i); }
					virtual void visit(item::combination::Alt* i) { output = eval(i); }

					virtual void visit(rule::Transient* i) { output = eval(i); }
					virtual void visit(rule::Operator* i) { output = eval(i); }
					virtual void visit(rule::Prefix* i) { output = eval(i); }
					virtual void visit(rule::Postfix* i) { output = eval(i); }

					virtual void visit(Grammar* i) { output = eval(i); }

					virtual O eval(item::token::Str*) = 0;
					virtual O eval(item::token::Re*) = 0;
					virtual O eval(item::token::Epsilon*) = 0;
					virtual O eval(item::token::Eof*) = 0;
					virtual O eval(item::token::Comment*) = 0;
					virtual O eval(item::token::T*) = 0;
					virtual O eval(item::token::Nt*) = 0;
					virtual O eval(item::token::Bow*) = 0;
					virtual O eval(item::token::AddToBag*) = 0;

					virtual O eval(item::combination::Rep01*) = 0;
					virtual O eval(item::combination::Rep0N*) = 0;
					virtual O eval(item::combination::Rep1N*) = 0;
					virtual O eval(item::combination::Prefix*) = 0;
					virtual O eval(item::combination::Postfix*) = 0;
					virtual O eval(item::combination::Seq*) = 0;
					virtual O eval(item::combination::RawSeq*) = 0;
					virtual O eval(item::combination::Alt*) = 0;

					virtual O eval(rule::Transient*) = 0;
					virtual O eval(rule::Operator*) = 0;
					virtual O eval(rule::Prefix*) = 0;
					virtual O eval(rule::Postfix*) = 0;

					virtual O eval(Grammar*) = 0;
		};


		template <typename R>
		class dummy_filter : public evaluator<R> {
			public:
				R operator() (item::base* _) {
					return evaluator<R>::process(_);
				}

				virtual R eval(item::token::Str*) { return 0; }
				virtual R eval(item::token::Re*) { return 0; }
				virtual R eval(item::token::Epsilon*) { return 0; }
				virtual R eval(item::token::Eof*) { return 0; }
				virtual R eval(item::token::Comment*) { return 0; }
				virtual R eval(item::token::T*) { return 0; }
				virtual R eval(item::token::Nt*) { return 0; }
				virtual R eval(item::token::Bow*) { return 0; }
				virtual R eval(item::token::AddToBag*) { return 0; }

				virtual R eval(item::combination::Rep01*) { return 0; }
				virtual R eval(item::combination::Rep0N*) { return 0; }
				virtual R eval(item::combination::Rep1N*) { return 0; }
				virtual R eval(item::combination::Prefix*) { return 0; }
				virtual R eval(item::combination::Postfix*) { return 0; }
				virtual R eval(item::combination::Seq*) { return 0; }
				virtual R eval(item::combination::RawSeq*) { return 0; }
				virtual R eval(item::combination::Alt*) { return 0; }

				virtual R eval(rule::Transient*) { return 0; }
				virtual R eval(rule::Operator*) { return 0; }
				virtual R eval(rule::Prefix*) { return 0; }
				virtual R eval(rule::Postfix*) { return 0; }

				virtual R eval(Grammar*) { return 0; }
		};

		class filter : public evaluator<item::base*> {
			public:
				item::base* operator() (item::base* _) {
					return process(_);
				}

				virtual item::base* eval(item::token::Str*) { return NULL; }
				virtual item::base* eval(item::token::Re*) { return NULL; }
				virtual item::base* eval(item::token::Epsilon*) { return NULL; }
				virtual item::base* eval(item::token::Eof*) { return NULL; }
				virtual item::base* eval(item::token::Comment*) { return NULL; }
				virtual item::base* eval(item::token::T*) { return NULL; }
				virtual item::base* eval(item::token::Nt*) { return NULL; }
				virtual item::base* eval(item::token::Bow*) { return NULL; }
				virtual item::base* eval(item::token::AddToBag*) { return NULL; }

				virtual item::base* eval(item::combination::Rep01*) { return NULL; }
				virtual item::base* eval(item::combination::Rep0N*) { return NULL; }
				virtual item::base* eval(item::combination::Rep1N*) { return NULL; }
				virtual item::base* eval(item::combination::Prefix*) { return NULL; }
				virtual item::base* eval(item::combination::Postfix*) { return NULL; }
				virtual item::base* eval(item::combination::Seq*) { return NULL; }
				virtual item::base* eval(item::combination::RawSeq*) { return NULL; }
				virtual item::base* eval(item::combination::Alt*) { return NULL; }

				virtual item::base* eval(rule::Transient*) { return NULL; }
				virtual item::base* eval(rule::Operator*) { return NULL; }
				virtual item::base* eval(rule::Prefix*) { return NULL; }
				virtual item::base* eval(rule::Postfix*) { return NULL; }

				virtual item::base* eval(Grammar*) { return NULL; }
		};

		class identity : public evaluator<item::base*> {
			public:
				item::base* operator() (item::base* _) {
					return process(_);
				}

				virtual item::base* eval(item::token::Str* x) { return x; }
				virtual item::base* eval(item::token::Re* x) { return x; }
				virtual item::base* eval(item::token::Epsilon* x) { return x; }
				virtual item::base* eval(item::token::Eof* x) { return x; }
				virtual item::base* eval(item::token::Comment* x) { return x; }
				virtual item::base* eval(item::token::T* x) { return x; }
				virtual item::base* eval(item::token::Nt* x) { return x; }
				virtual item::base* eval(item::token::Bow* x) { return x; }
				virtual item::base* eval(item::token::AddToBag* x) { return x; }

				virtual item::base* eval(item::combination::Rep01* x) { return x; }
				virtual item::base* eval(item::combination::Rep0N* x) { return x; }
				virtual item::base* eval(item::combination::Rep1N* x) { return x; }
				virtual item::base* eval(item::combination::Prefix* x) { return x; }
				virtual item::base* eval(item::combination::Postfix* x) { return x; }
				virtual item::base* eval(item::combination::Seq* x) { return x; }
				virtual item::base* eval(item::combination::RawSeq* x) { return x; }
				virtual item::base* eval(item::combination::Alt* x) { return x; }

				virtual item::base* eval(rule::Transient* x) { return x; }
				virtual item::base* eval(rule::Operator* x) { return x; }
				virtual item::base* eval(rule::Prefix* x) { return x; }
				virtual item::base* eval(rule::Postfix* x) { return x; }

				virtual item::base* eval(Grammar* x) { return x; }
		};


		class iterator_factory : public evaluator<item::_iterator_base*> {
			public:
				typedef item::_iterator_base* return_type;
				return_type operator() (const item::base* _) {
					return process((item::base*)_);
				}

				virtual return_type eval(item::token::Str* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::token::Re* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::token::Epsilon* x) {
					return new item::iterators::iterator_epsilon(x);
				}
				virtual return_type eval(item::token::Eof* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::token::Comment* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::token::T* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::token::Nt* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::token::Bow* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::token::AddToBag* x) {
					return new item::iterators::iterator_single(x);
				}

				virtual return_type eval(item::combination::Rep01* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::combination::Rep0N* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::combination::Rep1N* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::combination::Prefix* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::combination::Postfix* x) {
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::combination::Seq* x) {
					/*std::clog << "iterator Seq" << std::endl;*/
					return new item::iterators::iterator_vector(x);
				}
				virtual return_type eval(item::combination::RawSeq* x) {
					/*std::clog << "iterator RawSeq" << std::endl;*/
					return new item::iterators::iterator_single(x);
				}
				virtual return_type eval(item::combination::Alt* x) {
					/*std::clog << "iterator Alt" << std::endl;*/
					return new item::iterators::iterator_set(x);
				}

				virtual return_type eval(rule::Transient* x) {
					return new item::iterators::iterator_set(x);
				}
				virtual return_type eval(rule::Operator* x) {
					return new item::iterators::iterator_set(x);
				}
				virtual return_type eval(rule::Prefix* x) {
					return new item::iterators::iterator_set(x);
				}
				virtual return_type eval(rule::Postfix* x) {
					return new item::iterators::iterator_set(x);
				}

				virtual return_type eval(Grammar* x) {
					return new item::iterators::iterator_map(x);
				}
		};





		class token_filter : public filter {
			public:
				virtual item::base* eval(item::token::Str*x) { return x; }
				virtual item::base* eval(item::token::Re*x) { return x; }
				virtual item::base* eval(item::token::Epsilon*x) { return NULL; }
				virtual item::base* eval(item::token::Eof*x) { return x; }
				virtual item::base* eval(item::token::T*x) { return x; }
				virtual item::base* eval(item::token::Bow*x) { return x; }
				virtual item::base* eval(item::token::AddToBag*x) { return x; }

				virtual item::base* eval(item::combination::RawSeq*x) { return x; }
				virtual item::base* eval(item::combination::Seq*x) { return x->contents(); }
				virtual item::base* eval(item::combination::Alt*x) { return x->contents(); }
				virtual item::base* eval(item::combination::Rep01*x) { return x->contents(); }
				virtual item::base* eval(item::combination::Rep0N*x) { return x->contents(); }
				virtual item::base* eval(item::combination::Rep1N*x) { return x->contents(); }

				virtual item::base* eval(item::token::Nt*x) { return x; }
		};

		class producer_filter : public token_filter {
			public:
				virtual item::base* eval(item::token::Nt*x) { return NULL; }
		};


		class dummy : public visitor {
			public:
				virtual void visit(rule::Postfix* x) {}
				virtual void visit(rule::Prefix* x) {}
				virtual void visit(rule::Transient* x) {}
				virtual void visit(rule::Operator* x) {}
				virtual void visit(Grammar* x) {}
				virtual void visit(item::token::Str* x) {}
				virtual void visit(item::token::Re* x) {}
				virtual void visit(item::token::Epsilon* x) {}
				virtual void visit(item::token::Eof* x) {}
				virtual void visit(item::token::Comment* x) {}
				virtual void visit(item::token::T* x) {}
				virtual void visit(item::token::Nt* x) {}
				virtual void visit(item::token::Bow* x) {}
				virtual void visit(item::token::AddToBag* x) {}

				virtual void visit(item::combination::Rep01* x) {}
				virtual void visit(item::combination::Rep0N* x) {}
				virtual void visit(item::combination::Rep1N* x) {}
				virtual void visit(item::combination::Prefix* x) {}
				virtual void visit(item::combination::Postfix* x) {}
				virtual void visit(item::combination::Seq* x) {}
				virtual void visit(item::combination::RawSeq* x) {}
				virtual void visit(item::combination::Alt* x) {}
		};

		class debugger : public visitor {
			protected:
				std::ostream& os;
				int rec;
				void prefix() {
					for(int i=0;i<rec;++i) {
						os << '\t';
					}
				}
			public:
				debugger(std::ostream& o = std::clog) : os(o), rec(0) {}

				virtual void visit(rule::Postfix* x) {
					rule::Postfix::iterator i = x->begin(), j = x->end();
					while(i!=j) {
						prefix();
						os << x->tag() << " )= ";
						if(*i) { (*i)->accept(this); }
						os << std::endl;
						++i;
					}
				}
				virtual void visit(rule::Prefix* x) {
					rule::Prefix::iterator i = x->begin(), j = x->end();
					while(i!=j) {
						prefix();
						os << x->tag() << " (= ";
						if(*i) { (*i)->accept(this); }
						os << std::endl;
						++i;
					}
				}
				virtual void visit(rule::Transient* x) {
					rule::Transient::iterator i = x->begin(), j = x->end();
					while(i!=j) {
						prefix();
						os << x->tag() << " = ";
						if(*i) { (*i)->accept(this); }
						os << std::endl;
						++i;
					}
				}
				virtual void visit(rule::Operator* x) {
					rule::Operator::iterator i = x->begin(), j = x->end();
					while(i!=j) {
						prefix();
						os << x->tag() << " ::= ";
						if(*i) { (*i)->accept(this); }
						os << std::endl;
						++i;
					}
				}
				virtual void visit(Grammar* x) {
					prefix();
					os << "Grammar" << std::endl;
					Grammar::iterator i = x->begin(), j = x->end();
					++rec;
					while(i!=j) {
						if((*i).second) {
							/*prefix();*/
							/*os << (*i).first << " : " << (*i).second->size() << std::endl;;*/
							((*i).second)->accept(this);
						}
						os << std::endl;
						++i;
					}
					--rec;
				}
				virtual void visit(item::token::Str* x) {
					os << '~' << x->start() << "," << x->end() << '~';
				}
				virtual void visit(item::token::Re* x) {
					os << '/' << x->pattern() << '/';
				}
				virtual void visit(item::token::Epsilon* x) {
					os << "_epsilon";
				}
				virtual void visit(item::token::Eof* x) {
					os << "_EOF";
				}
				virtual void visit(item::token::Comment* x) {
					os << "Comment";
				}
				virtual void visit(item::token::T* x) {
					os << '"' << x->str() << '"';
				}
				virtual void visit(item::token::Nt* x) {
					os << "nt:" << x->tag();
				}
				virtual void visit(item::token::Bow* x) {
					os << '~' << x->tag() << '~';
				}

				virtual void visit(item::token::AddToBag* x) {
					os << '/' << x->pattern() << "/:" << x->tag() << (x->keep()?"!":"");
				}

				virtual void visit(item::combination::Rep01* x) {
					/*os << '(';*/
					x->contents()->accept(this);
					/*os << ")?";*/
				}
				virtual void visit(item::combination::Rep0N* x) {
					/*os << '(';*/
					x->contents()->accept(this);
					/*os << ")*";*/
				}
				virtual void visit(item::combination::Rep1N* x) {
#if 1
					x->contents()->accept(this);
#else
					os << '(';
					x->raw_cts->accept(this);
					os << ")+";
#endif
				}
				virtual void visit(item::combination::Prefix* x) {
					/*os << "Prefix";*/
					x->contents()->accept(this);
				}
				virtual void visit(item::combination::Postfix* x) {
					/*os << "Postfix";*/
					x->contents()->accept(this);
				}
				virtual void visit(item::combination::Seq* x) {
					/*prefix();*/
					/*os << "Seq";*/
					item::combination::Seq::iterator i=x->begin(), j=x->end();
					while(i!=j) {
						os << ' ';
						if(*i) { (*i)->accept(this); }
						++i;
					}
				}
				virtual void visit(item::combination::RawSeq* x) {
					os << "(.raw";
					item::combination::RawSeq::iterator i=x->begin(), j=x->end();
					while(i!=j) {
						os << ' ';
						if(*i) { (*i)->accept(this); }
						++i;
					}
					os << ')';
				}

				virtual void visit(item::combination::Alt* x) {
#if 1
					x->_->accept(this);
#else
					char c = '(';
					item::combination::Alt::iterator i=x->begin(), j=x->end();
					while(i!=j) {
						if(*i) {
							os << c;
							(*i)->accept(this);
						}
						++i;
						c = '|';
					}
					os << ')';
#endif
				}
		};

		class lr_item_debugger : public debugger {
			public:
				lr_item_debugger(std::ostream& o = std::clog) : debugger(o) {}

				virtual void visit(rule::Postfix* x) {
					os << x->tag();
				}
				virtual void visit(rule::Prefix* x) {
					os << x->tag();
				}
				virtual void visit(rule::Transient* x) {
					os << x->tag();
				}
				virtual void visit(rule::Operator* x) {
					os << x->tag();
				}
		};

		template <class I>
			debugger& operator << (debugger&d, const I* i) {
				((I*)i)->accept(&d);
				return d;
			}

		inline std::ostream& operator<<(std::ostream& o, const item::base* i) {
			debugger d(o);
			((item::base*)i)->accept(&d);
			return o;
		}




		class nt_remover : public identity {
			private:
				Grammar* g;
			public:
				nt_remover(Grammar*_) : g(_) {}

				virtual item::base* eval(item::token::Nt* x) { return (*g)[x->tag()]; }

				virtual item::base* eval(item::combination::Seq* x) {
					item::combination::Seq::iterator i, j=x->end();
					for(i=x->begin();i!=j;++i) {
						*i = process(*i);
					}
					return x;
				}
				virtual item::base* eval(item::combination::RawSeq* x) {
					item::combination::RawSeq::iterator i, j=x->end();
					for(i=x->begin();i!=j;++i) {
						*i = process(*i);
					}
					return x;
				}
				virtual item::base* eval(rule::Transient* x) {
					return rule_eval(x);
				}
				virtual item::base* eval(rule::Operator* x) {
					return rule_eval(x);
				}
				virtual item::base* eval(rule::Prefix* x) {
					return rule_eval(x);
				}
				virtual item::base* eval(rule::Postfix* x) {
					return rule_eval(x);
				}
				item::base* rule_eval(rule::base* x) {
					rule::base::iterator i, j=x->end();
					std::list<item::base*> remove, add;
					for(i=x->begin();i!=j;++i) {
						item::base* k = process(*i);
						if(k!=*i) {
							add.push_front(k);
							remove.push_front(*i);
						}
					}
					std::list<item::base*>::iterator li, lj;
					for(li=remove.begin(), lj=remove.end();li!=lj;++li) {
						x->erase(*li);
					}
					for(li=add.begin(), lj=add.end();li!=lj;++li) {
						x->insert(*li);
					}
					return x;
				}
				virtual item::base* eval(Grammar* x) {
					Grammar::iterator i, j=x->end();
					for(i=x->begin();i!=j;++i) {
						process((*i).second);
					}
					return x;
				}
		};







		class item_rewriter : public evaluator<item::base*> {
			private:
				Grammar* g;
			public:
				item_rewriter(Grammar*_) : g(_) {}

				item::base* operator() (ast_node_t _) {
					item::base* orig = item::base::from_ast(_, g);
					if(orig) {
						item::base* ret = process(orig);
						if(orig!=ret) {
							/*delete orig;*/
						}
						return ret;
					}
					return NULL;
				}

				virtual item::base* eval(item::token::Str* x) {
					/*std::clog << "eval(Str)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(item::token::Re* x) {
					/*std::clog << "eval(Re)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(item::token::Epsilon* x) {
					/*std::clog << "eval(Epsilon)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(item::token::Eof* x) {
					/*std::clog << "eval(Eof)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(item::token::Comment* x) {
					/*std::clog << "eval(Comment)" << std::endl;*/
					/*delete x;*/
					/*return NULL;*/
					return x;
				}
				virtual item::base* eval(item::token::T* x) {
					/*std::clog << "eval(T)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(item::token::Nt* x) {
					/*std::clog << "eval(Nt)" << std::endl;*/
					return x;
				}

				virtual item::base* eval(item::token::Bow* x) {
					/*std::clog << "eval(Bow)" << std::endl;*/
					return x;
				}

				virtual item::base* eval(item::token::AddToBag* x) {
					/*std::clog << "eval(AddToBag)" << std::endl;*/
					return x;
				}

				virtual item::base* eval(item::combination::Rep01* x) {
					/*debugger d(std::clog);*/
					/*std::clog << "eval(Rep01:" << x << ") "; d << x; std::clog << std::endl;*/
					return x->commit(g);
				}
				virtual item::base* eval(item::combination::Rep0N* x) {
					/*std::clog << "eval(Rep0N)" << std::endl;*/
					return x->commit(g);
				}
				virtual item::base* eval(item::combination::Rep1N* x) {
					/*std::clog << "eval(Rep1N)" << std::endl;*/
					return x->commit(g);
				}
				virtual item::base* eval(item::combination::Prefix* x) {
					/*std::clog << "eval(Prefix)" << std::endl;*/
					return x->commit(g);
				}
				virtual item::base* eval(item::combination::Postfix* x) {
					/*std::clog << "eval(Postfix)" << std::endl;*/
					return x->commit(g);
				}
				virtual item::base* eval(item::combination::Seq* x) {
					/*std::clog << "eval(Seq)" << std::endl;*/
					item::combination::Seq::iterator i, j;
					for(i=x->begin(), j=x->end();i!=j;++i) {
						process(*i);
					}
					return x->commit(g);
				}
				virtual item::base* eval(item::combination::RawSeq* x) {
					/*std::clog << "eval(RawSeq)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(item::combination::Alt* x) {
					/*std::clog << "rewrite(Alt)" << std::endl;*/
					item::combination::Alt::iterator i, j;
					for(i=x->begin(), j=x->end();i!=j;++i) {
						process(*i);
					}
					return x->commit(g);
				}

				virtual item::base* eval(rule::Transient* x) {
					/*std::clog << "eval(Transient)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(rule::Operator* x) {
					/*std::clog << "eval(Operator)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(rule::Prefix* x) {
					/*std::clog << "eval(Prefix)" << std::endl;*/
					return x;
				}
				virtual item::base* eval(rule::Postfix* x) {
					/*std::clog << "eval(Postfix)" << std::endl;*/
					return x;
				}

				virtual item::base* eval(Grammar* x) {
					/*std::clog << "eval(Grammar)" << std::endl;*/
					return NULL;
				}

		};


		class rmember_rewriter : public item_rewriter {
			private:
				Grammar* g;
				rule::base* r;
			public:
				rmember_rewriter(Grammar*_, rule::base*rule)
					: item_rewriter(_), r(rule) {}

				/*item::base* operator() (ast_node_t _) {*/
					/*return process(item::base::from_ast(_));*/
				/*}*/
				item::base* operator() (item::base* _) {
					/*std::clog << "rmb_rwrt(cid="<<_->class_id()<<")..."<<std::endl;*/
					return process(_);
				}

				virtual item::base* eval(item::token::Comment* x) {
					/*delete x;*/
					return NULL;
				}

				virtual item::base* eval(item::combination::Seq* x) {
					return x;
				}

				virtual item::base* eval(item::combination::Alt* x) {
					if(!x) {
						return NULL;
					}
					item::combination::Alt::iterator i=x->begin(), j=x->end();
					for(;i!=j;++i) {
						/*if(*i) { r->insert(*i); }*/
						item::base* tmp = process(*i);
						if(tmp) { r->insert(tmp); }
					}
					/*x->clear();*/
					/*delete x;*/
					return NULL;
				}
		};

		class reducer : public dummy_filter<ast_node_t> {
			private:
				ast_node_t ast;
				unsigned int o;
			public:
				reducer(ast_node_t _, unsigned int o_) : ast(_), o(o_) {}
				virtual ast_node_t eval(rule::Transient* x) {
					return x->reduce_ast(ast, o);
				}
				virtual ast_node_t eval(rule::Operator* x) {
					return x->reduce_ast(ast, o);
				}
				virtual ast_node_t eval(rule::Prefix* x) {
					return x->reduce_ast(ast, o);
				}
				virtual ast_node_t eval(rule::Postfix* x) {
					return x->reduce_ast(ast, o);
				}
		};
	}
}


#endif

