#include "lr.h"

#include <vector>
#include <string>
#include <iostream>
#include <typeinfo>

extern "C" {
#include "ast.h"
#include "bootstrap.h"
#include "tinyap.h"
#include "walkableast.h"
}

void test() {
	std::stringstream ss("(toto pouet)");
}



struct A {
	/*virtual ~A() {}*/
	virtual void foo() { std::cout << "A" << std::endl; }
};
struct B : public A {
	virtual void foo() { std::cout << "B" << std::endl; }
};
struct C : public A {
	virtual void foo() { std::cout << "C" << std::endl; }
};


void bar(A*x) { std::cout << "bar A" << std::endl; }
void bar(B*x) { std::cout << "bar B" << std::endl; }
void bar(C*x) { std::cout << "bar C" << std::endl; }


extern "C" {
/*#include "bootstrap.c"*/
}

void test_lr(lr::automaton& a, const char* text) {
	char* str = (char*)tinyap_serialize_to_string(a.recognize(text, strlen(text)));
	std::cout << '"' << text << "\" => " << str << std::endl;
	free(str);
}

void test_nl() {
	grammar::Grammar g(Cdr(Car(tinyap_get_ruleset("debug_nl"))));
	lr::automaton nl(&g);
	nl.dump_states();
	const char* pouet = "I saw a man in the park with a telescope";
	ast_node_t ast = nl.recognize(pouet, strlen(pouet));
	char* str = (char*)tinyap_serialize_to_string(ast);
	std::cout << '"' << pouet << "\" => " << str << std::endl;
	while(ast) {
		tinyap_walk(make_wast(Car(Car(ast))), "prettyprint", NULL);
		ast = Cdr(ast);
	}
	grammar::visitors::debugger debug;
	g.accept(&debug);
}

int main(int argc, char**argv) {
	tinyap_init();
	/*grammar::Grammar g(Cdr(Car(tinyap_get_ruleset("slr"))));*/
	grammar::Grammar short_gram(Cdr(Car(tinyap_get_ruleset(GRAMMAR_SHORT))));
	/*grammar::Grammar g(Cdr(Car(tinyap_get_ruleset(GRAMMAR_SHORT))));*/

	/*grammar::item::token::Re* re = new grammar::item::token::Re("");*/

	grammar::visitors::debugger debug;

	/*grammar::Grammar::iterator i=g.begin(), j=g.end();*/
	/*for(;i!=j;++i) {*/
		/*std::cout << "got " << (void*)(*i).first << " <=> "; (*i).second->accept(&debug); std::cout << std::endl;*/
	/*}*/

	/*debug.visit(re);*/
	/*delete re;*/
	/*const char* toto = regstr("_start");*/
	/*std::cout << g.size() << ' ' << (void*)toto << ' ' << g[toto] << std::endl;*/
	/*debug.visit(&g);*/

	std::cout << "v " << sizeof(std::vector<void*>) << std::endl;
	std::cout << "m " << sizeof(ext::hash_map<const char*, void*>) << std::endl;
	std::cout << "s " << sizeof(std::set<void*>) << std::endl;


	/*lr::automaton d2(&g);*/
	lr::automaton tinyaglrp(&short_gram);
	/*tinyaglrp.dump_states();*/
	/*d2.dump_states();*/
	/*test_lr(d2, "*id");*/
	/*test_lr(d2, "id=id");*/
	/*test_lr(d2, "*id=*id=id");*/

	/*grammar::Grammar g(Cdr(Car(tinyap_get_ruleset("debug_nl"))));*/
	/*lr::automaton nl(&g);*/
	/*nl.dump_states();*/
	test_nl();
#if 0
	/*grammar::rule::base* start = g["_start"];*/
	/*grammar::rule_iterator is(start);*/
	/*grammar::rule::base::iterator i = start->begin();*/
	/*grammar::rule_iterator s(start);*/
	const char* my_rule_tag = "Rep0N_1";
	grammar::item::iterator rule = grammar::item::iterator::create(g[my_rule_tag]);
	((grammar::item::base*)*rule)->accept(&debug);
	/*lr::item lrs(dynamic_cast<const grammar::rule::base*>(rule.context()), grammar::item::iterator::create(*rule));*/
	lr::item lrs(g[my_rule_tag], rule);
	/*lr::item lrs2(g[my_rule_tag], rule.next());*/

	std::cout << lrs << std::endl;
	/*std::cout << --lrs << std::endl;*/
	lr::item lrs2(g[my_rule_tag], ++rule);
	std::cout << lrs2 << std::endl;
	lr::item lrs3 = lrs2.next();
	std::cout << lrs3 << std::endl;

	lr::item_set I;
	lr::item_set C;
	{I.insert(lrs3);
	d2.closure(I, C);
	lr::item_set::iterator i, j=C.end();
	for(i=C.begin();i!=j;++i) {
		lr::item tmp = *i;
		std::cout << " :: " << tmp << std::endl;
	}}

	std::cout << std::endl;
	{I.clear();
	I.insert(lrs);
	d2.closure(I, C);
	lr::item_set::iterator i, j=C.end();
	for(i=C.begin();i!=j;++i) {
		lr::item tmp = *i;
		std::cout << " :: " << tmp << std::endl;
	}}

	std::cout << std::endl;
	{I.clear();
	I.insert(lr::item(g[regstr("_start")], grammar::item::iterator::create(g[regstr("_start")])));
	d2.closure(I, C);
	lr::item_set::iterator i, j=C.end();
	for(i=C.begin();i!=j;++i) {
		lr::item tmp = *i;
		std::cout << " :: " << tmp << std::endl;
	}
	std::cout << std::endl;
	grammar::visitors::producer_filter f;
	for(i=C.begin();i!=j;++i) {
		lr::item tmp = *i;
		if(f((grammar::item::base*)*tmp)) {
			std::cout << " :> " << tmp << std::endl;
			grammar::visitors::debugger d;
			((grammar::item::base*)*tmp)->accept(&d);
			std::cout << std::endl;
			lr::item_set J, D;
			J.insert(tmp.next());
			d2.closure(J, D);
			lr::item_set::iterator k, l=D.end();
			for(k=D.begin();k!=l;++k) {
				lr::item ktmp = *k;
				std::cout << " :: " << ktmp << std::endl;
			}
		}
	}
	}
#endif

	return 0;
}

