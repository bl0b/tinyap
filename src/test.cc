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

extern volatile int _node_alloc_count;
extern volatile int _node_dealloc_count;
extern volatile int delete_node_count;
extern volatile int newAtom_count;
extern volatile int newPair_count;

struct node_alloc_register {
	int nac;
	int ndc;
	int dnc;
	int ac;
	int pc;
	int done;
	int ok;
	const char* fnam;
	node_alloc_register(const char* _)
		: nac(_node_alloc_count), ndc(_node_dealloc_count)
		, dnc(delete_node_count)
		, ac(newAtom_count), pc(newPair_count)
		, done(0), ok(0)
		, fnam(_)
	{}
	node_alloc_register(int nac_, int ndc_, int dnc_, int ac_, int pc_)
		: nac(nac_), ndc(ndc_)
		, dnc(dnc_)
		, ac(ac_), pc(pc_)
		, done(0), ok(0)
	{}
	node_alloc_register& operator = (const node_alloc_register& t) {
		nac = t.nac;
		ndc = t.ndc;
		dnc = t.dnc;
		ac = t.ac;
		pc = t.pc;
		return *this;
	}
	void update() {
		*this = node_alloc_register(fnam);
	}
	int expect(const char* msg, int exp, int got) {
		std::cout << "[TEST] [" << fnam << "] " << msg << "; expected " << exp << ", got " << got << std::endl;
		return 0;
	}
	bool assert_one(int got, int exp, const char* msg, const char* f, size_t l) {
		if(got!=exp) {
			TEST_FAIL();
			std::cout << "[TEST] [" << fnam << "] " << f << ':' << l << " #" << done << ' ' << msg << "; expected " << exp << ", got " << got << std::endl;
			return false;
		} else {
			TEST_OK();
			return true;
		}
	}
	bool assert_delta_(int nacd, int ndcd, int dncd, int acd, int pcd, const char* f, size_t l) {
		bool ret = true;
		node_alloc_register cur(fnam);
		node_alloc_register delta = cur - *this;
		ret &= assert_one(delta.nac, nacd, "Wrong number of node allocations", f, l);
		ret &= assert_one(delta.ndc, ndcd, "Wrong number of node deallocations", f, l);
		ret &= assert_one(delta.dnc, dncd, "Wrong number of calls to delete_node", f, l);
		ret &= assert_one(delta.ac, acd, "Wrong number of calls to newAtom", f, l);
		ret &= assert_one(delta.pc, pcd, "Wrong number of calls to newPair", f, l);
		update();
		return ret;
	}
	bool assert_node_ref_(ast_node_t node, int ref, const char* f, size_t l) {
		return assert_one(node->raw.ref, ref, "Wrong reference counter", f, l);
	}
	node_alloc_register operator -(const node_alloc_register& t) const {
		return node_alloc_register(nac-t.nac, ndc-t.ndc, dnc-t.dnc, ac-t.ac, pc-t.pc);
	}
#define assert_delta(_1, _2, _3, _4, _5) assert_delta_(_1, _2, _3, _4, _5, __FILE__, __LINE__)
#define assert_node_ref(_1, _2) assert_node_ref_(_1, _2, __FILE__, __LINE__)
#define assert_eq(_got, _exp, _msg) assert_one(_got, _exp, _msg, __FILE__, __LINE__)
};


#define test_ONE_ATOM_ALLOC() assert_delta(1, 0, 0, 1, 0)
#define test_ONE_PAIR_ALLOC() assert_delta(1, 0, 0, 0, 1)
#define test_ONE_ATOM_REUSE() assert_delta(0, 0, 0, 1, 0)
#define test_ONE_PAIR_REUSE() assert_delta(0, 0, 0, 0, 1)
#define test_ONE_DEALLOC()    assert_delta(0, 1, 1, 0, 0)
#define test_ONE_DEREF()      assert_delta(0, 0, 1, 0, 0)




int hunt_leak() {
	return 0;
}


int test_append() {
	node_alloc_register alloc0("append");
	ast_node_t a,b,c;
	grammar::rule::internal::append append;
	a = PRODUCTION_OK_BUT_EMPTY;
	b = PRODUCTION_OK_BUT_EMPTY;
	c = append(a, b);
	alloc0.assert_delta(0, 0, 0, 0, 0);
	a = newPair(newAtom("w", 0), NULL);
	alloc0.assert_delta(2, 0, 0, 1, 1);
	b = PRODUCTION_OK_BUT_EMPTY;
	c = append(a, b);
	alloc0.ok += (c==a); alloc0.done++;
	c = append(b, a);
	alloc0.ok += (c==a); alloc0.done++;

	a = newPair(newAtom("tata", 0), NULL);
	a->raw.ref++;
	alloc0.assert_delta(2, 0, 0, 1, 1);

	c = append(a, a);
	alloc0.assert_delta(1, 0, 0, 0, 1);

	delete_node(c);
	alloc0.assert_delta(0, 1, 3, 0, 0);
	

	b = newPair(newAtom("toto", 0), newPair(newAtom("titi", 0), NULL));
	b->raw.ref++;
	alloc0.assert_delta(4, 0, 0, 2, 2);
	c = append(a, b);
	alloc0.assert_delta(1, 0, 0, 0, 1);
	delete_node(a);
	delete_node(b);
	delete_node(c);
	alloc0.assert_delta(0, 6, 6, 0, 0);
	return alloc0.ok-alloc0.done;
}

int test_nodealloc() {
	node_alloc_register alloc0("nodealloc");
	ast_node_t x, y, z;
	x = newAtom("omaeifnlkfubvljrbv", -5);
	x->raw.ref++;
	alloc0.assert_node_ref(x, 1);
	alloc0.test_ONE_ATOM_ALLOC();
	delete_node(x);
	alloc0.test_ONE_DEALLOC();
	y = newAtom("omaeiufnvalkfubvlzakejrbv", -1);
	y->raw.ref++;
	alloc0.test_ONE_ATOM_ALLOC();
	y = newAtom("omaeiufnvalkfubvlzakejrbv", -1);
	y->raw.ref++;
	alloc0.assert_node_ref(y, 2);
	alloc0.test_ONE_ATOM_REUSE();
	delete_node(y);
	alloc0.test_ONE_DEREF();
	delete_node(y);
	alloc0.test_ONE_DEALLOC();
	z = newPair(newAtom("aemlhg", -1), newAtom("aemfuhvn", -12));
	z->raw.ref++;
	alloc0.assert_delta(3, 0, 0, 2, 1);
	alloc0.assert_node_ref(z, 1);
	alloc0.assert_node_ref(z->pair._car, 1);
	alloc0.assert_node_ref(z->pair._cdr, 1);
	delete_node(z);
	alloc0.assert_delta(0, 3, 3, 0, 0);
	std::cout << "[TEST] [node_alloc] passed: " << alloc0.ok << '/' << alloc0.done << std::endl;

	return alloc0.ok-alloc0.done;
}




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
	ast_node_t tmp_ast;

	TEST_EQ((tmp_ast=newAtom("toto", 0)), newAtom("toto", 0));
	delete_node(tmp_ast);
	TEST_EQ((tmp_ast=newPair(newAtom("toto", 0), NULL)), newPair(newAtom("toto", 0), NULL));
	delete_node(tmp_ast);

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
	std::pair<ast_node_t, size_t> tmp_rec;
	TEST((tmp_rec = b.recognize("pouet", 0, 5)).second==5 ||!"Bow didn't recognize properly with a singleton !");
	delete_node(tmp_rec.first);
	
	trie_insert(mybow, "plop");
	TEST((tmp_rec=b.recognize("pouet", 0, 5)).second==5 ||!"Bow didn't recognize properly with 2 items !");
	delete_node(tmp_rec.first);
	TEST((tmp_rec=b.recognize("plop", 0, 4)).second==4 ||!"Bow didn't recognize properly with 2 items !");
	delete_node(tmp_rec.first);

	std::cout << "[TEST] [grammar] passed: " << ok << '/' << done << std::endl;
	return ok-done;
}


void automaton_post_init() {
    trie_t test_bow = grammar::item::token::Bow::find(regstr("_test"));
    std::clog << "init BOW _test @" << test_bow << std::endl;
    trie_insert(test_bow, "pouet");
    trie_insert(test_bow, "plop");
    trie_insert(test_bow, "coin");
}


int test_automaton(int n=-1) {
	typedef const char* test_case[3];
	test_case test_cases[] = {
		// start rule is X
/*#if 1*/
/*1*/
		{ "(OperatorRule X (EOF))", "", "((X:0))" },
		{ "(OperatorRule X (EOF))", "   ", "((X:3))" },
		{ "(OperatorRule X (EOF))", "a", NULL },
		{ "(OperatorRule X (Epsilon))", "", "((X:0))" },
		{ "(OperatorRule X (Epsilon))", "a", NULL },
		{ "(OperatorRule X (Epsilon))", "   ", "((X:3))" },
		{ "(OperatorRule X (T toto))", "toto", "((X:4))" },
		{ "(OperatorRule X (T toto))", "pouet", NULL },
		{ "(OperatorRule X (RE toto))", "toto", "((X:4 toto:0))" },

		/*{ "(OperatorRule X (Seq (T foo) (Rep01 (T pouet))))", "foo pouet", "((X))" },*/
#if 1

/*10*/	{ "(OperatorRule X (Rep01 (T pouet)))", "pouet", "((X:5))" },
		{ "(OperatorRule X (Rep01 (T pouet)))", "", "((X:0))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "pouet", "((X:5))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "pouet pouet pouet", "((X:17))" },
		{ "(OperatorRule X (Rep0N (T pouet)))", "", "((X:0))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "pouet", "((X:5))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "pouet pouet pouet", "((X:17))" },
		{ "(OperatorRule X (Rep1N (T pouet)))", "", NULL },

		{ "(OperatorRule X (Seq (T foo) (Rep01 (T pouet))))", "foo pouet", "((X:9))" },
		{ "(OperatorRule X (Seq (T foo) (Rep01 (T pouet))))", "foo ", "((X:4))" },
/*20*/	{ "(OperatorRule X (Seq (T foo) (Rep0N (T pouet))))", "foo pouet", "((X:9))" },
		{ "(OperatorRule X (Seq (T foo) (Rep0N (T pouet))))", "foo pouet pouet pouet", "((X:21))" },
		{ "(OperatorRule X (Seq (T foo) (Rep0N (T pouet))))", "foo ", "((X:4))" },
		{ "(OperatorRule X (Seq (T foo) (Rep1N (T pouet))))", "foo pouet", "((X:9))" },
		{ "(OperatorRule X (Seq (T foo) (Rep1N (T pouet))))", "foo pouet pouet pouet", "((X:21))" },
		{ "(OperatorRule X (Seq (T foo) (Rep1N (T pouet))))", "foo", NULL },

		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "pouet", "((X:5))" },
		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "", "((X:0))" },
		{ "(OperatorRule X (Alt (T foo) (Rep0N (T pouet))))", "pouet", "((X:5))" },
		{ "(OperatorRule X (Alt (T foo) (Rep0N (T pouet))))", "pouet pouet pouet", "((X:17))" },
/*30*/	{ "(OperatorRule X (Alt (T foo) (Rep0N (T pouet))))", "", "((X:0))" },
		{ "(OperatorRule X (Alt (T foo) (Rep1N (T pouet))))", "pouet", "((X:5))" },
		{ "(OperatorRule X (Alt (T foo) (Rep1N (T pouet))))", "pouet pouet pouet", "((X:17))" },
		{ "(OperatorRule X (Alt (T foo) (Rep1N (T pouet))))", "", NULL },

		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "foo", "((X:3))" },
		{ "(OperatorRule X (Alt (T foo) (Rep01 (T pouet))))", "foo foo", NULL },

		{ "(OperatorRule X (NT Y)) (OperatorRule Y (Epsilon))", "", "((X:0 (Y:0)))" },
		{ "(OperatorRule X (Prefix (RE toto) (NT Y))) (OperatorRule Y (RE pouet))", "totopouet", "((X:9 (Y:9 toto:0 pouet:4)))" },
		{ "(OperatorRule X (Prefix (NT Z) (NT Y))) (TransientRule Z (Seq (RE toto) (RE pou))) (OperatorRule Y (RE et))", "totopouet", "((X:9 (Y:9 toto:0 pou:4 et:7)))" },
		{ "(OperatorRule X (Postfix (RE toto) (NT Y))) (OperatorRule Y (RE pouet))", "totopouet", "((X:9 (Y:9 pouet:4 toto:0)))" },
/*40*/	{ "(OperatorRule X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~)))", "~\",\"~", "((X:5 \":1 \":3))" },
		{ "(OperatorRule X (Rep1N (NT pouet))) (OperatorRule pouet (RE pouet))", "pouet pouet pouet", "((X:17 (pouet:6 pouet:0) (pouet:12 pouet:6) (pouet:17 pouet:12)))" },
		{ "(OperatorRule X (Rep1N (NT coin))) (OperatorRule coin (RE pouet))", "pouet pouet pouet", "((X:17 (coin:6 pouet:0) (coin:12 pouet:6) (coin:17 pouet:12)))" },
		{ "(OperatorRule X (Rep1N (NT coin))) (OperatorRule coin (RawSeq (RE pou) (RE ..)))", "pouet pouat pouit", "((X:17 (coin:6 pou:0 et:3) (coin:12 pou:6 at:9) (coin:17 pou:12 it:15)))" },
		{ "(OperatorRule X (RawSeq (T to) (RE ...) (T ouet)))", "totopouet", "((X:9 top:2))" },
		{
			"(OperatorRule X (RawSeq (T #) (RE [^\\\\r\\n]*)))"
			, "# toto pouet", "((X:12 \\ toto\\ pouet:1))"
		},
		{
			"(OperatorRule X (NT Comment))"
			"(OperatorRule Comment (Alt (T #\\n) (T #\\r\\n) (RawSeq (T #) (RE [^\\\\r\\\\n]+))))"
			, "# toto pouet\n", "((X:13 (Comment:13 \\ toto\\ pouet:1)))"
		},
		{
			"(OperatorRule X (NT Comment))"
			"(OperatorRule Comment (Alt (T #\\n) (T #\\r\\n) (RawSeq (T #) (RE [^\\\\r\\\\n]+))))"
			, "\r\n# toto pouet\n\r\n", "((X:17 (Comment:17 \\ toto\\ pouet:3)))"
		},
		{
			"(OperatorRule X (Seq (NT Comment) (NT Comment) (NT Comment)))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			, "# toto pouet\n#\n#toto", "((X:20 (Comment:13 \\ toto\\ pouet:1) (Comment:15 :14) (Comment:20 toto:16)))"
		},
		{
			"(OperatorRule X (Rep1N (NT Comment)))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			, "# toto pouet\n#\n#toto", "((X:20 (Comment:13 \\ toto\\ pouet:1) (Comment:15 :14) (Comment:20 toto:16)))"
		},
/*50*/	{
			"(OperatorRule X (Rep1N (Alt (RE toto) (RE pouet))))"
			, "toto toto pouet\t toto", "((X:21 toto:0 toto:5 pouet:10 toto:17))"
		},
		{
			"(OperatorRule X (Rep1N (Alt (NT Comment))))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			/*, "# toto pouet\n#\n#toto", "((X (Comment \\ toto\\ pouet) (Comment ) (Comment toto)))"*/
			, "# toto pouet\n#\n#toto", "((X:20 (Comment:13 \\ toto\\ pouet:1) (Comment:15 :14) (Comment:20 toto:16)))"
		},
		{
			"(OperatorRule X (NT Grammar))"
			"(OperatorRule Grammar (Rep1N (Alt (NT Comment))))"
			"(OperatorRule Comment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			/*, "# toto pouet\n#\n#toto", "((X (Grammar (Comment \\ toto\\ pouet) (Comment ) (Comment toto))))"*/
			, "# toto pouet\n#\n#toto", "((X:20 (Grammar:20 (Comment:13 \\ toto\\ pouet:1) (Comment:15 :14) (Comment:20 toto:16))))"
		},
		{
			"(OperatorRule X (NT Gramar))"
			"(OperatorRule Gramar (Rep1N (Alt (NT Coment))))"
			"(OperatorRule Coment (RawSeq (T #) (RE [^\\\\r\\\\n]*)))"
			/*, "# toto pouet\n#\n#toto", "((X (Gramar (Coment \\ toto\\ pouet) (Coment ) (Coment toto))))"*/
			, "# toto pouet\n#\n#toto", "((X:20 (Gramar:20 (Coment:13 \\ toto\\ pouet:1) (Coment:15 :14) (Coment:20 toto:16))))"
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
			" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))))"
			/*, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"*/
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X:34 (RawSeq:34 (T:9 ~:5) (RE:18 [^~,]?:9) (T:22 ,:18) (RE:31 [^~,]?:22) (T:34 ~:31))))"
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
			" (OperatorRule RawSeq (Seq (T .raw) (Rep1N (NT rawseq_contents))))"
			" (TransientRule	rawseq_contents	(Alt (NT T) (NT STR) (NT RE) (NT BOW) (NT AddToBag)))"
			/*, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"*/
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X:34 (RawSeq:34 (T:9 ~:5) (RE:18 [^~,]?:9) (T:22 ,:18) (RE:31 [^~,]?:22) (T:34 ~:31))))"
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
			/*, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"*/
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X:34 (RawSeq:34 (T:9 ~:5) (RE:18 [^~,]?:9) (T:22 ,:18) (RE:31 [^~,]?:22) (T:34 ~:31))))"
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
			/*, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~))))"*/
			, ".raw \"~\" /[^~,]?/ \",\" /[^~,]?/ \"~\"", "((X:34 (RawSeq:34 (T:9 ~:5) (RE:18 [^~,]?:9) (T:22 ,:18) (RE:31 [^~,]?:22) (T:34 ~:31))))"
		},
		// now for some more tricky things
		{
			"(OperatorRule X (Alt (Seq (NT X) (NT X)) (T a)))"
			, "aa", "((X:2 (X:1) (X:2)))"
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
			/*, ".raw \"'\" /[\\]?./ \"'\"", "((X (RawSeq (T ') (RE [\\\\]?.) (T '))))"*/
			, ".raw \"'\" /[\\]?./ \"'\"", "((X:20 (RawSeq:20 (T:9 ':5) (RE:17 [\\\\]?.:9) (T:20 ':17))))"
		},
/*#endif*/
/*60*/	{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi", "((X:4 titi:0))"
		},
		{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi toto", "((X:9 titi:0 (X:9 toto:5)))"
		},
		{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi toto tata tutu", "((X:19 titi:0 (X:19 toto:5 (X:19 tata:10 (X:19 tutu:15)))))"
		},
		{	"(OperatorRule X (Alt (Seq (RE \\w+) (NT X)) (RE \\w+)))"
			, "titi toto tata", "((X:14 titi:0 (X:14 toto:5 (X:14 tata:10))))"
		},
		{ "(OperatorRule X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~)))", " ~\",\"~", "((X:6 \":2 \":4))" },
		{ "(OperatorRule X (RawSeq (T ~) (RE [^~,]?) (T ,) (RE [^~,]?) (T ~)))", " ~\",\"~ ", "((X:7 \":2 \":4))" },
		{ "(OperatorRule X (BOW _test !))", "pouet", "((X:5 pouet:0))" },
		{ "(OperatorRule X (BOW _test ))", "pouet", "((X:5))" },
        { "(OperatorRule X (Prefix (Prefix (NT a) (NT b)) (NT b)))"
          "(OperatorRule a (RE toto))"
          "(OperatorRule b (RE pouet))"
          , "toto pouet pouet", "((X:16 (b:16 (b:11 (a:5 toto:0) pouet:5) pouet:11)))"
        },
        { "(OperatorRule X (Seq (AddToBag (RE toto) bag !) (BOW bag !)))", "toto toto", "((X:9 toto:0 toto:5))" },
        { "(OperatorRule X (Seq (AddToBag (RE toto) bag !) (BOW bag )))", "toto toto", "((X:9 toto:0))" },
        { "(OperatorRule X (Alt (NT a) (NT b) (NT c))) (OperatorRule a (T pouet)) (OperatorRule b (RE pouet)) (OperatorRule c (BOW _test))"
          , "pouet", "((X:5 (a:5)))" },
        { "(OperatorRule X (Alt (NT a) (NT b) (NT c))) (OperatorRule a (T toto)) (OperatorRule b (RE pouet)) (OperatorRule c (BOW _test))"
          , "pouet", "((X:5 (c:5)))" },
        { "(OperatorRule X (Alt (NT a) (NT b) (NT c))) (OperatorRule a (T pouet)) (OperatorRule b (RE toto)) (OperatorRule c (BOW _test))"
          , "toto", "((X:4 (b:4 toto:0)))" },
        /*{ "(OperatorRule X (Seq (AddToBag (RE toto) bag ) (BOW bag !)))", "toto toto", "((X:9 toto:5))" },*/
        { "(OperatorRule X (Seq (T toto) (STR  )))", "toto pouet ", "((X:11 pouet\\ :5))" },
        { "(OperatorRule X (Seq (T toto) (NT a)))"
          "(OperatorRule a (Seq (STR  ) (EOF)))", "toto ", NULL },
        { "(OperatorRule X (Seq (T toto) (NT a)))"
          "(OperatorRule a (Seq (STR  ) (EOF)))", "toto pouet",
          "((X:10 (a:10 pouet:5)))"
        },
#endif
		{ NULL, NULL, NULL }
	};

	test_case* tc = test_cases;
	unsigned int done=0, ok=0;
	std::streambuf* rdclog = std::clog.rdbuf();
	std::streambuf* rdcerr = std::cerr.rdbuf();
	std::vector<int> failures;
	tinyap_terminate();
	while((*tc)[0]&&n--) {
		std::cerr << "\r[automaton] #" << (done+1) << "...";
		/*int alloc_delta0 = _node_alloc_count-_node_dealloc_count;*/
		/*int alloc_delta2 = newPair_count + newAtom_count - delete_node_count;*/
		std::stringstream capture;
		std::clog.rdbuf(capture.rdbuf());
		std::cerr.rdbuf(capture.rdbuf());
		std::stringstream ofn;
		ofn << "failed.test.";
		ofn << (done+1);
		if(lr::automaton::test(done+1, (*tc)[0], (*tc)[1], (*tc)[2], automaton_post_init)) {
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
		/*int alloc_delta1 = _node_alloc_count-_node_dealloc_count;*/
		/*int alloc_delta3 = newPair_count + newAtom_count - delete_node_count;*/
		/*if(alloc_delta1 != alloc_delta0) {*/
			/*std::cout << "[TEST] [automaton] #" << done << " leak detected (" << (alloc_delta1-alloc_delta0) << ')' << std::endl;*/
		/*}*/
		/*if(alloc_delta2 != alloc_delta3) {*/
			/*std::cout << "[TEST] [automaton] #" << done << " new/delete leak detected (" << (alloc_delta3-alloc_delta2) << ')' << std::endl;*/
		/*}*/
		std::clog.rdbuf(rdclog);
		std::cerr.rdbuf(rdcerr);
	}
	std::cerr << '\r';
	tinyap_init();
	for(std::vector<int>::iterator i=failures.begin(), j=failures.end();i!=j;++i) {
		std::cout << "[TEST] [automaton] #" << (*i) << " failed. See ./failed.test." << (*i) << " for output." << std::endl;
	}
	std::cout << "[TEST] [automaton] passed: " << ok << '/' << done << std::endl;
	return ok-done;
}



int test_tinyap_append() {
    tinyap_t p = tinyap_new();
    tinyap_set_grammar_ast(p, ast_unserialize("((Grammar (TransientRule _start (NT X))"
                "(OperatorRule X (Rep1N (T toto)))"
                "))"));
    tinyap_append_grammar(p, ast_unserialize("((Grammar (TransientRule X (T pouet))))"));
    tinyap_set_source_buffer(p, "pouet", 5);
    tinyap_parse(p, true);
    if(!tinyap_get_output(p)) {
        std::clog << "no output" << std::endl;
        return 1;
    }
    tinyap_delete(p);
    return 0;
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
	Ast ast = nl.parse(pouet, strlen(pouet));
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
	int n=-1;
	if(argc>1) {
		std::stringstream(argv[1]) >> n;
	}
	/*test_nl();*/

	/*return 0;*/
	return test_nodealloc() + test_grammar() + test_automaton(n) + test_tinyap_append();
	/*return test_nodealloc();*/
}

