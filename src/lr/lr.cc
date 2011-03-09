#include "lr.h"
extern "C" {
	const char* ast_serialize_to_string(ast_node_t);
}
namespace grammar {
namespace item {
	struct hash_an {
		size_t operator()(const ast_node_t x) const {
			return (size_t)x;
		}
	};

	static ext::hash_map<const ast_node_t, base*, hash_an, lr::ptr_eq<_ast_node_t> > registry;

	struct clean_registry_at_exit {
		~clean_registry_at_exit() {
			ext::hash_map<const ast_node_t, base*, hash_an, lr::ptr_eq<_ast_node_t> >::iterator
				i, j=registry.end();
			for(i=registry.begin();i!=j;++i) {
				delete (*i).second;
			}
		}
	} _clean_registry_at_exit;

	base* base::from_ast(const ast_node_t n, Grammar* g) {
		base* cached = registry[n];
		if(cached) {
			/*std::cout << "reusing cached item ";*/
			visitors::debugger d;
			cached->accept(&d);
			std::cout << std::endl;
			return cached;
		} else {
			/*std::cout << "no cached item for node " << ast_serialize_to_string(n) << std::endl;*/
			registry[n] = cached;
		}
		visitors::item_rewriter rw(g);
		const char* tag = Value(Car(n));
		if(tag==STR_RE) {
			cached = new token::Re(Value(Car(Cdr(n))));
		} else if(tag==STR_T) {
			cached = new token::T(Value(Car(Cdr(n))));
		} else if(tag==STR_EOF) {
			cached = new token::Eof();
		} else if(tag==STR_Epsilon) {
			cached = new token::Epsilon();
		} else if(tag==STR_Comment) {
			cached = new token::Comment(Value(Car(Cdr(n))));
		} else if(tag==STR_NT) {
			cached = new token::Nt(Value(Car(Cdr(n))));
		} else if(tag==STR_Alt) {
			combination::Alt* edit = new combination::Alt();
			ast_node_t m=Cdr(n);
			while(m) {
				base* i = rw(Car(m));
				if(i) { edit->insert(i); }
				m=Cdr(m);
			}
			cached = edit;
		} else if(tag==STR_RawSeq) {
			combination::RawSeq* edit = new combination::RawSeq();
			ast_node_t m=Cdr(n);
			while(m) {
				base* i = rw(Car(m));
				if(i) { edit->push_back(i); }
				m=Cdr(m);
			}
			cached = edit;
		} else if(tag==STR_Seq) {
			combination::Seq* edit = new combination::Seq();
			ast_node_t m=Cdr(n);
			while(m) {
				base* i = rw(Car(m));
				if(i) { edit->push_back(i); }
				m=Cdr(m);
			}
			cached = edit;
		} else if(tag==STR_Rep0N) {
			cached = new combination::Rep0N(rw(Car(Cdr(n))));
		} else if(tag==STR_Rep01) {
			cached = new combination::Rep01(rw(Car(Cdr(n))));
		} else if(tag==STR_Rep1N) {
			cached = new combination::Rep1N(rw(Car(Cdr(n))));
		} else if(tag==STR_RPL) {
			cached = NULL;
		} else if(tag==STR_STR) {
			ast_node_t x = Cdr(n);
			cached = new token::Str(Value(Car(x)), Value(Car(Cdr(x))));
		} else if(tag==STR_BOW) {
			ast_node_t x = Cdr(n);
			cached = new token::Bow(Value(Car(x)), !!Cdr(x));
		} else if(tag==STR_AddToBag) {
			cached = NULL;
		} else if(tag==STR_Prefix) {
			ast_node_t x = Cdr(n);
			ast_node_t nt = Car(Cdr(x));
			cached = new combination::Prefix(rw(Car(x)), Value(Car(Cdr(nt))));
		} else if(tag==STR_Postfix) {
			ast_node_t x = Cdr(n);
			ast_node_t nt = Car(Cdr(x));
			cached = new combination::Postfix(rw(Car(x)), Value(Car(Cdr(nt))));
		} else if(tag==STR_TransientRule) {
			ast_node_t x = Cdr(n);
			cached = new rule::Transient(Value(Car(x)), from_ast(Car(Cdr(x)), g), g);
		} else if(tag==STR_OperatorRule) {
			ast_node_t x = Cdr(n);
			cached = new rule::Operator(Value(Car(x)), from_ast(Car(Cdr(x)), g), g);
		} else if(tag==STR_Space) {
			cached = new token::Nt(STR_Space);
		} else if(tag==STR_NewLine) {
			cached = new token::Nt(STR_NewLine);
		} else if(tag==STR_Indent) {
			cached = new token::Nt(STR_Indent);
		} else if(tag==STR_Dedent) {
			cached = new token::Nt(STR_Dedent);
		}
		/*std::cout << "adding item ";*/
		/*visitors::debugger d;*/
		/*cached->accept(&d);*/
		/*std::cout << " in cache for node " << ast_serialize_to_string(n) << std::endl;*/
		registry[n] = cached;
		/*std::cout << "now registry[" << ast_serialize_to_string(n) << "] = ";*/
		/*registry[n]->accept(&d);*/
		/*std::cout << std::endl;*/
		return cached;
	}


	iterator iterator::create(const base*item) {
		visitors::iterator_factory f;
		return iterator(f(item));
	}
}

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
			/*push_back(rmb);*/
			insert(rmb);
		/*} else {*/
			/*std::cout << "  => skipped rmb in " << name << std::endl;*/
			/*push_back(new item::token::Epsilon());*/
			/*insert(new item::token::Epsilon());*/
		}
	}
}


Grammar::Grammar(ast_node_t rules) {
	while(rules) {
		ast_node_t rule = Car(rules);
		const char* tag = Value(Car(Cdr(rule)));
		add_rule(tag, dynamic_cast<rule::base*>(item::base::from_ast(rule, this)));
		rules = Cdr(rules);
	}
}

Grammar::~Grammar() {
	/*iterator i=begin(), j=end();*/
	/*for(;i!=j;++i) {*/
		/*delete i->second;*/
	/*}*/
}


}/* namespace grammar */


namespace io {
}

