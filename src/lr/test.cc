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


#define TEST_OK() do { ++done; ++ok; } while(0)
#define TEST_FAIL() do { ++done; } while(0)

#define __(_x_) #_x_
#define TEST(_expr_) do { if(!(_expr_)) { TEST_FAIL(); std::cout << "Test " __(_expr_) " failed" << std::endl; } else { TEST_OK(); } } while(0)

#define TEST_EQ(_lv, _rv) do { if((_lv) != (_rv)) { TEST_FAIL(); std::cout << "Test " __(_lv == _rv) " failed, got " __(_lv) " == " << (_lv) << " instead" << std::endl; } else { TEST_OK(); } } while(0)

void test_grammar() {
	using namespace grammar;
	using namespace item;
	unsigned int done=0, ok=0;
	Grammar g(NULL);
	rule::Transient* R;
	token::Epsilon* eps = token::Epsilon::instance();
	combination::Alt* alt = new combination::Alt();
	token::Nt* nt = new token::Nt("test");
	token::Re* re = new token::Re("...");
	combination::Seq* seq = new combination::Seq();

	seq->push_back(re);
	seq->push_back(nt);

	TEST_EQ(seq->size(), 2);

	alt->insert(seq);
	alt->insert(eps);

	TEST_EQ(alt->size(), 2);
	R = new rule::Transient(nt->tag(), alt, &g);

	TEST_EQ(R->size(), 2);

	g.add_rule(R->tag(), R);

	TEST_EQ(g[nt->tag()]->size(), 2);

	combination::Rep01* r01 = new combination::Rep01(&g, nt);
	base* x = r01->commit(&g);
	token::Nt* rc = dynamic_cast<token::Nt*>(x);
	TEST_EQ(rc, x);
	TEST_EQ(g[rc->tag()]->size(), 2);

	Grammar g2(NULL);

	g.add_rule(R->tag(), R);

	x = r01->commit(&g);
	rc = dynamic_cast<token::Nt*>(x);
	TEST_EQ(rc, x);

	TEST_EQ(g[nt->tag()]->size(), 2);


	std::cout << "[TEST] passed: " << ok << '/' << done << std::endl;
}

void test_automaton() {
	typedef const char* test_case[3];
	test_case test_cases[] = {
		// start rule is X
		{ "(OperatorRule X (Epsilon))", "", "((X))" },
		{ "(OperatorRule X (Epsilon))", "a", NULL },
		{ "(OperatorRule X (T toto))", "toto", "((X))" },
		{ "(OperatorRule X (RE toto))", "toto", "((X toto))" },
		{ "(OperatorRule X (T toto))", "pouet", NULL },
		{ "(OperatorRule X (Rep01 (T pouet)))", "pouet", "((X))" },
		{ "(OperatorRule X (Rep01 (T pouet)))", "", "((X))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "pouet", "((X))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "pouet pouet pouet", "((X))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "", "((X))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "pouet", "((X))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "pouet pouet pouet", "((X))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "", NULL },
		{ "(OperatorRule X (NT Y)) (OperatorRule Y (Epsilon))", "", "((X (Y)))" },
		{ "(OperatorRule X (Prefix (RE toto) (NT Y))) (OperatorRule Y (RE pouet))", "totopouet", "((X (Y toto pouet)))" },
		{ "(OperatorRule X (Postfix (RE toto) (NT Y))) (OperatorRule Y (RE pouet))", "totopouet", "((X (Y pouet toto)))" },
		{ NULL, NULL, NULL }
	};

	test_case* tc = test_cases;
	unsigned int done=0, ok=0;
	while((*tc)[0]) {
		if(lr::automaton::test((*tc)[0], (*tc)[1], (*tc)[2])) {
			++ok;
		}
		++done;
		++tc;
	}
	std::cout << "[TEST] passed: " << ok << '/' << done << std::endl;
}







void test_lr(lr::automaton& a, const char* text) {
	char* str = (char*)tinyap_serialize_to_string(a.recognize(text, strlen(text)));
	std::cout << '"' << text << "\" => " << str << std::endl;
	free(str);
}

void test_nl() {
	grammar::Grammar g(Cdr(Car(tinyap_get_ruleset("debug_nl"))));
	lr::automaton nl(&g);
	/*nl.dump_states();*/
	const char* pouet = "I saw a man in the park with a telescope";
	ast_node_t ast = nl.recognize(pouet, strlen(pouet));
	char* str = (char*)tinyap_serialize_to_string(ast);
	std::cout << '"' << pouet << "\" => " << str << std::endl;
	while(ast) {
		wast_t wa = make_wast(Car(Car(ast)));
		tinyap_walk(wa, "prettyprint", NULL);
		wa_del(wa);
		ast = Cdr(ast);
	}
	free(str);
	grammar::visitors::debugger debug;
	g.accept(&debug);
}

int main(int argc, char**argv) {
	tinyap_init();

	test_grammar();

	test_automaton();


	/*grammar::Grammar dbg_g(Cdr(Car(tinyap_get_ruleset("test"))));*/
	/*grammar::Grammar g(Cdr(Car(tinyap_get_ruleset("slr"))));*/
	/*grammar::Grammar short_gram(Cdr(Car(tinyap_get_ruleset(GRAMMAR_SHORT))));*/

	/*grammar::item::token::Re* re = new grammar::item::token::Re("");*/

	/*grammar::visitors::debugger debug;*/

	/*grammar::Grammar::iterator i=g.begin(), j=g.end();*/
	/*for(;i!=j;++i) {*/
		/*std::cout << "got " << (void*)(*i).first << " <=> "; (*i).second->accept(&debug); std::cout << std::endl;*/
	/*}*/

	/*debug.visit(re);*/
	/*delete re;*/
	/*const char* toto = regstr("_start");*/
	/*std::cout << g.size() << ' ' << (void*)toto << ' ' << g[toto] << std::endl;*/
	/*debug.visit(&g);*/

	/*std::cout << "v " << sizeof(std::vector<void*>) << std::endl;*/
	/*std::cout << "m " << sizeof(ext::hash_map<const char*, void*>) << std::endl;*/
	/*std::cout << "s " << sizeof(std::set<void*>) << std::endl;*/

	/*lr::automaton dbg_a(&dbg_g);*/
	/*dbg_a.dump_states();*/
	/*std::cout << "parse epsilon ? " << dbg_a.recognize("", 0) << std::endl;*/
	/*std::cout << "parse abbbbb ? " << dbg_a.recognize("abbbbb", 6) << std::endl;*/
	/*std::cout << "parse atotob ? " << dbg_a.recognize("atotob", 6) << std::endl;*/

	/*test_nl();*/

	/*lr::automaton d2(&g);*/
	/*lr::automaton tinyaglrp(&short_gram);*/
	/*tinyaglrp.dump_states();*/
	/*d2.dump_states();*/
	/*test_lr(d2, "*id");*/
	/*test_lr(d2, "id=id");*/
	/*test_lr(d2, "*id=*id=id");*/
	/*test_lr(d2, "*id=toto");*/

	/*grammar::Grammar g(Cdr(Car(tinyap_get_ruleset("debug_nl"))));*/
	/*lr::automaton nl(&g);*/
	/*nl.dump_states();*/
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

