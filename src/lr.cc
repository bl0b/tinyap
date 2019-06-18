#include <unordered_map>
#include "lr.h"
#include "static_init.h"

extern "C" {
	const char* ast_serialize_to_string(ast_node_t, int);
}


extern "C" {
	counter tinyap_allocs = 0;
	counter tinyap_reallocs = 0;
	counter tinyap_frees = 0;
	counter tinyap_ram_size = 0;
	counter gss_allocs = 0;
	counter gss_reallocs = 0;
	counter gss_frees = 0;
	counter gss_ram_size = 0;
	counter states_count = 0;
	counter gss_shifts = 0;
	counter gss_reduces = 0;
}


namespace grammar {
namespace item {

	registry_t::~registry_t() {
		/*std::cerr << "registry_t::~registry_t" << std::endl;*/
		std::vector<base*>::iterator i,j;
		for(i=begin(), j=end();i!=j;++i) {
			delete *i;
		}
	}

	static std::unordered_map<const ast_node_t, base*, lr::hash_an, lr::ptr_eq<_ast_node_t> >& registry = _static_init.grammar_registry;

	std::unordered_map<const char*, token::Nt*>& token::Nt::registry = _static_init.nt_registry;

	void clean_registry_at_exit() {
		typedef std::unordered_map<const ast_node_t, base*, lr::hash_an, lr::ptr_eq<_ast_node_t> >::iterator mapiter_t;
		typedef std::unordered_map<const char*, token::Nt*>::iterator nt_reg_iter_t;
		typedef std::unordered_map<const char*, trie_t>::iterator trie_reg_iter_t;
		mapiter_t i, j=registry.end();
		for(i=registry.begin();i!=j;++i) {
			delete_node((ast_node_t)i->first);
			delete i->second;
		}
		registry.clear();

		nt_reg_iter_t nti, ntj;
		nti = _static_init.nt_registry.begin();
		ntj = _static_init.nt_registry.end();
		for(;nti!=ntj;++nti) {
			delete nti->second;
		}
		_static_init.nt_registry.clear();

		trie_reg_iter_t ti, tj;
		ti = _static_init.trie_registry.begin();
		tj = _static_init.trie_registry.end();
		for(;ti!=tj;++ti) {
			trie_free(ti->second);
		}
		_static_init.trie_registry.clear();

		/*std::cerr << "clean_registry_at_exit" << std::endl;*/
	}

	base* base::from_ast(const ast_node_t n, Grammar* g) {
		base* ret=NULL;
		base* cached = registry[n];
		if(cached) {
			/*std::cout << "reusing cached item ";*/
			/*visitors::debugger d;*/
			/*cached->accept(&d);*/
			/*std::cout << std::endl;*/
			return cached;
		} else {
			/*std::cout << "no cached item for node " << n << std::endl;*/
		}
		const char* tag = regstr(Value(Car(n)));
        /*std::cout << "tag=" << tag << '@' << ((void*)tag) << "  OperatorRule=@" << ((void*)STR_OperatorRule) << std::endl;*/
		if(tag==STR_RE) {
			ret = cached = new token::Re(Value(Car(Cdr(n))));
		} else if(tag==STR_T) {
			ret = cached = new token::T(Value(Car(Cdr(n))));
		} else if(tag==STR_EOF) {
			ret = cached = new token::Eof();
		} else if(tag==STR_Epsilon) {
			ret = cached = new token::Epsilon();
		} else if(tag==STR_Comment) {
			ret = cached = new token::Comment(Value(Car(Cdr(n))));
		} else if(tag==STR_NT) {
			ret = token::Nt::instance(Value(Car(Cdr(n))));
			cached = NULL;
		} else if(tag==STR_Alt) {
			combination::Alt* edit = new combination::Alt();
			ast_node_t m=Cdr(n);
			while(m) {
				base* i = from_ast(Car(m), g);
				if(i) { edit->insert(i); }
				m=Cdr(m);
			}
			if(edit->size()>1) {
				ret = cached = edit;
			} else if(edit->size()==1) {
				ret = *edit->begin();
				cached = NULL;
				edit->clear();
				delete edit;
			} else {
				delete edit;
				ret = cached = NULL;
			}
		} else if(tag==STR_RawSeq) {
			combination::RawSeq* edit = new combination::RawSeq();
			ast_node_t m=Cdr(n);
			while(m) {
				base* i = from_ast(Car(m), g);
				if(i) { edit->push_back(i); }
				m=Cdr(m);
			}
			if(edit->size()>1) {
				ret = cached = edit;
			} else if(edit->size()==1) {
				ret = *edit->begin();
				cached = NULL;
				edit->clear();
				delete edit;
			} else {
				delete edit;
				ret = cached = NULL;
			}
		} else if(tag==STR_Seq) {
			combination::Seq* edit = new combination::Seq();
			ast_node_t m=Cdr(n);
			while(m) {
				base* i = from_ast(Car(m), g);
				if(i) { edit->push_back(i); }
				m=Cdr(m);
			}
			if(edit->size()>1) {
				ret = cached = edit;
			} else if(edit->size()==1) {
				ret = *edit->begin();
				cached = NULL;
				edit->clear();
				delete edit;
			} else {
				delete edit;
				ret = cached = NULL;
			}
		} else if(tag==STR_Rep0N) {
			ret = cached = new combination::Rep0N(g, from_ast(Car(Cdr(n)), g));
		} else if(tag==STR_Rep01) {
			ret = cached = new combination::Rep01(g, from_ast(Car(Cdr(n)), g));
		} else if(tag==STR_Rep1N) {
			ret = cached = new combination::Rep1N(g, from_ast(Car(Cdr(n)), g));
		} else if(tag==STR_RPL) {
			ret = cached = NULL;
		} else if(tag==STR_STR) {
			ast_node_t x = Cdr(n);
			ret = cached = new token::Str(x&&Car(x)?Value(Car(x)):"", x&&Cdr(x)?Value(Car(Cdr(x))):"");
		} else if(tag==STR_BOW) {
			ast_node_t x = Cdr(n);
			ret = cached = new token::Bow(Value(Car(x)), !!Cdr(x));
		} else if(tag==STR_AddToBag) {
			ast_node_t x = Cdr(n);
			// pattern is inside (RE pattern) in Car(x)
			// bag name is in Car(Cdr(x))
			ret = cached = new token::AddToBag(Value(Car(Cdr(Car(x)))), Value(Car(Cdr(x))), !!Cdr(Cdr(x)));
		} else if(tag==STR_Prefix) {
			ast_node_t x = Cdr(n);
			ast_node_t nt = Car(Cdr(x));
			ret = cached = new combination::Prefix(g, from_ast(Car(x), g), Value(Car(Cdr(nt))));
			std::cerr << "x=" << x << " nt=" << nt << std::endl;
		} else if(tag==STR_Postfix) {
			ast_node_t x = Cdr(n);
			ast_node_t nt = Car(Cdr(x));
			ret = cached = new combination::Postfix(g, from_ast(Car(x), g), Value(Car(Cdr(nt))));
		} else if(tag==STR_TransientRule) {
			ast_node_t x = Cdr(n);
			ret = gc(new rule::Transient(Value(Car(x)), from_ast(Car(Cdr(x)), g), g));
			cached = NULL;
		} else if(tag==STR_OperatorRule) {
			ast_node_t x = Cdr(n);
			ret = gc(new rule::Operator(Value(Car(x)), from_ast(Car(Cdr(x)), g), g));
			cached = NULL;
		} else if(tag==STR_Space) {
			/*cached = new token::Nt(STR_Space);*/
			ret = cached = NULL;
		} else if(tag==STR_NewLine) {
			/*cached = new token::Nt(STR_NewLine);*/
			ret = cached = NULL;
		} else if(tag==STR_Indent) {
			/*cached = new token::Nt(STR_Indent);*/
			ret = cached = NULL;
		} else if(tag==STR_Dedent) {
			/*cached = new token::Nt(STR_Dedent);*/
			ret = cached = NULL;
		}
		/*std::cout << "adding item ";*/
		/*visitors::debugger d;*/
		/*if (ret) { ret->accept(&d); } else { std::cout << "NULL"; }*/
		/*std::cout << " in cache for node " << n << std::endl;*/


		if(cached) {
			registry[n] = cached;
			/*std::cerr << "registry[" << n << "] = " << cached << std::endl;*/
			/*n->raw.ref++;*/
            ref(n);
		} else {
			registry.erase(n);
		}

		
		/*std::cout << "now registry[" << ast_serialize_to_string(n) << "] = ";*/
		/*registry[n]->accept(&d);*/
		/*std::cout << std::endl;*/
		return ret;
	}


	iterator iterator::create(const base*item) {
		visitors::iterator_factory f;
		return iterator(f(item));
	}

namespace combination {

	void base::contents_commit_helper(Grammar*g, item::base* raw_cts) const {
		visitors::item_rewriter(g).process(raw_cts);
	}

	std::pair<ast_node_t, unsigned int> rec_recog(const char* source, unsigned int offset, unsigned int size, RawSeq::const_iterator i, RawSeq::const_iterator j) {
		if(i == j) {
			return std::pair<ast_node_t, unsigned int>(0, offset);
		}
		/*typedef std::list<Ast> recog_stack_t;*/
		typedef std::list<ast_node_t> recog_stack_t;
		recog_stack_t stack;
		while(offset<size && i!=j) {
			std::pair<ast_node_t, unsigned int> cur = (*i)->recognize(source, offset, size);
			if(!cur.first) {
				return std::pair<ast_node_t, unsigned int>(0, offset);
			}
			offset = cur.second;
			if(cur.first != PRODUCTION_OK_BUT_EMPTY) {
				stack.push_back(Car(cur.first));
				/*Car(cur.first)->raw.ref++;*/
                ref(Car(cur.first));
                /*std::cout << cur.first << std::endl;*/
				delete_node(cur.first);
			}
			++i;
		}
		ast_node_t ret = NULL;
		recog_stack_t::reverse_iterator si, sj = stack.rend();
		for(si=stack.rbegin();si!=sj;++si) {
			/*ret = rule::internal::append()(*si, ret);*/
			/*ret = newPair(Car(*si), ret);*/
			ret = newPair(*si, ret);
			delete_node(*si);
		}
		stack.clear();
		return std::pair<ast_node_t, unsigned int>(ret, offset);
#if 0
		if(!cur.first) {
			cur.second = 0;
			return cur;
		}
		std::pair<ast_node_t, unsigned int> tail = rec_recog(source, cur.second, size, ++i, j);
		cur.second = tail.second;
		if(tail.first) {
			ast_node_t x;
			if(cur.first&&cur.first!=PRODUCTION_OK_BUT_EMPTY&&Cdr(cur.first)) {
				x = rule::internal::append()(cur.first, tail.first);
			} else {
				x = newPair(Car(cur.first), tail.first);
				/*delete_node(tail.first);*/
				/*delete_node(cur.first);*/
			}

			return std::pair<ast_node_t, unsigned int>(x, cur.second);
		} else if(tail.second) {
			if(cur.first) {
				return cur;
			}
			return tail;
		} else {
			delete_node(cur.first);
			cur.first = NULL;
			return cur;
		}
#endif
	}

	std::pair<ast_node_t, unsigned int> RawSeq::recognize(const char* source, unsigned int offset, unsigned int size) const {
#if 1
#  if 1
		return rec_recog(source, offset, size, begin(), end());
#  else
		std::pair<ast_node_t, unsigned int> skip = rec_recog(source, offset, size, begin(), end());
		delete_node(skip.first);
		return std::pair<ast_node_t, unsigned int>(newPair(newAtom("FREEME", 0), NULL), skip.second);
#  endif
#else
		visitors::debugger d;
		/*rule::internal::append append;*/
		RawSeq::const_iterator i, j;
		std::pair<ast_node_t, unsigned int> ret(NULL, offset);
		std::cout << "matching rawseq ? " << std::string(source+offset, source+offset+20) << std::endl;
		for(i=begin(), j=end();i!=j&&ret.second<=size;++i) {
			(*i)->accept(&d);
			std::pair<ast_node_t, unsigned int> tmp = (*i)->recognize(source, ret.second, size);
			if(tmp.first) {
				std::cout << " matched" << std::endl;
				ast_node_t oldret = ret.first;
				if(ret.first==PRODUCTION_OK_BUT_EMPTY) {
					ret.first = tmp.first;
				} else if(tmp.first!=PRODUCTION_OK_BUT_EMPTY) {
					ret.first = rule::internal::append()(ret.first, tmp.first);
				}
				///*ret.first = append(tmp.first, ret.first);*/
				/*delete_node(oldret);*/
				ret.second = tmp.second;
				/*delete_node(tmp.first);*/
				std::cout << ret.first << std::endl;
			} else {
				std::cout << " failed" << std::endl;
				delete_node(ret.first);
				return std::pair<ast_node_t, unsigned int>(NULL, offset);
			}
		}
		if(i!=j) {
			std::cout << "failed at end of text" << std::endl;
			delete_node(ret.first);
			return std::pair<ast_node_t, unsigned int>(NULL, offset);
		}
		std::cout << "OK ! new offset = " << ret.second << std::endl;
		return ret;
#endif
	}



	void Rep0N::contents(Grammar*g, item::base*x) {
		/*std::cout << "POUET Rep0N" << std::endl;*/
		_ = item::token::Nt::instance(rule::base::auto_tag<Rep0N>());
		raw_cts = x;
		item::base* X = visitors::item_rewriter(g).process(raw_cts);
		Alt* alt = gc(new Alt());
		Seq* seq = gc(new Seq());
		alt->insert(token::Epsilon::instance());
		seq->push_back(X);
		seq->push_back(_);
		alt->insert(seq);
		/*alt->contents(g, alt);*/
		/*alt->commit(g);*/
		cts = alt;
	}

	void Rep1N::contents(Grammar*g, item::base*x) {
		/*std::cout << "POUET Rep1N" << std::endl;*/
		_ = item::token::Nt::instance(rule::base::auto_tag<Rep1N>());
		Alt* alt = item::gc(new Alt());
		Seq* seq = item::gc(new Seq());
		raw_cts = x;
		item::base* X = visitors::item_rewriter(g).process(raw_cts);
		alt->insert(X);
		seq->push_back(X);
		seq->push_back(_);
		alt->insert(seq);
		/*alt->contents(g, alt);*/
		/*alt->commit(g);*/
		cts = alt;
	}

    void Prefix::contents(Grammar* g, item::base* x) {
        _ = item::token::Nt::instance(rule::base::auto_tag<Prefix>());
        cts = item::gc(new Seq())
                ->add(visitors::item_rewriter(g).process(x))
                ->add(item::token::Nt::instance(tag()));
    }

    void Postfix::contents(Grammar* g, item::base* x) {
        _ = item::token::Nt::instance(rule::base::auto_tag<Postfix>());
        cts = item::gc(new Seq())
                ->add(visitors::item_rewriter(g).process(x))
                ->add(item::token::Nt::instance(tag()));
    }

} /* namespace combination */
} /* namespace item */

namespace rule {
	void base::init(const char* name, item::base* _, Grammar*g)
	{
		tag_ = regstr(name);
		if(!_) { std::cout << "NULL rmember !" << std::endl; throw "COIN!"; return; }
		visitors::rmember_rewriter rw(g, this);
		visitors::debugger debug;
		/*std::cout << " adding rmember "; _->accept(&debug); std::cout << std::endl;*/
		item::base* rmb = rw(_);
		if(rmb) {
			/*std::cout << "  => as "; _->accept(&debug); std::cout << std::endl;*/
			insert(rmb);
		} else {
			/*std::cout << "  => skipped rmb in " << name << std::endl;*/
			/*insert(new item::token::Epsilon());*/
		}
	}
}


Grammar::Grammar(ast_node_t rules) {
    /*std::cout << "DEBUG GRAMMAR " << rules << std::endl;*/
    /*std::cout << "pouet" << std::endl;*/
    while(rules) {
        ast_node_t rule = Car(rules);
        /*std::cout << "         RULE " << rule << std::endl;*/
        if(Value(Car(rule))!=STR_Comment) {
            const char* tag = Value(Car(Cdr(rule)));
            add_rule(tag, dynamic_cast<rule::base*>(item::base::from_ast(rule, this)));
        }
        rules = Cdr(rules);
    }
	/* initialize whitespace-skipping subsystem */
	rule::base* WS = (*this)["_whitespace"];
	ws = NULL;
	if(WS) {
		/*std::cout << "have user-defined whitespaces" << std::endl;*/
		item::iterator WSi = item::iterator::create(WS);
		item::iterator expri = item::iterator::create(*WSi);
		const item::token::Re* check_re = dynamic_cast<const item::token::Re*>(*expri);
		if(check_re) {
			/*std::cout << "     user-defined whitespaces /" << check_re->pattern() << '/' << std::endl;*/
			ws = new ws_re(check_re->pattern());
		} else {
			const item::token::T* check_t = dynamic_cast<const item::token::T*>(*expri);
			if(check_t) {
				/*std::cout << "     user-defined whitespaces \"" << check_re->pattern() << '"' << std::endl;*/
				ws = new ws_str(check_t->str());
			}
		}
	}
	/* default to basic whitespace definition */
	if(!ws) {
		ws = new ws_str(" \t\n\r");
	}
}

Grammar::~Grammar() {
	delete ws;
	/*iterator i=begin(), j=end();*/
	/*for(;i!=j;++i) {*/
		/*delete i->second;*/
	/*}*/
}


}/* namespace grammar */


