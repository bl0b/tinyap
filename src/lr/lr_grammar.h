#ifndef _TINYAP_LR_GRAMMAR_H_
#define _TINYAP_LR_GRAMMAR_H_

#include <cxxabi.h>		/* for demangling */

#include "string_registry.h"

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
				virtual std::pair<ast_node_t, unsigned int> recognize(const char*, unsigned int, unsigned int) const = 0;
		};
	}
}

namespace std {
	template <>
		struct less<grammar::item::base*> {
			bool operator()(grammar::item::base* a, grammar::item::base* b) const { return a->is_less(b); }
		};
}

extern "C" void escape_ncpy(char**dest, char**src, int count, const char* delim);

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
				int length() const { return impl->len(); }

				bool operator<(const iterator&i_) const {
					/*std::clog << "{{iterator::inf : context " << (impl->context()==i_.impl->context()) << " impl->inf " << (impl->inf(i_.impl)) << "}}";*/
					return impl->context()<i_.impl->context() || (impl->context()==i_.impl->context() && impl->inf(i_.impl));
				}

				bool operator==(const iterator&i_) const {
					/*std::clog << "{{iterator::eq : context " << (impl->context()==i_.impl->context()) << " impl->eq " << (impl->eq(i_.impl)) << "}}";*/
					return impl->context()==i_.impl->context() && impl->eq(i_.impl);
				}

				bool operator!=(const iterator&i_) const {
					/*std::clog << "{{iterator::neq : !context " << (impl->context()!=i_.impl->context()) << " !impl->eq " << (!impl->eq(i_.impl)) << "}}";*/
					return impl->context()!=i_.impl->context() || !impl->eq(i_.impl);
				}

				static iterator create(const item::base*);
		};

		namespace iterators {
			class iterator_epsilon : public _iterator_base {
				public:
					iterator_epsilon(const base*c) : _iterator_base(c) {
						/*std::clog << "iterator_epsilon" << std::endl;*/
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
						/*std::clog << "iterator_single" << std::endl;*/
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
						/*std::clog << "iterator_vector" << std::endl;*/
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
						/*std::clog << "iterator_vector::eq ";*/
						return !!is && begin==is->begin && end==is->end && seq==is->seq;
					}
					virtual bool inf(const _iterator_base*i) const {
						const iterator_vector*is = dynamic_cast<const iterator_vector*>(i);
						/*std::clog << "iterator_vector::inf ";*/
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
						/*std::clog << "iterator_set" << std::endl;*/
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
						/*std::clog << "iterator_set::eq" << std::endl;*/
						return !!is && begin==is->begin && end==is->end && seq==is->seq;
					}

					virtual bool inf(const _iterator_base*i) const {
						const iterator_set*is = dynamic_cast<const iterator_set*>(i);
						/*std::clog << "iterator_set::inf" << std::endl;*/
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
					item_with_class_id() { /*std::clog << "new object (cid=" << class_id() << ')' << std::endl;*/ }
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

			template <class B>
				class Re_base : public impl<B> {
					private:
						const char* pattern_;
						RE_TYPE cache;
					protected:
						virtual ast_node_t publish(const char* match, unsigned int offset) const = 0;
					public:
						Re_base(const char*reg_expr) : pattern_(reg_expr) {
							int error_ofs;
							const char* error;
							cache = pcre_compile(reg_expr, PCRE_NEWLINE_ANY, &error, &error_ofs, NULL);
							if(error) {
								std::cerr << "Error : regex compilation of \"" << reg_expr
									<< "\" failed (" << error << " at #" << error_ofs << ")" << std::endl;
								throw std::string();
							}
						}
						/*Re(const Re& _) : pattern_(_.pattern_), cache(_.cache) {}*/
						virtual ~Re_base() { if(cache) { pcre_free(cache); } }
						const char* pattern() const { return pattern_; }
						virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
							int token[3];
							if(re_exec(cache, source, offset, size, token, 3)) {
								char*lbl=match2str(source+offset,0,token[1], "/");
								/*return std::pair<ast_node_t, unsigned int>(newAtom(lbl, offset), offset+token[1]);*/
								return std::pair<ast_node_t, unsigned int>(publish(lbl, offset), offset+token[1]);
							} else {
								return std::pair<ast_node_t, unsigned int>(NULL, offset);
							}
						}
				};

			class Re : public Re_base<Re> {
				protected:
					virtual ast_node_t publish(const char* match, unsigned int offset) const {
						return newPair(newAtom(match, offset), NULL);
					}
				public:
					Re(const char*p) : Re_base<Re>(p) {}
			};

			class T : public impl<T> {
				private:
					const char* str_;
					size_t slen;
				public:
					T(const char*s) : str_(s), slen(strlen(s)) {}
					T(const T& _) : str_(_.str_) {}
					const char* str() const { return str_; }
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						if(size>=(offset+slen)&&!strncmp(str_, source+offset, slen)) {
							return std::pair<ast_node_t, unsigned int>(PRODUCTION_OK_BUT_EMPTY, offset+slen);
						}
						return std::pair<ast_node_t, unsigned int>(NULL, offset);
					}
			};

			class Comment : public impl<Comment> {
				private:
					const char* str_;
				public:
					Comment(const char*s) : str_(s) {}
					Comment(const Comment& _) : str_(_.str_) {}
					const char* str() const { return str_; }
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						return std::pair<ast_node_t, unsigned int>(PRODUCTION_OK_BUT_EMPTY, offset);
					}
			};

			class Str : public impl<Str> {
				private:
					const char* delim_start, * delim_end;
					unsigned int sslen, eslen;
				public:
					Str(const char* s, const char* e) : delim_start(s), delim_end(e), sslen(strlen(s)), eslen(strlen(e)) {}
					Str(const Str& _) : delim_start(_.delim_start), delim_end(_.delim_end) {}
					const char* start() const { return delim_start; }
					const char* end() const { return delim_end; }
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						char* _src;
						char* _end;
						char* _match;
						char* ret;
						unsigned int ofs = offset;
						_src = (char*)(ofs+source);
						if(strncmp(delim_start, source+ofs, sslen)) {
							/*std::cout << "[DEBUG:Str] didn't match start" << std::endl;*/
							return std::pair<ast_node_t, unsigned int>(NULL, offset);
						}
						_src += sslen;
						if(!*delim_end) {
							/*std::cout << "[DEBUG:Str] match to EOS" << std::endl;*/
							/*_match = ret = _stralloc(source+size-_src+1);*/
							/*escape_ncpy(&_match, &_src, source+size-_src, delim_end);*/
							_match = ret = match2str(source, 0, source+size-_src, delim_end);
							ofs = size;
						} else {
							for(_end = _src;;) {
								_match = strstr(_end, delim_end);
								if(!_match) {
									/*std::cout << "[DEBUG:Str] didn't match end." << std::endl;*/
									if(_end==_src) {
										/*std::cout << "[DEBUG:Str] didn't match anything at all." << std::endl;*/
										return std::pair<ast_node_t, unsigned int>(NULL, ofs);
									}
									break;
								}
								if(_match>_src&&*(_match-1)=='\\') {
									/*std::cout << "[DEBUG:Str] skipping escaped end" << std::endl;*/
									_end = _match+eslen;
									continue;
								} else {
									/*std::cout << "[DEBUG:Str] matched unescaped end" << std::endl;*/
									_end = _match;
									break;
								}
							}
							
							/*while((_match=strchr(_end, (int)*delim_end))&&_match>_end&&*(_match-1)=='\\') {*/
								/*_end = _match+1;*/
							/*}*/
							/*if(_end==_src&&!_match) {*/
								/*return std::pair<ast_node_t, unsigned int>(NULL, ofs);*/
							/*} else if(_match) {*/
								/*_end = _match;*/
							/*}*/
							//ret = _stralloc(_end-_src+1);
							//_match = ret;
							//escape_ncpy(&_match, &_src, _end-_src, delim_end);
							ret = match2str(_src, 0, _end-_src, delim_end);
							//*_match=0;
							ofs = _end-source+eslen;
						}
						/*printf(__FILE__ ":%i\n", __LINE__);*/
						return std::pair<ast_node_t, unsigned int>(newPair(newAtom(ret, offset), NULL), ofs);
					}
			};

			class Epsilon : public impl<Epsilon> {
				public:
					Epsilon() {}
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						return std::pair<ast_node_t, unsigned int>(PRODUCTION_OK_BUT_EMPTY, offset);
					}
					static Epsilon* instance() {
						/* FIXME rename pouet */
						/* TODO fix this FIXME */
						static Epsilon pouet;
						return &pouet;
					}
			};

			class Eof : public impl<Eof> {
				public:
					Eof() {}
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						if(offset==size) {
							return std::pair<ast_node_t, unsigned int>(PRODUCTION_OK_BUT_EMPTY, offset);
						} else {
							return std::pair<ast_node_t, unsigned int>(NULL, offset);
						}
					}
			};

			class Bow : public impl<Bow> {
				private:
					const char* tag_;
					bool keep_;
					trie_t mybow;
					static ext::hash_map<const char*, trie_t>& all;
				public:
					Bow(const char*_, bool k) : tag_(_), keep_(k) {
						mybow = find(_);
						/*std::clog << "using BOW " << tag_ << " (from " << _ << ") @" << mybow << std::endl;*/
					}
					Bow(const Bow& _) : tag_(_.tag_), keep_(_.keep_), mybow(_.mybow) {}
					const char* tag() const { return tag_; }
					bool keep() const { return keep_; }
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						/*std::clog << "POUET " << tag_ << " @" << mybow << std::endl;*/
						/*trie_dump(mybow, 1);*/
						unsigned long slen = trie_match_prefix(mybow, source+offset);
						if(slen>0) {
							/*std::clog << "found a word in bow " << tag_ << " : " << std::string(source+offset, source+offset+slen) << std::endl;*/
							if(!keep_) {
								return std::pair<ast_node_t, unsigned int>(PRODUCTION_OK_BUT_EMPTY, offset+slen);
							} else {
								char*tok = _stralloc(slen+1);
								strncpy(tok, source+offset, slen);
								tok[slen]=0;
								ast_node_t ast = newPair(newAtom(tok, offset), NULL);
								_strfree(tok);
								return std::pair<ast_node_t, unsigned int>(ast, offset+slen);
							}
						/*} else {*/
							/*std::clog << "didn't find any word in bow " << tag_ << std::endl;*/
						}
						return std::pair<ast_node_t, unsigned int>(NULL, offset);
					}

					static trie_t find(const char*tag) {
						/*static struct tries : ext::hash_map<const char*, trie_t> {
							~tries() {
								iterator i, j;
								for(i=begin(), j=end(); i!=j; ++i) {
									trie_free((*i).second);
								}
							}
						} all;*/
						trie_t ret = all[tag];
						if(!ret) {
							all[tag] = ret = trie_new();
						}
						return ret;
					}
			};

			class AddToBag : public Re_base<AddToBag> {
				protected:
					bool keep_;
					trie_t bag;
					const char* tag_;
					virtual ast_node_t publish(const char* match, unsigned int offset) const {
						trie_insert(bag, match);
						if(keep_) {
							return newPair(newAtom(match, offset), NULL);
						} else {
							return PRODUCTION_OK_BUT_EMPTY;
						}
					}
				public:
					AddToBag(const char* p, const char* b, bool k) : Re_base<AddToBag>(p), keep_(k), bag(Bow::find(b)), tag_(b) {
						/*std::cout << "NEW AddToBag(" << pattern() << ", " << tag() << ", " << keep() << std::endl;*/
					}
					bool keep() const { return keep_; }
					const char* tag() const { return tag_; }
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						std::pair<ast_node_t, unsigned int> ret = Re_base<AddToBag>::recognize(source, offset, size);
						/*std::cout << "AddToBag => " << ret.first << " +" << ret.second << std::endl;*/
						if(ret.second>offset) {
							trie_insert(bag, Value(Car(ret.first)));
						}
						if(keep_) {
							return ret;
						} else {
							return std::pair<ast_node_t, unsigned int>(PRODUCTION_OK_BUT_EMPTY, ret.second);
						}
					}
			};

		}
	}

	namespace rule {
		class base : public item::base, public std::set<item::base*> {
			private:
				const char* tag_;
				static int counter() {
					static int i=0;
					return ++i;
				}
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
					int status;
					char*buffy = abi::__cxa_demangle(typeid(X).name(), 0, 0, &status);
					/*std::clog << "got buffy="<<buffy<<std::endl;*/
					char*p=buffy+strlen(buffy)-1;
					while(p>=buffy&&*p!=':') {
						--p;
					}
					std::stringstream ss;
					ss << (++p) << '_' << counter();
					free(buffy);
					return regstr(ss.str().c_str());
				}
				/*virtual reduction_mode mode() const = 0;*/
				virtual const bool keep_empty() const = 0;
				virtual ast_node_t reduce_ast(ast_node_t ast, unsigned int offset) const = 0;
				virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
					return std::pair<ast_node_t, unsigned int>(NULL, offset);
				}
		};

		template <class C>
			class rule_impl : public item::item_with_class_id<C, base> {
				public:
					rule_impl(const char* name, item::base* _, Grammar* g) { base::init(name, _, g); }
		};

		namespace internal {
			struct append {
				ast_node_t operator()(ast_node_t a, ast_node_t b, bool drop_empty = true) const {
					std::cerr << "append <= " << a << " + " << b << std::endl;
					if(a==PRODUCTION_OK_BUT_EMPTY||!a) { return b?b:PRODUCTION_OK_BUT_EMPTY; }
					if(b==PRODUCTION_OK_BUT_EMPTY||!b) { return a; }
					ast_node_t ret = rec(a, b, drop_empty);
					if(a!=ret&&b!=ret) {
						/*delete_node(a);*/
						/*delete_node(b);*/
					}
					/*delete_node(a);*/
					std::cerr << "append => " << ret << std::endl;
					return ret;
				}
				private:
					ast_node_t rec(ast_node_t a, ast_node_t b, bool drop_empty) const {
						/*std::cout << "rec a:" << (a?a->raw.ref:-1) << "=" << a << " b:" << (b?b->raw.ref:-1) << "=" << b << std::endl;*/
						if(!a) {
							/*std::cout << "rec return b" << std::endl;*/
							return b;
						}
						ast_node_t tail = rec(Cdr(a), b, drop_empty);
						if(drop_empty && Car(a)==PRODUCTION_OK_BUT_EMPTY) {
							/*std::cout << "rec return tail " << tail << std::endl;*/
							return tail;
						}
						ast_node_t ret = newPair(Car(a), tail);
						/*std::cout << "rec return new pair " << ret << std::endl;*/
						return ret;
					}
			};
			struct pfx_extract : public append {
				ast_node_t rule, pfx, tag, cdr;
				pfx_extract(ast_node_t ast) {
					struct {
						/* store last valid car(ast) in rule, prefix is the rest. */
						ast_node_t operator()(ast_node_t ast, ast_node_t&rule) const {
							if(Cdr(ast)) {
								return newPair(Car(ast), (*this)(Cdr(ast), rule));
							}
							rule = Car(ast);
							/*rule->raw.ref++;*/
							return NULL;
						}
					} dirty_extraction;
					/*std::clog << "PFX EXTRACTOR : " << ast << std::endl;*/
					pfx = dirty_extraction(ast, rule);
					/*pfx = Car(ast);*/
					/*rule = Cdr(ast);*/
					/*while(Cdr(rule)) {*/
						/*rule = Cdr(rule);*/
					/*}*/
					/*rule = Car(rule);*/
					/*std::clog << "rule = " << rule << std::endl;*/
					tag = Car(rule);
					/*tag->raw.ref++;*/
					/*std::clog << "tag = " << tag << std::endl;*/
					cdr = Cdr(rule);
					/*cdr->raw.ref++;*/
					/*delete_node(ast);*/
					/*std::clog << "cdr = " << cdr << std::endl;*/
				}
				ast_node_t prefix() const { return newPair(tag, internal::append()(pfx, cdr)); }
				ast_node_t postfix() const { return newPair(tag, internal::append()(cdr, pfx)); }
			};
		}

		class Operator : public rule_impl<Operator> {
			public:
				Operator(const char*tag, item::base* contents, Grammar* g)
					: rule_impl<Operator>(tag, contents, g)
				{}
				/*virtual reduction_mode mode() const { return Subtree; }*/
				virtual ast_node_t reduce_ast(ast_node_t ast, unsigned int offset) const {
					/*return newPair(newAtom(tag(), offset), ast==PRODUCTION_OK_BUT_EMPTY?NULL:ast);*/
					/*return internal::append()(newPair(newAtom(tag(), offset), NULL), ast);*/
					/*std::clog << "Operator reduction" << std::endl;*/
					return newPair(newPair(newAtom(tag(), offset), ast==PRODUCTION_OK_BUT_EMPTY?NULL:ast), NULL);
					/*return newPair(internal::append()(newPair(newAtom(tag(), offset), NULL), ast), NULL);*/
					/*return newPair(newPair(newAtom(tag(), offset), ast==PRODUCTION_OK_BUT_EMPTY?NULL:ast), NULL);*/
				}
				virtual const bool keep_empty() const { return false; }
		};

		class Transient : public rule_impl<Transient> {
			public:
				Transient(const char* tag, item::base* contents, Grammar* g)
					: rule_impl<Transient>(tag, contents, g)
				{}
				/*virtual reduction_mode mode() const { return List; }*/
				virtual ast_node_t reduce_ast(ast_node_t ast, unsigned int offset) const {
					/*std::clog << "Transient reduction" << std::endl;*/
					/*if(ast) { ast->raw.ref++; }*/
					return ast;
				}
				virtual const bool keep_empty() const { return false; }
		};

		class Postfix : public Transient {
			public:
				Postfix(const char* tag, item::base* contents, Grammar* g) :
					Transient(tag, contents, g)
				{}
				/*virtual reduction_mode mode() const { return Red_Postfix; }*/
				virtual ast_node_t reduce_ast(ast_node_t ast, unsigned int offset) const {
					/*std::clog << "Postfix reduce_ast has " << ast << std::endl;*/
					return newPair(internal::pfx_extract(ast).postfix(), NULL);
				}
				virtual const bool keep_empty() const { return true; }
		};

		class Prefix : public Transient {
			public:
				Prefix(const char* tag, item::base* contents, Grammar* g) :
					Transient(tag, contents, g)
				{}
				/*virtual reduction_mode mode() const { return Red_Prefix; }*/
				virtual ast_node_t reduce_ast(ast_node_t ast, unsigned int offset) const {
					/*std::clog << "Prefix reduce_ast has " << ast << std::endl;*/
					return newPair(internal::pfx_extract(ast).prefix(), NULL);
				}
				virtual const bool keep_empty() const { return true; }
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
						/*std::clog << "iterator_map" << std::endl;*/
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

	namespace item {
		namespace token {
			class Nt : public impl<Nt> {
				private:
					static ext::hash_map<const char*, Nt*>& registry;
					const char* tag_;
				public:
					Nt(const char*_) : tag_(_) {}
					Nt(const Nt& _) : tag_(_.tag_) {}
					const char* tag() const { return tag_; }
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						return std::pair<ast_node_t, unsigned int>(NULL, offset);
					}
#if 0
					virtual bool is_same(const base* i) const {
						if(!i) {
							return !this;
						}
						if(class_id()==i->class_id()) {
							return equal_to<Nt>()(
										dynamic_cast<const Nt*>(this),
										dynamic_cast<const Nt*>(i));
						}
						const grammar::rule::base* R =
							dynamic_cast<const grammar::rule::base*>(i);
						if(R) {
							return tag()==R->tag();
						}
						return false;
					}
#endif
					static Nt* instance(const char*tag) {
						/*static struct internal_registry : public ext::hash_map<const char*, Nt*> {
							~internal_registry() {
								internal_registry::iterator i, j=end();
								for(i=begin();i!=j;++i) {
									delete i->second;
								}
							}
						} registry;*/
						Nt* ret = registry[tag];
						/*std::clog << "(NT) requesting Nt(\"" << tag << "\")" << std::endl;*/
						if(!ret) {
							ret = registry[tag] = new Nt(regstr(tag));
							/*std::clog << "(NT)   created Nt(\"" << tag << "\") @" << ret << std::endl;*/
						}
						/*std::clog << "(NT)   returning @" << ret << std::endl;*/
						return ret;
					}
			};
		}
	}


	class Grammar : public map_type, public item::item_with_class_id<Grammar> {
		public:
			struct _whitespace {
				const char* s;
				_whitespace(const char*_) : s(_) {}
				virtual ~_whitespace() {}
				virtual unsigned int trim(const char* source, unsigned int offset, unsigned int size) = 0;
			};

			struct ws_str : public _whitespace {
				ws_str(const char*_) : _whitespace(_) {}
				virtual unsigned int trim(const char* source, unsigned int offset, unsigned int size) {
					while(offset<size&&strchr(s, *(source+offset))) {
						++offset;
					}
					return offset;
				}
			};

			struct ws_re : public _whitespace {
				RE_TYPE cache;
				ws_re(const char*_) : _whitespace(_) {
					const char* error;
					int error_ofs;
					cache = pcre_compile(_, 0, &error, &error_ofs, NULL);
				}
				~ws_re() { pcre_free(cache); }
				virtual unsigned int trim(const char* source, unsigned int offset, unsigned int size) {
					int token[3];
					if(re_exec(cache, source, offset, size, token, 3)) {
						return offset+token[1];
					}
					return offset;
				}
			};

			unsigned int skip(const char* ptr, unsigned int ofs, unsigned int sz) {
				return ws->trim(ptr, ofs, sz);
			}

			_whitespace* ws;

			Grammar(ast_node_t rules);
			~Grammar();
			void add_rule(const char* tag, rule::base* rule) {
				if(!rule) {
					return;
				}
				/*visitors::debugger debug;*/
				/*std::clog << "Grammar{"<<this<<"}->add_rule("<<rule->tag()<<")"<<std::endl;*/
				Grammar::iterator exist = find((char*)tag);
				if(exist!=end()) {
					/* merge contents */
					exist->second->insert(rule->begin(), rule->end());
				} else {
					(*this)[tag] = rule;
					/*insert(value_type(tag, rule));*/
				}
				/*std::clog << "Grammar{"<<this<<"}->add_rule("<<rule->tag()<<") DONE size now = "<<size()<<std::endl;*/
				/*visitors::debugger d;*/
				/*(*this)[tag]->accept(&d);*/
				/*std::clog << (*this)[tag] << std::endl;*/
			}
			void add_rule(rule::base* rule) {
				add_rule(rule->tag(), rule);
			}
			virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
				return std::pair<ast_node_t, unsigned int>(NULL, offset);
			}
	};



	namespace item {
		namespace combination {
			class base : public item::base {
				public:
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const {
						return std::pair<ast_node_t, unsigned int>(NULL, offset);
					}
				protected:
					virtual void contents(Grammar*g, item::base*) = 0;
					void contents_commit_helper(Grammar*g, item::base*) const;
			};
			template <class CLS, class B=base> class impl
				: public item_with_class_id<CLS, B> {
				};

#if 0
			class single_base : public base {
				/*protected:*/
					/*item::base* _;*/
				public:
					single_base() : base()/*, _(0)*/ {}
					virtual ~single_base() { /*if(_) { delete _; }*/ }
					virtual item::base* contents() const { /*return _;*/ return NULL; }
					virtual void contents(item::base* i) { /*_=i;*/ }
			};
#endif

			template <class CLS> class single : public impl<CLS, base> {
				protected:
					item::token::Nt* _;
					item::base* cts;
					item::base* raw_cts;
					friend class visitors::debugger;
					virtual void contents(Grammar*g, item::base*x) = 0;
						/*_ = item::token::Nt::instance(rule::base::auto_tag<CLS>());*/
						/*cts = x;*/
					/*}*/
				public:
					single() : _(0), cts(0), raw_cts(0) {}
					virtual ~single() { /*if(_) { delete _; }*/ }
					item::base* contents() const { return _; }
					token::Nt* commit(Grammar*g) const {
						if(raw_cts) {
							base::contents_commit_helper(g, raw_cts);
						}
						g->add_rule(gc(new typename CLS::expansion_rule_type(_->tag(), cts, g)));
						return _;
					}
			};

			template <class C> class tagged_single : public single<C> {
					protected:
						const char* _t;
					public:
						tagged_single(const char* tag) : single<C>(), _t(tag) {}
						const char* tag() const { /*std::clog << "asking for tag in tagged_single => " << _t << std::endl;*/ return _t; }
			};


			template <class S>
				class seq_base : public impl<S>, public std::vector<item::base*> {
					protected:
						virtual void contents(Grammar*g, grammar::item::base*_) {}
					public:
						typedef rule::Transient expansion_rule_type;
						virtual item::base* contents() const { return (item::base*)this; }
						virtual ~seq_base() {
							/*std::vector<item::base*>::iterator i, j = end();*/
							/*for(i=begin();i!=j;++i) {*/
								/*delete *i;*/
							/*}*/
						}
				};

			class Seq : public seq_base<Seq> {
				protected:
					item::token::Nt* _;
				public:
					using seq_base<Seq>::contents;
					Seq() : _(item::token::Nt::instance(rule::base::auto_tag<Seq>())) {}
					Seq* add(item::base* x) { push_back(x); return this; }
					token::Nt* commit(Grammar*g) const {
						g->add_rule(gc(new rule::Transient(_->tag(), (item::base*)this, g)));
						return _;
					}
			};
			class RawSeq : public seq_base<RawSeq> {
				public:
					virtual std::pair<ast_node_t, unsigned int> recognize(const char* source, unsigned int offset, unsigned int size) const;
			};

			class Alt : public single<Alt>, public std::set<item::base*> {
				protected:
					virtual void contents(Grammar*g, item::base*x) {
						_ = item::token::Nt::instance(rule::base::auto_tag<Alt>());
						cts = this;
					}
				public:
					typedef rule::Transient expansion_rule_type;
					using single<Alt>::contents;
					Alt() { contents(NULL, this); }
					/*virtual ~Alt() {*/
						/*std::set<item::base*>::iterator i, j = end();*/
						/*for(i=begin();i!=j;++i) {*/
							/*delete *i;*/
						/*}*/
					/*}*/
					/*virtual item::base* contents() const { return (item::base*)this; }*/
			};

			class Rep01 : public single<Rep01> {
				protected:
					virtual void contents(Grammar*g, item::base*x) {
						/*std::clog << "POUET Rep01" << std::endl;*/
						_ = item::token::Nt::instance(rule::base::auto_tag<Rep01>());
						Alt* alt = gc(new Alt());
						alt->insert(x);
						alt->insert(token::Epsilon::instance());
						/*alt->contents(g, alt);*/
						/*alt->commit(g);*/
						cts = alt;
					}
				public:
					typedef rule::Transient expansion_rule_type;
					using single<Rep01>::contents;
					Rep01(Grammar*g, item::base* _i) : single<Rep01>() { contents(g, _i); }
			};

			class Rep0N : public single<Rep0N> {
				protected:
					virtual void contents(Grammar*g, item::base*x);
				public:
					typedef rule::Transient expansion_rule_type;
					using single<Rep0N>::contents;
					Rep0N(Grammar*g, item::base* ctts) : single<Rep0N>() { contents(g, ctts); }
			};

			class Rep1N : public single<Rep1N> {
				protected:
					virtual void contents(Grammar*g, item::base*x);
				public:
					typedef rule::Transient expansion_rule_type;
					using single<Rep1N>::contents;
					using single<Rep1N>::commit;
					Rep1N(Grammar*g, item::base* ctts) : single<Rep1N>() { contents(g, ctts); }
			};

			class Prefix : public tagged_single<Prefix> {
				protected:
					virtual void contents(Grammar*g, item::base*x) {}
				public:
					typedef rule::Prefix expansion_rule_type;
					using single<Prefix>::contents;
					using single<Prefix>::commit;
					Prefix(Grammar*g, item::base* prefix, const char* nt) : tagged_single<Prefix>(nt) {
						_ = item::token::Nt::instance(rule::base::auto_tag<Prefix>());
						cts = gc(new Seq())->add(prefix)->add(item::token::Nt::instance(nt));
					}
			};

			class Postfix : public tagged_single<Postfix> {
				protected:
					virtual void contents(Grammar*g, item::base*x) {}
				public:
					typedef rule::Postfix expansion_rule_type;
					using single<Postfix>::contents;
					using single<Postfix>::commit;
					Postfix(Grammar*g, item::base* postfix, const char* nt) : tagged_single<Postfix>(nt) {
						_ = item::token::Nt::instance(rule::base::auto_tag<Postfix>());
						cts = gc(new Seq())->add(postfix)->add(token::Nt::instance(nt));
					}
			};
		}
	}
}



#include "lr_equal_to.h"
#include "lr_less_than.h"


#endif
