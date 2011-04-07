#ifndef _TINYAP_LR_LESS_THAN_H_
#define _TINYAP_LR_LESS_THAN_H_

namespace grammar {
	namespace item {

		template <> struct less_than<rule::Prefix > {
			bool operator() (const rule::Prefix* a, const rule::Prefix*b) const {
				if(a->size()!=b->size()) {
					return a->size()<b->size();
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return (*ia)->is_less(*ib);
					}
					++ia;
					++ib;
				}
				return false;
			}
		};

		template <> struct less_than<rule::Postfix > {
			bool operator() (const rule::Postfix* a, const rule::Postfix*b) const {
				if(a->size()!=b->size()) {
					return a->size()<b->size();
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return (*ia)->is_less(*ib);
					}
					++ia;
					++ib;
				}
				return false;
			}
		};

		template <> struct less_than<rule::Transient > {
			bool operator() (const rule::Transient* a, const rule::Transient*b) const {
				if(a->size()!=b->size()) {
					return a->size()<b->size();
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return (*ia)->is_less(*ib);
					}
					++ia;
					++ib;
				}
				return false;
			}
		};

		template <> struct less_than<rule::Operator > {
			bool operator() (const rule::Operator* a, const rule::Operator*b) const {
				if(a->size()!=b->size()) {
					return a->size()<b->size();
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return (*ia)->is_less(*ib);
					}
					++ia;
					++ib;
				}
				return false;
			}
		};

		template <> struct less_than<combination::Postfix > {
			bool operator() (const combination::Postfix* a, const combination::Postfix*b) const { return a->tag()<b->tag() || a->contents()<b->contents(); }
		};

		template <> struct less_than<combination::Alt > {
			bool operator() (const combination::Alt* a, const combination::Alt*b) const { return *a < *b; }
		};

		template <> struct less_than<combination::Seq > {
			bool operator() (const combination::Seq* a, const combination::Seq*b) const {
				if(a->size()!=b->size()) {
					return a->size()<b->size();
				}
				iterator ia = iterator::create(a), ib = iterator::create(b);
				while(!ia.at_end()) {
					if(!(*ia)->is_same(*ib)) {
						return (*ia)->is_less(*ib);
					}
					++ia;
					++ib;
				}
				return false;
			}
		};

		template <> struct less_than<combination::RawSeq > {
			bool operator() (const combination::RawSeq* a, const combination::RawSeq*b) const {
				if(a->size()!=b->size()) {
					return a->size()<b->size();
				}
				/*iterator ia = iterator::create(a), ib = iterator::create(b);*/
				/*while(!ia.at_end()) {*/
					/*if(!(*ia)->is_same(*ib)) {*/
						/*return (*ia)->is_less(*ib);*/
					/*}*/
					/*++ia;*/
					/*++ib;*/
				/*}*/
				combination::RawSeq::const_iterator ia = a->begin(), ib = b->begin(), ja = a->end(), jb = b->end();
				while(ia!=ja && ib!=jb) {
					if(!(*ia)->is_less(*ib)) {
						return false;
					}
					++ia;
					++ib;
				}
				/* ia==ja et ib==jb => eq
				 *   ==        !=   => inf
				 *   !=        ==   => sup
				 *   !=        !=   => wtf
				 */
				return ia==ja && ib!=jb;
			}
		};

		template <> struct less_than<combination::Prefix > {
			bool operator() (const combination::Prefix* a, const combination::Prefix*b) const { return a->tag()<b->tag() || a->contents()<b->contents(); }
		};

		template <> struct less_than<combination::Rep01 > {
			bool operator() (const combination::Rep01* a, const combination::Rep01*b) const { return a->contents()<b->contents(); }
		};

		template <> struct less_than<combination::Rep0N > {
			bool operator() (const combination::Rep0N* a, const combination::Rep0N*b) const { return a->contents()<b->contents(); }
		};

		template <> struct less_than<combination::Rep1N > {
			bool operator() (const combination::Rep1N* a, const combination::Rep1N*b) const { return a->contents()<b->contents(); }
		};

		template <> struct less_than<Grammar> {
			bool operator() (const Grammar* a, const Grammar*b) const { return a->size()<b->size(); }
		};

		template <> struct less_than<token::T> {
			bool operator() (const token::T* a, const token::T*b) const { return strcmp(a->str(), b->str())<0; }
		};

		template <> struct less_than<token::Comment> {
			bool operator() (const token::Comment* a, const token::Comment*b) const { return strcmp(a->str(), b->str())<0; }
		};

		template <> struct less_than<token::Re> {
			bool operator() (const token::Re* a, const token::Re*b) const { return strcmp(a->pattern(), b->pattern())<0; }
		};

		template <> struct less_than<token::Nt> {
			bool operator() (const token::Nt* a, const token::Nt*b) const { return strcmp(a->tag(), b->tag())<0; }
		};

		template <> struct less_than<token::Bow> {
			bool operator() (const token::Bow* a, const token::Bow*b) const { return strcmp(a->tag(), b->tag())<0 || a->keep()<b->keep(); }
		};

		template <> struct less_than<token::AddToBag> {
			bool operator() (const token::AddToBag* a, const token::AddToBag*b) const { return strcmp(a->tag(), b->tag())<0 || strcmp(a->pattern(), b->pattern())<0 || a->keep()<b->keep(); }
		};

		template <> struct less_than<token::Str> {
			bool operator() (const token::Str* a, const token::Str*b) const { return strcmp(a->start(), b->start())<0 || strcmp(a->end(), b->end())<0; }
		};

		template <> struct less_than<token::Eof > {
			bool operator() (const token::Eof* a, const token::Eof*b) const { return false; }
		};

		template <> struct less_than<token::Epsilon > {
			bool operator() (const token::Epsilon* a, const token::Epsilon*b) const { return false; }
		};

	}
}

#endif

