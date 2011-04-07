#include "lr.h"

extern "C" {
	const char* ast_serialize_to_string(ast_node_t);
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
		std::vector<base*>::iterator i,j;
		for(i=begin(), j=end();i!=j;++i) {
			delete *i;
		}
	}

	ext::hash_map<const ast_node_t, base*, lr::hash_an, lr::ptr_eq<_ast_node_t> > registry;

	struct clean_registry_at_exit {
		~clean_registry_at_exit() {
			ext::hash_map<const ast_node_t, base*, lr::hash_an, lr::ptr_eq<_ast_node_t> >::iterator
				i, j=registry.end();
			for(i=registry.begin();i!=j;++i) {
				delete (*i).second;
			}
		}
	} _clean_registry_at_exit;

	base* base::from_ast(const ast_node_t n, Grammar* g) {
		base* ret=NULL;
		base* cached = registry[n];
		if(cached) {
			/*std::cout << "reusing cached item ";*/
			/*visitors::debugger d;*/
			/*cached->accept(&d);*/
			/*std::cout << std::endl;*/
			return cached;
		/*} else {*/
			/*std::cout << "no cached item for node " << ast_serialize_to_string(n) << std::endl;*/
		}
		const char* tag = Value(Car(n));
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
			ret = cached = NULL;
		} else if(tag==STR_Prefix) {
			ast_node_t x = Cdr(n);
			ast_node_t nt = Car(Cdr(x));
			ret = cached = new combination::Prefix(g, from_ast(Car(x), g), Value(Car(Cdr(nt))));
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
		/*cached->accept(&d);*/
		/*std::cout << " in cache for node " << ast_serialize_to_string(n) << std::endl;*/


		registry[n] = cached;

		
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

	std::pair<ast_node_t, unsigned int> RawSeq::recognize(const char* source, unsigned int offset, unsigned int size) const
	{
		/*visitors::debugger d;*/
		/*rule::internal::append append;*/
		RawSeq::const_iterator i, j;
		std::pair<ast_node_t, unsigned int> ret(NULL, offset);
		/*std::cout << "matching rawseq ? " << std::string(source+offset, source+offset+20) << std::endl;*/
		for(i=begin(), j=end();ret.second<size&&i!=j&&ret.second<=size;++i) {
			/*(*i)->accept(&d);*/
			std::pair<ast_node_t, unsigned int> tmp = (*i)->recognize(source, ret.second, size);
			if(tmp.first) {
				/*std::cout << " matched" << std::endl;*/
				ret.first = rule::internal::append()(ret.first, tmp.first);
				/*ret.first = append(tmp.first, ret.first);*/
				ret.second = tmp.second;
				/*std::cout << ret.first << std::endl;*/
			} else {
				/*std::cout << " failed" << std::endl;*/
				return std::pair<ast_node_t, unsigned int>(NULL, offset);
			}
		}
		if(i!=j) {
			/*std::cout << "failed at end of text" << std::endl;*/
			return std::pair<ast_node_t, unsigned int>(NULL, offset);
		}
		/*std::cout << "OK ! new offset = " << ret.second << std::endl;*/
		return ret;
	}



	void Rep0N::contents(Grammar*g, item::base*x) {
		/*std::cout << "POUET Rep0N" << std::endl;*/
		_ = item::token::Nt::instance(rule::base::auto_tag<Rep0N>());
		raw_cts = x;
		item::base* X = visitors::item_rewriter(g).process(raw_cts);
		Alt* alt = gc(new Alt());
		Seq* seq = gc(new Seq());
		alt->insert(token::Epsilon::instance());
		seq->push_back(_);
		seq->push_back(X);
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
		seq->push_back(_);
		seq->push_back(X);
		alt->insert(seq);
		/*alt->contents(g, alt);*/
		/*alt->commit(g);*/
		cts = alt;
	}

} /* namespace combination */
} /* namespace item */

namespace rule {
	void base::init(const char* name, item::base* _, Grammar*g)
	{
		tag_ = name;
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
	if(rules) {
		/*std::cout << "DEBUG GRAMMAR " << ast_serialize_to_string(rules) << std::endl;*/
		/*std::cout << "pouet" << std::endl;*/
		while(rules) {
			ast_node_t rule = Car(rules);
			if(regstr(Value(Car(rule)))!=STR_Comment) {
				const char* tag = Value(Car(Cdr(rule)));
				add_rule(tag, dynamic_cast<rule::base*>(item::base::from_ast(rule, this)));
			}
			rules = Cdr(rules);
		}
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


