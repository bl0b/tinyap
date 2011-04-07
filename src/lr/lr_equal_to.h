#ifndef _TINYAP_LR_EQUAL_TO_H_
#define _TINYAP_LR_EQUAL_TO_H_

namespace grammar {
	namespace item {
		template <> struct equal_to<Grammar> {
			bool operator() (const Grammar* a, const Grammar*b) const { return *a==*b; }
		};

		template <> struct equal_to<rule::Transient> {
			bool operator() (const rule::Transient* a, const rule::Transient*b) const {
				if(a->tag()!=b->tag()) {
					return false;
				}
				if(a->size()!=b->size()) {
					return false;
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return false;
					}
					++ia;
					++ib;
				}
				return true;
			}
		};

		template <> struct equal_to<rule::Operator> {
			bool operator() (const rule::Operator* a, const rule::Operator*b) const {
				if(a->tag()!=b->tag()) {
					return false;
				}
				if(a->size()!=b->size()) {
					return false;
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return false;
					}
					++ia;
					++ib;
				}
				return true;
			}
		};

		template <> struct equal_to<rule::Prefix> {
			bool operator() (const rule::Prefix* a, const rule::Prefix*b) const {
				if(a->tag()!=b->tag()) {
					return false;
				}
				if(a->size()!=b->size()) {
					return false;
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return false;
					}
					++ia;
					++ib;
				}
				return true;
			}
		};

		template <> struct equal_to<rule::Postfix> {
			bool operator() (const rule::Postfix* a, const rule::Postfix*b) const {
				if(a->tag()!=b->tag()) {
					return false;
				}
				if(a->size()!=b->size()) {
					return false;
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return false;
					}
					++ia;
					++ib;
				}
				return true;
			}
		};

		template <> struct equal_to<token::Eof> {
			bool operator() (const token::Eof* a, const token::Eof*b) const { return true; }
		};

		template <> struct equal_to<token::Epsilon> {
			bool operator() (const token::Epsilon* a, const token::Epsilon*b) const { return true; }
		};

		template <> struct equal_to<token::Re> {
			bool operator() (const token::Re* a, const token::Re*b) const { return !strcmp(a->pattern(), b->pattern()); }
		};

		template <> struct equal_to<token::T> {
			bool operator() (const token::T* a, const token::T*b) const { return !strcmp(a->str(), b->str()); }
		};

		template <> struct equal_to<token::Comment> {
			bool operator() (const token::Comment* a, const token::Comment*b) const { return !strcmp(a->str(), b->str()); }
		};

		template <> struct equal_to<token::Nt> {
			bool operator() (const token::Nt* a, const token::Nt*b) const { return !strcmp(a->tag(), b->tag()); }
		};

		template <> struct equal_to<token::Bow> {
			bool operator() (const token::Bow* a, const token::Bow*b) const { return a->keep()==b->keep() && !strcmp(a->tag(), b->tag()); }
		};

		template <> struct equal_to<token::AddToBag> {
			bool operator() (const token::AddToBag* a, const token::AddToBag*b) const { return a->keep()==b->keep() && !(strcmp(a->tag(), b->tag()) || strcmp(a->pattern(), b->pattern())); }
		};

		template <> struct equal_to<token::Str> {
			bool operator() (const token::Str* a, const token::Str*b) const { return !(strcmp(a->start(), b->start())||strcmp(a->end(), b->end())); }
		};



		template <> struct equal_to<combination::Alt> {
			bool operator() (const combination::Alt* a, const combination::Alt*b) const { return *a == *b; }
		};

		template <> struct equal_to<combination::Seq> {
			bool operator() (const combination::Seq* a, const combination::Seq*b) const {
				if(a->size()!=b->size()) {
					return false;
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return false;
					}
					++ia;
					++ib;
				}
				return true;
			}
		};

		template <> struct equal_to<combination::RawSeq> {
			bool operator() (const combination::RawSeq* a, const combination::RawSeq*b) const {
				if(a->size()!=b->size()) {
					return false;
				}
				/*iterator ia = iterator::create(a), ib = iterator::create(b);*/
				combination::RawSeq::const_iterator ia = a->begin(), ib = b->begin(), ja = a->end(), jb = b->end();
				while(ia!=ja && ib!=jb) {
					if(!(*ia)->is_same(*ib)) {
						return false;
					}
					++ia;
					++ib;
				}
				return ia==ja && ib==jb;
			}
		};

	template <> struct equal_to<combination::Postfix > {
			bool operator() (const combination::Postfix* a, const combination::Postfix*b) const { return a->tag()==b->tag() && a->contents()==b->contents(); }
		};

		template <> struct equal_to<combination::Prefix > {
			bool operator() (const combination::Prefix* a, const combination::Prefix*b) const { return a->tag()==b->tag() && a->contents()==b->contents(); }
		};

		template <> struct equal_to<combination::Rep01 > {
			bool operator() (const combination::Rep01* a, const combination::Rep01*b) const { return a->contents()==b->contents(); }
		};

		template <> struct equal_to<combination::Rep0N > {
			bool operator() (const combination::Rep0N* a, const combination::Rep0N*b) const { return a->contents()==b->contents(); }
		};

		template <> struct equal_to<combination::Rep1N > {
			bool operator() (const combination::Rep1N* a, const combination::Rep1N*b) const { return a->contents()==b->contents(); }
		};

	}
}

#endif

