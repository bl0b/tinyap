#include "lr.h"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
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
#define TEST(_expr_) do { if(!(_expr_)) { TEST_FAIL(); std::cout << "[TEST] [grammar] Test " __(_expr_) " failed" << std::endl; } else { TEST_OK(); } } while(0)

#define TEST_EQ(_lv, _rv) do { if((_lv) != (_rv)) { TEST_FAIL(); std::cout << "[TEST] [grammar] Test " __(_lv == _rv) " failed, got " __(_lv) " == " << (_lv) << " instead" << std::endl; } else { TEST_OK(); } } while(0)

int test_grammar() {
	using namespace grammar;
	using namespace item;
	unsigned int done=0, ok=0;
	Grammar g(NULL);
	rule::Transient* R;
	token::Epsilon* eps = token::Epsilon::instance();
	combination::Alt* alt = gc(new combination::Alt());
	token::Nt* nt = gc(new token::Nt("test"));
	token::Re* re = gc(new token::Re("..."));
	combination::Seq* seq = gc(new combination::Seq());

	TEST_EQ(newAtom("toto", 0), newAtom("toto", 0));
	TEST_EQ(newPair(newAtom("toto", 0), NULL), newPair(newAtom("toto", 0), NULL));

	seq->push_back(re);
	seq->push_back(nt);

	TEST_EQ(seq->size(), 2);

	alt->insert(seq);
	alt->insert(eps);

	TEST_EQ(alt->size(), 2);
	R = gc(new rule::Transient(nt->tag(), alt, &g));

	TEST_EQ(R->size(), 2);

	g.add_rule(R->tag(), R);

	TEST_EQ(g[nt->tag()]->size(), 2);
	{
		grammar::rule::base::iterator i, j;
		i=g[nt->tag()]->begin();
		j=i;
		++j;
		grammar::item::combination::Seq* seq = dynamic_cast<grammar::item::combination::Seq*>(*i);
		grammar::item::token::Epsilon* eps = dynamic_cast<grammar::item::token::Epsilon*>(*j);
		if(!seq) {
			seq = dynamic_cast<grammar::item::combination::Seq*>(*j);
			eps = dynamic_cast<grammar::item::token::Epsilon*>(*i);
		}
		TEST(seq||!"Couldn't find any Seq in rule");
		TEST(eps||!"Couldn't find any Epsilon in rule");
	}

	


	combination::Rep01* r01 = gc(new combination::Rep01(&g, nt));
	base* x = r01->commit(&g);
	token::Nt* rc = dynamic_cast<token::Nt*>(x);
	TEST_EQ(rc, x);
	TEST_EQ(g[rc->tag()]->size(), 2);
	TEST_EQ(g[rc->tag()]->size(), 2);
	{
		grammar::rule::base::iterator i, j;
		i=g[rc->tag()]->begin();
		j=i;
		++j;
		grammar::item::token::Nt* nt = dynamic_cast<grammar::item::token::Nt*>(*i);
		grammar::item::token::Epsilon* eps = dynamic_cast<grammar::item::token::Epsilon*>(*j);
		if(!nt) {
			nt = dynamic_cast<grammar::item::token::Nt*>(*j);
			eps = dynamic_cast<grammar::item::token::Epsilon*>(*i);
		}
		TEST(nt||!"Couldn't find any Nt in rule");
		TEST(eps||!"Couldn't find any Epsilon in rule");
	}



	{
		combination::Rep0N* r0N = gc(new combination::Rep0N(&g, nt));
		base* x = r0N->commit(&g);
		token::Nt* rc = dynamic_cast<token::Nt*>(x);
		TEST_EQ(rc, x);
		TEST_EQ(g[rc->tag()]->size(), 2);
		TEST_EQ(g[rc->tag()]->size(), 2);
		{
			grammar::rule::base::iterator i, j;
			i=g[rc->tag()]->begin();
			j=i;
			++j;
			grammar::item::combination::Seq* seq = dynamic_cast<grammar::item::combination::Seq*>(*i);
			grammar::item::token::Epsilon* eps = dynamic_cast<grammar::item::token::Epsilon*>(*j);
			if(!seq) {
				seq = dynamic_cast<grammar::item::combination::Seq*>(*j);
				eps = dynamic_cast<grammar::item::token::Epsilon*>(*i);
			}
			TEST(seq||!"Couldn't find any Seq in rule");
			TEST(eps||!"Couldn't find any Epsilon in rule");
		}
	}


	Grammar g2(NULL);

	g.add_rule(R->tag(), R);

	x = r01->commit(&g);
	rc = dynamic_cast<token::Nt*>(x);
	TEST_EQ(rc, x);

	TEST_EQ(g[nt->tag()]->size(), 2);


	std::cout << "[TEST] [grammar] passed: " << ok << '/' << done << std::endl;
	return ok-done;
}

int test_automaton() {
	typedef const char* test_case[3];
	test_case test_cases[] = {
		// start rule is X
/*1*/	{ "(OperatorRule X (EOF))", "", "((X))" },
		{ "(OperatorRule X (EOF))", "   ", "((X))" },
		{ "(OperatorRule X (EOF))", "a", NULL },
		{ "(OperatorRule X (Epsilon))", "", "((X))" },
		{ "(OperatorRule X (Epsilon))", "a", NULL },
		{ "(OperatorRule X (Epsilon))", "   ", "((X))" },
		{ "(OperatorRule X (T toto))", "toto", "((X))" },
		{ "(OperatorRule X (RE toto))", "toto", "((X toto))" },
		{ "(OperatorRule X (T toto))", "pouet", NULL },

/*10*/	{ "(OperatorRule X (Rep01 (T pouet)))", "pouet", "((X))" },
		{ "(OperatorRule X (Rep01 (T pouet)))", "", "((X))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "pouet", "((X))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "pouet pouet pouet", "((X))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "", "((X))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "pouet", "((X))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "pouet pouet pouet", "((X))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "", NULL },

		{ "(OperatorRule X (Seq (T foo) (Rep01 (T pouet))))", "foo pouet", "((X))" },
		{ "(OperatorRule X (Seq (T foo) (Rep01 (T pouet))))", "foo ", "((X))" },
/*20*/	{ "(OperatorRule X (Seq (T foo) (Rep0N (T pouet))))", "foo pouet", "((X))" },
		{ "(OperatorRule X (Seq (T foo) (Rep0N (T pouet))))", "foo pouet pouet pouet", "((X))" },
		{ "(OperatorRule X (Seq (T foo) (Rep0N (T pouet))))", "foo ", "((X))" },
		{ "(OperatorRule X (Seq (T foo) (Rep1N (T pouet))))", "foo pouet", "((X))" },
		{ "(OperatorRule X (Seq (T foo) (Rep1N (T pouet))))", "foo pouet pouet pouet", "((X))" },
		{ "(OperatorRule X (Seq (T foo) (Rep1N (T pouet))))", "foo", NULL },

		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "pouet", "((X))" },
		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "", "((X))" },
		{ "(OperatorRule X (Alt (T foo) (Rep0N (T pouet))))", "pouet", "((X))" },
		{ "(OperatorRule X (Alt (T foo) (Rep0N (T pouet))))", "pouet pouet pouet", "((X))" },
/*30*/	{ "(OperatorRule X (Alt (T foo) (Rep0N (T pouet))))", "", "((X))" },
		{ "(OperatorRule X (Alt (T foo) (Rep1N (T pouet))))", "pouet", "((X))" },
		{ "(OperatorRule X (Alt (T foo) (Rep1N (T pouet))))", "pouet pouet pouet", "((X))" },
		{ "(OperatorRule X (Alt (T foo) (Rep1N (T pouet))))", "", NULL },

		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "foo", "((X))" },
		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "foo foo", NULL },

		{ "(OperatorRule X (NT Y)) (OperatorRule Y (Epsilon))", "", "((X (Y)))" },
		{ "(OperatorRule X (Prefix (RE toto) (NT Y))) (OperatorRule Y (RE pouet))", "totopouet", "((X (Y toto pouet)))" },
		{ "(OperatorRule X (Prefix (NT Z) (NT Y))) (TransientRule Z (Seq (RE toto) (RE pou))) (OperatorRule Y (RE et))", "totopouet", "((X (Y toto pou et)))" },
/*40*/	{ "(OperatorRule X (Postfix (RE toto) (NT Y))) (OperatorRule Y (RE pouet))", "totopouet", "((X (Y pouet toto)))" },
		{ "(OperatorRule X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~)))", "~\",\"~", "((X \" \"))" },
		{ "(OperatorRule X (Rep1N (NT pouet))) (OperatorRule pouet (RE pouet))", "pouet pouet pouet", "((X (pouet pouet) (pouet pouet) (pouet pouet)))" },
		{ "(OperatorRule X (Rep1N (NT coin))) (OperatorRule coin (RE pouet))", "pouet pouet pouet", "((X (coin pouet) (coin pouet) (coin pouet)))" },
		{ "(OperatorRule X (Rep1N (NT coin))) (OperatorRule coin (RawSeq (RE pou) (RE ..)))", "pouet pouat pouit", "((X (coin pou et) (coin pou at) (coin pou it)))" },
		{ "(OperatorRule X (RawSeq (T to) (RE ...) (T ouet)))", "totopouet", "((X top))" },
		{
			"(OperatorRule X (RawSeq (T #) (RE [^\\\\r\\n]*)))"
			, "# toto pouet", "((X \\ toto\\ pouet))"
		},
		{
			"(OperatorRule X (NT Comment))"
			"(OperatorRule Comment (Alt (T #\\n) (T #\\r\\n) (RawSeq (T #) (RE [^\\\\r\\\\n]+))))"
			, "# toto pouet\n", "((X (Comment \\ toto\\ pouet)))"
		},
		{
			"(OperatorRule X (NT Comment))"
			"(OperatorRule Comment (Alt (T #\\n) (T #\\r\\n) (RawSeq (T #) (RE [^\\\\r\\\\n]+))))"
			, "\r\n# toto pouet\n\r\n", "((X (Comment \\ toto\\ pouet)))"
		},
		{
			"(OperatorRule X (Seq (NT Comment) (NT Comment) (NT Comment)))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			, "# toto pouet\n#\n#toto", "((X (Comment \\ toto\\ pouet) (Comment ) (Comment toto)))"
		},
/*50*/	{
			"(OperatorRule X (Rep1N (NT Comment)))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			, "# toto pouet\n#\n#toto", "((X (Comment \\ toto\\ pouet) (Comment ) (Comment toto)))"
		},
		{
			"(OperatorRule X (Rep1N (Alt (RE toto) (RE pouet))))"
			, "toto toto pouet\t toto", "((X toto toto pouet toto))"
		},
		{
			"(OperatorRule X (Rep1N (Alt (NT Comment))))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			, "# toto pouet\n#\n#toto", "((X (Comment \\ toto\\ pouet) (Comment ) (Comment toto)))"
		},
		{
			"(OperatorRule X (NT Grammar))"
			"(OperatorRule Grammar (Rep1N (Alt (NT Comment))))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			, "# toto pouet\n#\n#toto", "((X (Grammar (Comment \\ toto\\ pouet) (Comment ) (Comment toto))))"
		},
		{
			"(OperatorRule X (NT Gramar))"
			"(OperatorRule Gramar (Rep1N (Alt (NT Coment))))"
			"(OperatorRule Coment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			, "# toto pouet\n#\n#toto", "((X (Gramar (Coment \\ toto\\ pouet) (Coment ) (Coment toto))))"
		},
/*55*/	{
"(OperatorRule T (STR \" \"))"
"(OperatorRule RE (STR / /))"
"(OperatorRule STR (RawSeq (T ~) (RE [^~,]?)  (T ,)"
"  (RE [^~,]?) (T ~)))"
"(OperatorRule BOW (RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (RE !?) (T ~)))"
"(OperatorRule AddToBag (Seq (NT RE) (T :) (NT symbol) (RE !?)))"
			"(OperatorRule X (NT RawSeq))"
			" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))))"
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"
		},
		{
"(OperatorRule T (STR \" \"))"
"(OperatorRule RE (STR / /))"
"(OperatorRule STR (RawSeq (T ~) (RE [^~,]?)  (T ,)"
"  (RE [^~,]?) (T ~)))"
"(OperatorRule BOW (RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (RE !?) (T ~)))"
"(OperatorRule AddToBag (Seq (NT RE) (T :) (NT symbol) (RE !?)))"
			"(OperatorRule X (NT RawSeq))"
			" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (NT rawseq_contents))))"
			" (TransientRule	rawseq_contents	(Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))"
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"
		},
		{
			"(OperatorRule T (STR \" \"))"
			"(OperatorRule RE (STR / /))"
			"(OperatorRule STR (RawSeq (T ~) (RE [^~,]?)  (T ,)"
			"  (RE [^~,]?) (T ~)))"
			"(OperatorRule BOW (RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (RE !?) (T ~)))"
			"(OperatorRule AddToBag (Seq (NT RE) (T :) (NT symbol) (RE !?)))"
			"(OperatorRule X (NT RawSeq))"
			" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (Seq (Space) (NT rawseq_contents)))))"
			" (TransientRule rawseq_contents (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))"
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"
		},
		{
			"(OperatorRule T (STR \" \"))"
			"(OperatorRule RE (STR / /))"
			"(OperatorRule STR (RawSeq (T ~) (RE [^~,]?)  (T ,)"
			"  (RE [^~,]?) (T ~)))"
			"(OperatorRule BOW (RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (RE !?) (T ~)))"
			"(OperatorRule AddToBag (Seq (NT RE) (T :) (NT symbol) (RE !?)))"
			"(OperatorRule X (NT RawSeq))"
			" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (Alt (NT rawseq_contents)))))"
			" (TransientRule rawseq_contents (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))"
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"
		},
		// now for some more tricky things
		{
			"(OperatorRule X (Alt (Seq (NT X) (NT X)) (T a)))"
			, "aa", "((X (X) (X)))"
		},
		{
			"(OperatorRule T (STR \" \"))"
			"(OperatorRule RE (STR / /))"
			"(OperatorRule STR (RawSeq (T ~) (RE [^~,]?)  (T ,)"
			"  (RE [^~,]?) (T ~)))"
			"(OperatorRule BOW (RawSeq (T ~) (RE [_a-zA-Z][_a-zA-Z0-9]*) (RE !?) (T ~)))"
			"(OperatorRule AddToBag (Seq (NT RE) (T :) (NT symbol) (RE !?)))"
			"(OperatorRule X (NT RawSeq))"
			" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (Seq (Space) (NT rawseq_contents)))))"
			" (TransientRule rawseq_contents (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))"
			, ".raw \"'\" /[\\]?./ \"'\"", "((X (RawSeq (T ') (RE [\\\\]?.) (T '))))"
		},
		{ NULL, NULL, NULL }
	};

	test_case* tc = test_cases;
	unsigned int done=0, ok=0;
	std::streambuf* rdclog = std::clog.rdbuf();
	std::streambuf* rdcerr = std::cerr.rdbuf();
	std::vector<int> failures;
	while((*tc)[0]) {
		std::stringstream capture;
		std::clog.rdbuf(capture.rdbuf());
		std::cerr.rdbuf(capture.rdbuf());
		std::stringstream ofn;
		ofn << "failed.test.";
		ofn << (done+1);
		if(lr::automaton::test(done+1, (*tc)[0], (*tc)[1], (*tc)[2])) {
			++ok;
			std::cout << "[TEST] [automaton] #" << (done+1) << " passed." << std::endl;
			unlink(ofn.str().c_str());
		} else {
			std::ofstream o(ofn.str().c_str(), std::ios_base::out);
			o << capture.str();
			failures.push_back(done+1);
		}
		++done;
		++tc;
	}
	std::clog.rdbuf(rdclog);
	std::cerr.rdbuf(rdcerr);
	for(std::vector<int>::iterator i=failures.begin(), j=failures.end();i!=j;++i) {
		std::cout << "[TEST] [automaton] #" << (*i) << " failed. See ./failed.test." << (*i) << " for output." << std::endl;
	}
	std::cout << "[TEST] [automaton] passed: " << ok << '/' << done << std::endl;
	return ok-done;
}







void test_lr(lr::automaton& a, const char* text) {
	char* str = (char*)tinyap_serialize_to_string(a.parse(text, strlen(text)));
	std::cout << '"' << text << "\" => " << str << std::endl;
	free(str);
}

void test_nl() {
	grammar::Grammar g(Cdr(Car(tinyap_get_ruleset("debug_nl"))));
	lr::automaton nl(&g);
	/*nl.dump_states();*/
	const char* pouet = "I saw a man in the park with a telescope";
	ast_node_t ast = nl.parse(pouet, strlen(pouet));
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
	/*tinyap_init();*/

	return test_grammar() + test_automaton();


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

	return 0;
#endif
}

