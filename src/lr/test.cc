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

	TEST(gc(new token::Nt("foobar"))->is_same(gc(new token::Nt("foobar")))||!"Nt comparison is broken");
	TEST(gc(new token::Nt("foobar"))->is_same(token::Nt::instance("foobar"))||!"Nt comparison is broken");
	TEST_EQ(token::Nt::instance("foobar"), token::Nt::instance("foobar"));

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

	trie_t mybow = grammar::item::token::Bow::find("test");
	trie_t mybow2 = grammar::item::token::Bow::find("test");
	TEST(mybow==mybow2||!"BOW registry doesn't return unique bows");
	trie_insert(mybow, "pouet");
	grammar::item::token::Bow b("test", true);
	TEST(b.recognize("pouet", 0, 5).second==5 ||!"Bow didn't recognize properly with a singleton !");
	
	trie_insert(mybow, "plop");
	TEST(b.recognize("pouet", 0, 5).second==5 ||!"Bow didn't recognize properly with 2 items !");
	TEST(b.recognize("plop", 0, 4).second==4 ||!"Bow didn't recognize properly with 2 items !");

	std::cout << "[TEST] [grammar] passed: " << ok << '/' << done << std::endl;
	return ok-done;
}

int test_automaton() {
	typedef const char* test_case[3];
	test_case test_cases[] = {
		// start rule is X
#if 1
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
			"(TransientRule symbol (RE [_a-zA-Z][0-9a-zA-Z_]*))"
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
			"(TransientRule symbol (RE [_a-zA-Z][0-9a-zA-Z_]*))"
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
			"(TransientRule symbol (RE [_a-zA-Z][0-9a-zA-Z_]*))"
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
			"(TransientRule symbol (RE [_a-zA-Z][0-9a-zA-Z_]*))"
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
			"(TransientRule symbol (RE [_a-zA-Z][0-9a-zA-Z_]*))"
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
#endif
/*60*/	{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi", "((X titi))"
		},
		{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi toto", "((X titi (X toto)))"
		},
		{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi toto tata tutu", "((X titi (X toto (X tata (X tutu)))))"
		},
		{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi toto tata", "((X titi (X toto (X tata))))"
		},
		{ "(OperatorRule X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~)))", " ~\",\"~", "((X \" \"))" },
		{ "(OperatorRule X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~)))", " ~\",\"~ ", "((X \" \"))" },
		{ "(OperatorRule X (BOW _test !))", "pouet", "((X pouet))" },
		{ "(OperatorRule X (BOW _test ))", "pouet", "((X))" },
		{ NULL, NULL, NULL }
	};

	trie_t test_bow = grammar::item::token::Bow::find(regstr("_test"));
	std::clog << "init BOW _test @" << test_bow << std::endl;
	trie_insert(test_bow, "pouet");
	trie_insert(test_bow, "plop");
	trie_insert(test_bow, "coin");

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
			/*std::cout << "[TEST] [automaton] #" << (done+1) << " passed." << std::endl;*/
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
	std::cout << '"' << pouet << "\" => " << ast << std::endl;
	/*while(ast) {*/
		/*wast_t wa = make_wast(Car(ast));*/
		/*tinyap_walk(wa, "prettyprint", NULL);*/
		/*wa_del(wa);*/
		/*ast = Cdr(ast);*/
	/*}*/
	std::ofstream df("nl.dot", std::ios::out);
	df << "digraph i_saw_a_man_in_the_park_with_a_telescope {" << std::endl;
	df << *nl.stack;
	df << '}' << std::endl;
	/*grammar::visitors::debugger debug;*/
	/*g.accept(&debug);*/

}

int main(int argc, char**argv) {
	test_nl();

	/*return 0;*/
	return test_grammar() + test_automaton();
}

