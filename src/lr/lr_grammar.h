#ifndef _TINYAP_LR_GRAMMAR_H_
#define _TINYAP_LR_GRAMMAR_H_

#include <cxxabi.h>		/* for demangling */

namespace grammar {
	struct _h {
		size_t operator()(const char*k) const {
			return (size_t) regstr(k);
		}
	};

	struct _c {
		bool operator()(const char*k1, const char* k2) const {
			return !strcmp(k1, k2);
		}
	};

	typedef ext::hash_map<const char*, rule::base*, _h, _c> map_type;

	namespace item {
		class base {
			protected:
				static int _class_id_counter() {
					static int _=0;
					return ++_;
				}
			public:
				virtual ~base() {}
				virtual int class_id() const = 0;
				virtual bool is_same(const base* i) const = 0;
				virtual bool is_less(const base* i) const = 0;
				virtual ast_node_t to_ast() { return NULL; }
				static base* from_ast(ast_node_t, Grammar*g = NULL);
				virtual void accept(visitors::visitor*v) = 0;
		};
	}
}

namespace std {
	template <>
		struct less<grammar::item::base*> {
			bool operator()(grammar::item::base* a, grammar::item::base* b) const { return a->is_less(b); }
		};
}

namespace grammar {
	namespace item {
		class _iterator_base {
			private:
				const base* context_;
				size_t ref_;
			public:
				_iterator_base(const base*c) : context_(c), ref_(0) {}
				virtual ~_iterator_base() {}
				const base* context() const { return context_; }
				void ref() { ++ref_; }
				bool unref() { return  --ref_<=0; }
				virtual void inc() = 0;
				virtual void dec() = 0;
				bool shared() const { return ref_>1; }
				virtual bool at_start() const = 0;
				virtual bool at_end() const = 0;
				virtual const base* get() const = 0;
				virtual size_t len() const = 0;
				virtual bool eq(const _iterator_base*i) const = 0;
				virtual bool inf(const _iterator_base*i) const = 0;
				virtual _iterator_base* clone() const = 0;
		};
		class iterator {
			private:
				_iterator_base* impl;
				iterator(_iterator_base* i_) : impl(i_) { impl->ref(); }
				iterator() : impl(0) {}
			public:
				const base* context() const { return impl->context(); }
				iterator(const iterator&i) : impl(i.impl) { impl->ref(); }
				~iterator() { if(impl->unref()) { delete impl; } }
				iterator& operator = (const iterator& i) { if(impl&&impl->unref()) { delete impl; } impl=i.impl; impl->ref(); return *this; }
				iterator& operator --() { if(impl->shared()) { impl->unref(); impl = impl->clone(); impl->ref(); } impl->dec(); return *this; }
				iterator& operator ++() { if(impl->shared()) { impl->unref(); impl = impl->clone(); impl->ref(); } impl->inc(); return *this; }
				bool at_start() const { return impl->at_start(); }
				bool at_end() const { return impl->at_end(); }
				const base* operator*() const { return impl->get(); }

				bool operator<(const iterator&i_) const {
					/*std::cout << "{{iterator::inf : context " << (impl->context()==i_.impl->context()) << " impl->inf " << (impl->inf(i_.impl)) << "}}";*/
					return impl->context()<i_.impl->context() || (impl->context()==i_.impl->context() && impl->inf(i_.impl));
				}

				bool operator==(const iterator&i_) const {
					/*std::cout << "{{iterator::eq : context " << (impl->context()==i_.impl->context()) << " impl->eq " << (impl->eq(i_.impl)) << "}}";*/
					return impl->context()==i_.impl->context() && impl->eq(i_.impl);
				}

				bool operator!=(const iterator&i_) const {
					/*std::cout << "{{iterator::neq : !context " << (impl->context()!=i_.impl->context()) << " !impl->eq " << (!impl->eq(i_.impl)) << "}}";*/
					return impl->context()!=i_.impl->context() || !impl->eq(i_.impl);
				}

				static iterator create(const item::base*);
		};

		namespace iterators {
			class iterator_epsilon : public _iterator_base {
				public:
					iterator_epsilon(const base*c) : _iterator_base(c) {
						/*std::cout << "iterator_epsilon" << std::endl;*/
					}
					virtual void inc() {}
					virtual void dec() {}
					virtual const base* get() const { return context(); }
					virtual bool at_start() const { return true; }
					virtual bool at_end() const { return true; }
					virtual size_t len() const { return 0; }
					virtual bool eq(const _iterator_base*i) const {
						return !!dynamic_cast<const iterator_epsilon*>(i);
					}
					virtual bool inf(const _iterator_base*i) const {
						return false;
					}
					virtual _iterator_base* clone() const {
						return new iterator_epsilon(context());
					}
			};

			class iterator_single : public _iterator_base {
				private:
					bool end;
				public:
					iterator_single(const base*c) : _iterator_base(c), end(false) {
						/*std::cout << "iterator_single" << std::endl;*/
					}
					virtual void inc() { end=true; }
					virtual void dec() { end=false; }
					virtual bool at_start() const { return !end; }
					virtual bool at_end() const { return end; }
					virtual const base* get() const { return end?NULL:context(); }
					virtual size_t len() const { return 1; }
					virtual bool eq(const _iterator_base*i) const {
						const iterator_single*is = dynamic_cast<const iterator_single*>(i);
						return !!is && context()==is->context() && end==is->end;
					}
					virtual bool inf(const _iterator_base*i) const {
						const iterator_single*is = dynamic_cast<const iterator_single*>(i);
						return !!is && ( context()==is->context() ? is->end && !end : context()<is->context());
					}
					virtual _iterator_base* clone() const {
						iterator_single* c = new iterator_single(context());
						c->end = end;
						return c;
					}
			};

			class iterator_vector : public _iterator_base {
				private:
					std::vector<base*>::const_iterator seq, begin, end;
				public:
					iterator_vector(const base*c) : _iterator_base(c) {
						/*std::cout << "iterator_vector" << std::endl;*/
						const std::vector<base*>* v = dynamic_cast<const std::vector<base*>*>(c);
						begin = v->begin();
						end = v->end();
						seq = begin;
					}
					virtual void inc() { ++seq; }
					virtual void dec() { --seq; }
					virtual const base* get() const { return seq==end?NULL:*seq; }
					virtual bool at_start() const { return seq==begin; }
					virtual bool at_end() const { return seq==end; }
					virtual size_t len() const { return end-begin; }
					virtual bool eq(const _iterator_base*i) const {
						const iterator_vector*is = dynamic_cast<const iterator_vector*>(i);
						/*std::cout << "iterator_vector::eq ";*/
						return !!is && begin==is->begin && end==is->end && seq==is->seq;
					}
					virtual bool inf(const _iterator_base*i) const {
						const iterator_vector*is = dynamic_cast<const iterator_vector*>(i);
						/*std::cout << "iterator_vector::inf ";*/
						return !!is && (begin < is->begin || end < is->end || seq < is->seq);
					}
					virtual _iterator_base* clone() const {
						iterator_vector* c = new iterator_vector(context());
						c->seq = seq;
						c->begin = begin;
						c->end = end;
						return c;
					}
			};

			class iterator_set : public _iterator_base {
				private:
					std::set<base*>::const_iterator seq, begin, end;
					size_t sz;
				public:
					iterator_set(const base*c) : _iterator_base(c) {
						/*std::cout << "iterator_set" << std::endl;*/
						const std::set<base*>* v = dynamic_cast<const std::set<base*>*>(c);
						sz = v->size();
						begin = v->begin();
						end = v->end();
						seq = begin;
					}
					virtual void inc() { ++seq; }
					virtual void dec() { --seq; }
					virtual const base* get() const { return seq==end?NULL:*seq; }
					virtual bool at_start() const { return seq==begin; }
					virtual bool at_end() const { return seq==end; }
					virtual size_t len() const { return sz; }

					virtual bool eq(const _iterator_base*i) const {
						const iterator_set*is = dynamic_cast<const iterator_set*>(i);
						/*std::cout << "iterator_set::eq" << std::endl;*/
						return !!is && begin==is->begin && end==is->end && seq==is->seq;
					}

					virtual bool inf(const _iterator_base*i) const {
						const iterator_set*is = dynamic_cast<const iterator_set*>(i);
						/*std::cout << "iterator_set::inf" << std::endl;*/
						return !!is && (begin != is->begin || end != is->end || *seq < *is->seq);
					}

					/*virtual bool eq(const _iterator_base*i) const {*/
						/*const iterator_set*is = dynamic_cast<const iterator_set*>(i);*/
						/*return !!is && seq==is->seq;*/
					/*}*/
					/*virtual bool inf(const _iterator_base*i) const {*/
						/*const iterator_set*is = dynamic_cast<const iterator_set*>(i);*/
						/*return !!is && *seq<*is->seq;*/
					/*}*/

					virtual _iterator_base* clone() const {
						iterator_set* c = new iterator_set(context());
						c->seq = seq;
						c->begin = begin;
						c->end = end;
						c->sz = sz;
						return c;
					}
			};
		}

		template <class C>
			struct equal_to {
				bool operator() (const C* a, const C*b) const;
			};

		template <class C>
			struct less_than {
				bool operator() (const C* a, const C*b) const;
			};

		template <class C, class S=base >
			class item_with_class_id : public S {
				public:
					item_with_class_id() { /*std::cout << "new object (cid=" << class_id() << ')' << std::endl;*/ }
					virtual void accept(visitors::visitor*v) { v->visit(dynamic_cast<C*>(this)); }
					int class_id() const {
						static int _=base::_class_id_counter();
						return _;
					}
					const char* class_name() const {
						return typeid(C).name();
					}
					virtual bool is_same(const base* i) const {
						return	!i
								? !this
								: class_id()==i->class_id()
									&&
								  equal_to<C>()(dynamic_cast<const C*>(this),
												dynamic_cast<const C*>(i));
					}
					virtual bool is_less(const base* i) const {
						return	!i
								? false
								: class_id()<i->class_id()
									||
								  (class_id()==i->class_id()
								    &&
								  less_than<C>()(dynamic_cast<const C*>(this),
												 dynamic_cast<const C*>(i)));
					}
			};

		namespace token {
			class base : public item::base {
				/*virtual ast_node_t recognize(parse_context_t t) const = 0;*/
			};

			template <class C>
				class impl
				: public item_with_class_id<C>
			{};

			class Re
			: public impl<Re> {
				private:
					const char* pattern_;
					RE_TYPE cache;
				public:
					Re(const char*reg_expr) : pattern_(reg_expr) {
					int error_ofs;
					const char* error;
						cache = pcre_compile(reg_expr, 0, &error, &error_ofs, NULL);
						if(error) {
							std::cerr << "Error : regex compilation of \"" << reg_expr << "\" failed (" << error << " at #" << error_ofs << ")" << std::endl;
							throw std::string();
						}
					}
					Re(const Re& _) : pattern_(_.pattern_), cache(_.cache) {}
					~Re() { if(cache) { pcre_free(cache); } }
					const char* pattern() const { return pattern_; }
					/*virtual ast_node_t recognize(parse_context_t t) const;*/
			};

			class T : public impl<T> {
				private:
					const char* str_;
				public:
					T(const char*s) : str_(s) {}
					T(const T& _) : str_(_.str_) {}
					const char* str() const { return str_; }
					/*virtual ast_node_t recognize(parse_context_t t) const;*/
			};

			class Comment : public impl<Comment> {
				private:
					const char* str_;
				public:
					Comment(const char*s) : str_(s) {}
					Comment(const Comment& _) : str_(_.str_) {}
					const char* str() const { return str_; }
					/*virtual ast_node_t recognize(parse_context_t t) const {*/
						/*return NULL;*/
					/*}*/
			};

			class Str : public impl<Str> {
				private:
					const char* start_, * end_;
				public:
					Str(const char* s, const char* e) : start_(s), end_(e) {}
					Str(const Str& _) : start_(_.start_), end_(_.end_) {}
					const char* start() const { return start_; }
					const char* end() const { return end_; }
					/*virtual ast_node_t recognize(parse_context_t t) const;*/
			};

			class Epsilon : public impl<Epsilon> {
				public:
					Epsilon() {}
					/*virtual ast_node_t recognize(parse_context_t t) const {*/
						/*return PRODUCTION_OK_BUT_EMPTY;*/
					/*}*/
			};

			class Eof : public impl<Eof> {
				public:
					Eof() {}
					/*virtual ast_node_t recognize(parse_context_t t) const {*/
						/*return t->ofs==t->size?PRODUCTION_OK_BUT_EMPTY:NULL;*/
					/*}*/
			};

			class Bow : public impl<Bow> {
				private:
					const char* tag_;
					bool keep_;
				public:
					Bow(const char*_, bool k) : tag_(_), keep_(k) {}
					Bow(const Bow& _) : tag_(_.tag_), keep_(_.keep_) {}
					const char* tag() const { return tag_; }
					bool keep() const { return keep_; }
					/*virtual ast_node_t recognize(parse_context_t t) const;*/
			};

			class Nt : public impl<Nt> {
				private:
					const char* tag_;
				public:
					Nt(const char*_) : tag_(_) {}
					Nt(const Nt& _) : tag_(_.tag_) {}
					const char* tag() const { return tag_; }
					/*virtual ast_node_t recognize(parse_context_t t) const;*/
			};
		}
	}

	namespace rule {
		class base : public item::base, public std::set<item::base*> {
			private:
				const char* tag_;
			public:
				enum reduction_mode {
					Subtree,
					List,
					Red_Prefix,
					Red_Postfix
				};
				base() : tag_(NULL) {}
				base(const char* name, item::base* _, Grammar*g) { init(name, _, g); }
				~base() {
					/*std::set<item::base*>::iterator i, j = end();*/
					/*for(i=begin();i!=j;++i) {*/
						/*delete *i;*/
					/*}*/
				}
				void init(const char* name, item::base* _, Grammar*);
				const char* tag() const { return tag_; }
				void tag(const char*x) { tag_=x; }
				template <class X>
				static const char* auto_tag() {
					static int counter=0;
					int status;
					char*buffy = abi::__cxa_demangle(typeid(X).name(), 0, 0, &status);
					/*std::cout << "got buffy="<<buffy<<std::endl;*/
					char*p=buffy+strlen(buffy)-1;
					while(p>buffy&&*p!=':') {
						--p;
					}
					std::stringstream ss;
					ss << (++p) << '_' << (++counter);
					free(buffy);
					return regstr(ss.str().c_str());
				}
				virtual reduction_mode mode() const = 0;
		};

		template <class C>
			class rule_impl : public item::item_with_class_id<C, base> {
				public:
					rule_impl(const char* name, item::base* _, Grammar* g) { base::init(name, _, g); }
		};

		class Operator : public rule_impl<Operator> {
			public:
				Operator(const char*tag, item::base* contents, Grammar* g)
					: rule_impl<Operator>(tag, contents, g)
				{}
				virtual reduction_mode mode() const { return Subtree; }
		};

		class Transient : public rule_impl<Transient> {
			public:
				Transient(const char* tag, item::base* contents, Grammar* g)
					: rule_impl<Transient>(tag, contents, g)
				{}
				virtual reduction_mode mode() const { return List; }
		};

		class Postfix : public Transient {
			public:
				Postfix(const char* tag, item::base* contents, Grammar* g) :
					Transient(tag, contents, g)
				{}
				virtual reduction_mode mode() const { return Red_Postfix; }
		};

		class Prefix : public Transient {
			public:
				Prefix(const char* tag, item::base* contents, Grammar* g) :
					Transient(tag, contents, g)
				{}
				virtual reduction_mode mode() const { return Red_Prefix; }
		};
	}

	namespace item {
		namespace iterators {
			class iterator_map : public _iterator_base {
				private:
					map_type::const_iterator seq, begin, end;
					size_t sz;
				public:
					iterator_map(const base*c) : _iterator_base(c) {
						/*std::cout << "iterator_map" << std::endl;*/
						const map_type* v = dynamic_cast<const map_type*>(c);
						sz = v->size();
						begin = v->begin();
						end = v->end();
						seq = begin;
					}
					virtual void inc() { ++seq; }
					virtual void dec() {}
					virtual const base* get() const { return seq==end?NULL:dynamic_cast<const base*>((*seq).second); }
					virtual bool at_start() const { return seq==begin; }
					virtual bool at_end() const { return seq==end; }
					virtual size_t len() const { return sz; }
					virtual bool eq(const _iterator_base*i) const {
						const iterator_map*is = dynamic_cast<const iterator_map*>(i);
						return !!is && seq==is->seq;
					}
					virtual bool inf(const _iterator_base*i) const {
						const iterator_map*is = dynamic_cast<const iterator_map*>(i);
						return !!is && *seq<*is->seq;
					}
					virtual _iterator_base* clone() const {
						iterator_map* c = new iterator_map(context());
						c->seq = seq;
						c->begin = begin;
						c->end = end;
						c->sz = sz;
						return c;
					}
			};
		}
	}


	class Grammar : public map_type, public item::item_with_class_id<Grammar> {
		public:
			Grammar(ast_node_t rules);
			~Grammar();
			void add_rule(const char* tag, rule::base* rule) {
				if(!rule) {
					return;
				}
				/*visitors::debugger debug;*/
				/*std::cout << "Grammar{"<<this<<"}->add_rule("<<rule->tag()<<")"<<std::endl;*/
				Grammar::iterator exist = find((char*)tag);
				if(exist!=end()) {
					/* merge contents */
					exist->second->insert(rule->begin(), rule->end());
				} else {
					(*this)[tag] = rule;
					/*insert(value_type(tag, rule));*/
				}
				/*std::cout << "Grammar{"<<this<<"}->add_rule("<<rule->tag()<<") DONE size now = "<<size()<<std::endl;*/
				/*visitors::debugger d;*/
				/*(*this)[tag]->accept(&d);*/
				std::cout << (*this)[tag] << std::endl;
			}
			void add_rule(rule::base* rule) {
				add_rule(rule->tag(), rule);
			}
	};



	namespace item {
		namespace combination {
			class base : public item::base {};
			template <class CLS, class B=base> class impl
				: public item_with_class_id<CLS, B> {
					virtual item::base* contents() const = 0;
					virtual void contents(item::base*) = 0;
				};

			class single_base : public base {
				protected:
					item::base* _;
				public:
					single_base() : base(), _(0) {}
					virtual ~single_base() { /*if(_) { delete _; }*/ }
					virtual item::base* contents() const { return _; }
					virtual void contents(item::base* i) { _=i; }
			};

			template <class CLS> class single : public impl<CLS, base> {
				protected:
					item::base* _;
				public:
					single() : _(0) {}
					virtual ~single() { /*if(_) { delete _; }*/ }
					single(item::base* contents_) { contents(contents_); }
					virtual item::base* contents() const { return _; }
					virtual void contents(item::base* i) { _=i; }
			};

			template <class C> class tagged_single : public single<C> {
					protected:
						const char* _t;
					public:
						tagged_single(const char* tag, item::base* contents) : single<C>(contents), _t(tag) {}
						const char* tag() const { std::cout << "asking for tag in tagged_single => " << _t << std::endl; return _t; }
			};


			template <class S>
				class seq_base : public impl<S>, public std::vector<item::base*> {
					public:
						virtual item::base* contents() const { return (item::base*)this; }
						virtual ~seq_base() {
							/*std::vector<item::base*>::iterator i, j = end();*/
							/*for(i=begin();i!=j;++i) {*/
								/*delete *i;*/
							/*}*/
						}
						virtual void contents(item::base* i) {}
				};

			class Seq : public seq_base<Seq> {};
			class RawSeq : public seq_base<RawSeq> {};
			class Alt : public impl<Alt>, public std::set<item::base*> {
					public:
						virtual ~Alt() {
							/*std::set<item::base*>::iterator i, j = end();*/
							/*for(i=begin();i!=j;++i) {*/
								/*delete *i;*/
							/*}*/
						}
						virtual item::base* contents() const { return (item::base*)this; }
						virtual void contents(item::base* i) {}
				};

			class Rep01 : public single<Rep01> {
				public:
					Rep01(item::base* _i) : single<Rep01>(_i) { std::cout << "Rep01 contents="<<_i<<std::endl; _=_i; }
					virtual item::base* contents() const { std::cout << "asking for contents ? got " << _ << std::endl; return _; }
					virtual void contents(item::base* i) { _=i; }
			};

			class Rep0N : public single<Rep0N> {
				public:
					Rep0N(item::base* contents) : single<Rep0N>(contents) {}
					virtual item::base* contents() const { return _; }
					virtual void contents(item::base* i) { _=i; }
			};

			class Rep1N : public single<Rep1N> {
				public:
					Rep1N(item::base* contents) : single<Rep1N>(contents) {}
					virtual item::base* contents() const { return _; }
					virtual void contents(item::base* i) { _=i; }
			};

			class Prefix : public tagged_single<Prefix> {
				public:
					Prefix(item::base* prefix, const char* nt) : tagged_single<Prefix>(nt, prefix) {}
					virtual item::base* contents() const { return _; }
					virtual void contents(item::base* i) { _=i; }
			};

			class Postfix : public tagged_single<Postfix> {
				public:
					Postfix(item::base* postfix, const char* nt) : tagged_single<Postfix>(nt, postfix) {}
					virtual item::base* contents() const { return _; }
					virtual void contents(item::base* i) { _=i; }
			};
		}
	}
}



#include "lr_equal_to.h"
#include "lr_less_than.h"

#endif
