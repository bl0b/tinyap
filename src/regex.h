#ifndef _TINYAML_REGEX_H_
#define _TINYAML_REGEX_H_

#include "lr_base.h"
/*#include "lr_grammar.h"*/

#include <memory>
#include <deque>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <initializer_list>

#define MAX_CHAR 127

typedef unsigned char re_char_t;

static inline std::ostream& operator << (std::ostream& os, const std::vector<int>& s)
{
    os << '{';
    auto i = s.begin(), j = s.end();
    if (i != j) {
        os << (*i);
        for (++i; i != j; ++i) {
            os << ',' << (*i);
        }
    }
    os << '}';
    return os;
}

template <typename T>
struct set {
    typedef std::vector<T> container_type;
    typedef T value_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    std::vector<T> data;

    set(size_t res) : data(res) {}

    set(std::initializer_list<T> elems)
        : data(elems)
    {}

    set(const set& s) : data(s.data) {}
    set(set&& s) : data(s.data) {}
    set() : data() {}

    iterator begin() { return data.begin(); }
    iterator end() { return data.end(); }
    const_iterator begin() const { return data.cbegin(); }
    const_iterator end() const { return data.cend(); }
    const_iterator cbegin() const { return data.cbegin(); }
    const_iterator cend() const { return data.cend(); }

    void sort() { std::sort(begin(), end()); }

    void clear() { data.clear(); }
    size_t size() const { return data.size(); }

    void insert(const T& t) { data.push_back(t); }
    void insert(T&& t) { data.push_back(t); }

    void insert(const set& s)
    {
        set output(data.size() + s.data.size());
        /*std::cout << data << std::endl;*/
        std::set_union(data.begin(), data.end(), s.begin(), s.end(), output.data.begin());
        /*std::swap(output.data, data);*/
        data.swap(output.data);
    }

    const T& back() const { return data.back(); }

    set& operator = (const set& s)
    {
        data = s.data;
        return *this;
    }

    set& operator = (set&& s)
    {
        /*std::swap(data, s.data);*/
        data.swap(s.data);
        return *this;
    }

    static set& set_union(const set& output) { return output; }

    static set set_union(const set& scar, std::initializer_list<const set&> scdr)
    {
        set big = set_union(scdr);
        set output(scar.size() + big.data.size());
        std::set_union(scar.begin(), scar.end(),
                       big.begin(), big.end(),
                       output.begin());
        return output;
    }

    bool operator < (const set<T>& s) const
    {
        auto si = s.begin(), sj = s.end(), i = begin(), j = end();
        while (si != sj && i != j && *si == *i) { ++si; ++i; }
        bool s_not_at_end = si != sj;
        return i == j ? s_not_at_end
                      : s_not_at_end ? *i < *si
                                     : false;
    }
};


struct character_class_t {
    unsigned long long cls[2];

    typedef std::pair<int, unsigned long long> split_t;

    split_t split(char c) { return {!!(c & 64), 1LL << (c & 63)}; }

    void _set(char c) {
        split_t spl = split(c);
        /*std::cout << "cls[" << spl.first << "] |= " << std::hex << spl.second << std::endl;*/
        cls[spl.first] |= spl.second;
    }

    void _reset(char c) {
        split_t spl = split(c);
        cls[spl.first] &= ~spl.second;
    }

    bool _test(char c) {
        split_t spl = split(c);
        return !!(cls[spl.first] & spl.second);
    }

    void insert(char c) { _set(c); }
    void insert(const character_class_t& c) { cls[0] |= c.cls[1]; cls[1] |= c.cls[1]; }

    void erase(char c) { _reset(c); }

    void set_all() { cls[0] = cls[1] = ~(0LL); }
    void reset_all() { cls[0] = cls[1] = 0LL; }
    void toggle(char c)
    {
        split_t spl = split(c);
        cls[spl.first] ^= spl.second;
    }

    struct iterator {
        unsigned long long buffer[2];
        unsigned char pos;

        void _find_next()
        {
            if (pos == 128) {
                /*std::cout << std::endl << "|_find_next at end" << std::endl;*/
                return;
            }

            int i = !!(pos >> 6);
            /*std::cout << std::endl << "|_find_next i=" << i << std::endl;*/

            if (buffer[i] == 0) {
                pos = (i + 1) << 6;
                /*std::cout << std::endl << "|_find_next empty buffer! #" << i << " set pos=" << ((int)pos) << std::endl;*/
                _find_next();
                return;
            }

            int skip = __builtin_ffsll(buffer[i]);
            /*std::cout << std::endl << "|buffer=" << buffer[i] << " skip=" << skip << std::endl;*/
            pos += skip;
            buffer[i] >>= skip;
            /*std::cout << std::endl << "|_find_next pos=" << ((int)pos) << std::endl;*/
        }

        iterator(unsigned long long c0, unsigned long long c1)
            : pos(!c0 ? 64 + (!c1 ? 64 : 0) : 0)
        {
            buffer[0] = c0;
            buffer[1] = c1;
            _find_next();
        }
        char operator * () const { return pos - 1; }
        iterator& operator ++ () { _find_next(); return *this; }
        bool operator == (const iterator& i)
        {
            /*std::cout << "compare pos " << ((int)pos) << '/' << ((int)i.pos) << std::endl;*/
            return pos == i.pos;
        }
        bool operator != (const iterator& i)
        {
            /*std::cout << "compare pos " << ((int)pos) << '/' << ((int)i.pos) << std::endl;*/
            return pos != i.pos;
        }
    };

    character_class_t() { cls[0] = cls[1] = 0; }

    bool find(char c) { return _test(c); }

    bool empty() const { return cls[0] == 0 && cls[1] == 0; }
    bool not_empty() const { return cls[0] != 0 || cls[1] != 0; }

    iterator begin() const { return {cls[0], cls[1]}; }
    iterator end() const { return {0, 0}; }
};


static inline std::ostream& operator << (std::ostream& os, const set<int>& s)
{
    return os << s.data;
}

static inline std::ostream& operator << (std::ostream& os, const character_class_t& s)
{
    os << '[';
    for (char c: s) {
        if (c < 32) {
            os << '\\' << ((int)c);
        } else {
            os << c;
        }
    }
    return os << ']';
}

template <typename TokenType>
struct re_ast_node {
    typedef std::vector<re_ast_node<TokenType>*> vector;
    int leaf_num;
    bool grouping;
    TokenType accept_token;
    character_class_t character_class;
    set<int> followpos;
    set<int> m_firstpos;
    set<int> m_lastpos;
    re_ast_node<TokenType>* child1;
    re_ast_node<TokenType>* child2;

    virtual std::ostream& output(std::ostream& os)
    {
        return _output(os, "");
    }

    std::ostream& _output(std::ostream& os, const std::string& pfx)
    {
        static int indent = 0;
        os << std::setw(indent * 4) << "" << pfx;
        os << '(';
        os << '#' << firstpos() << ' ';
        if (character_class.not_empty()) {
            os << character_class << ' ';
            if (leaf_num) {
                os << leaf_num << ' ';
            }
        }
        if (child1) {
            ++indent;
            os << std::endl;
            child1->output(os);
            --indent;
        }
        if (child2) {
            ++indent;
            os << std::endl;
            child2->output(os);
            --indent;
        }
        if (child1 || child2) {
            os << std::endl << std::setw(indent * 4 + 2) << "";
        }
        return os << '$' << lastpos() << " F" << followpos << ')';
    }

    re_ast_node()
        : leaf_num(0), grouping(false), accept_token(0), character_class(), child1(0), child2(0)
    {}
    re_ast_node(re_ast_node<TokenType>* a)
        : leaf_num(0), grouping(false), accept_token(0), character_class(), child1(a), child2(0)
    {}
    re_ast_node(re_ast_node<TokenType>* a, re_ast_node<TokenType>* b)
        : leaf_num(0), grouping(false), accept_token(0), character_class(), child1(a), child2(b)
    {}
    virtual ~re_ast_node()
    {
        if (child1) {
            delete child1;
        }
        if (child2) {
            delete child2;
        }
    }

    int reorder(vector& table, int num=1)
    {
        if (!(child1 || child2)) {
            leaf_num = num;
            table.push_back(this);
            m_firstpos.clear();
            m_lastpos.clear();
            return ++num;
        }
        if (child1) {
            num = child1->reorder(table, num);
        }
        if (child2) {
            num = child2->reorder(table, num);
        }
        return num;
    }

    virtual void followpos_rule(vector& fp_table) {}

    void compute_followpos(vector& table)
    {
        if (child2) {
            child2->compute_followpos(table);
            followpos.insert(child2->followpos);
        }
        if (child1) {
            child1->compute_followpos(table);
            followpos.insert(child1->followpos);
        }
        followpos_rule(table);
    }

    virtual void propagate_followpos()
    {
        if (child2) {
            child2->propagate_followpos();
            followpos.insert(child2->followpos);
        }
        if (child1) {
            child1->propagate_followpos();
            if (!child2 || child2->nullable()) {
                followpos.insert(child1->followpos);
            }
            /* also add fp to child1 if child2 is nullable! */
            if (child2 && child2->nullable()) {
                child1->followpos.insert(child2->followpos);
            }
        }
    }

    virtual bool nullable()
    {
        return character_class.empty();
    }

    virtual set<int>& firstpos_()
    {
        if (!leaf_num) {
            return m_firstpos;
        }
        if (!nullable()) {
            m_firstpos.insert(leaf_num);
        }
        return m_firstpos;
        /*return ret;*/
    }

    virtual set<int>& lastpos_()
    {
        if (leaf_num) {
            /*m_lastpos = firstpos();*/
            m_lastpos.insert(leaf_num);
        }
        return m_lastpos;
    }

    set<int>& firstpos()
    {
        if (m_firstpos.size()) {
            return m_firstpos;
        }
        return firstpos_();
    }

    set<int>& lastpos()
    {
        if (m_lastpos.size()) {
            return m_lastpos;
        }
        return lastpos_();
    }
};

template <typename TokenType>
using re_ast_node_vector = typename re_ast_node<TokenType>::vector;

template <typename TokenType>
struct final_node : re_ast_node<TokenType> {
    using re_ast_node<TokenType>::m_firstpos;
    using re_ast_node<TokenType>::m_lastpos;
    using re_ast_node<TokenType>::child1;
    using re_ast_node<TokenType>::child2;
    using re_ast_node<TokenType>::_output;
    using re_ast_node<TokenType>::accept_token;
    using re_ast_node<TokenType>::leaf_num;
    final_node(TokenType tok)
        : re_ast_node<TokenType>()
    { accept_token = tok; }
    set<int>& firstpos_() {
        m_firstpos = {leaf_num};
        return m_firstpos;
    }
    set<int>& lastpos_() {
        m_lastpos = {leaf_num};
        return m_lastpos;
    }
    bool nullable() { return false; }

    std::ostream& output(std::ostream& os)
    {
        return _output(os, "FINAL");
    }
};

template <typename TokenType>
struct initial_node : re_ast_node<TokenType> {
    using re_ast_node<TokenType>::m_firstpos;
    using re_ast_node<TokenType>::m_lastpos;
    using re_ast_node<TokenType>::_output;
    using re_ast_node<TokenType>::leaf_num;
    initial_node() : re_ast_node<TokenType>() {}
    set<int>& firstpos_() {
        m_firstpos = {leaf_num};
        return m_firstpos;
    }
    set<int>& lastpos_() {
        m_lastpos = {leaf_num};
        return m_lastpos;
    }
    bool nullable() { return false; }

    std::ostream& output(std::ostream& os)
    {
        return _output(os, "INITIAL");
    }
};

template <typename TokenType>
struct cat_node : re_ast_node<TokenType> {
    using re_ast_node<TokenType>::child1;
    using re_ast_node<TokenType>::child2;
    using re_ast_node<TokenType>::_output;
    using re_ast_node<TokenType>::m_firstpos;
    using re_ast_node<TokenType>::m_lastpos;

    bool m_nullable_init;
    bool m_nullable;

    cat_node(re_ast_node<TokenType>* a, re_ast_node<TokenType>* b)
        : re_ast_node<TokenType>(a, b)
        , m_nullable_init(false)
    {}

    virtual void followpos_rule(re_ast_node_vector<TokenType>& table)
    {
        /*std::cout << "cat_node::followpos_rule" << std::endl;*/
        set<int>& fp2 = child2->firstpos();
        /*std::cout << "fp2=" << fp2 << std::endl;*/
        for (auto lp: child1->lastpos()) {
            /*std::cout << lp << "<->" << table[lp]->leaf_num << std::endl;*/
            table[lp]->followpos.insert(fp2);
        }
    }

    virtual bool nullable()
    {
        if (!m_nullable_init) {
            m_nullable = child1->nullable() && child2->nullable();
            m_nullable_init = true;
        }
        return m_nullable;
    }

    virtual set<int>& firstpos_()
    {
        m_firstpos = child1->firstpos();
        if (child1->nullable()) {
            m_firstpos.insert(child2->firstpos());
            /*set<int> tmp = child2->firstpos();*/
            /*ret.insert(tmp.begin(), tmp.end());*/
        }
        return m_firstpos;
        /*return ret;*/
    }

    virtual set<int>& lastpos_()
    {
        m_lastpos = child2->lastpos();
        if (child2->nullable()) {
            m_lastpos.insert(child2->firstpos());
            /*set<int> tmp = child2->firstpos();*/
            /*tmp.insert(ret.begin(), ret.end());*/
            /*return tmp;*/
        }
        return m_lastpos;
    }

    virtual std::ostream& output(std::ostream& os)
    {
        return _output(os, "CAT");
    }
};

template <typename TokenType>
struct or_node : re_ast_node<TokenType> {
    using re_ast_node<TokenType>::m_firstpos;
    using re_ast_node<TokenType>::m_lastpos;
    using re_ast_node<TokenType>::child1;
    using re_ast_node<TokenType>::child2;
    using re_ast_node<TokenType>::_output;

    bool m_nullable_init;
    bool m_nullable;

    or_node(re_ast_node<TokenType>* a, re_ast_node<TokenType>* b)
        : re_ast_node<TokenType>(a, b)
        , m_nullable_init(false)
    {}

    virtual bool nullable()
    {
        if (!m_nullable_init) {
            m_nullable = child1->nullable() || child2->nullable();
        }
        return m_nullable;
    }

    virtual set<int>& firstpos_()
    {
        m_firstpos = child1->firstpos();
        m_firstpos.insert(child2->firstpos());
        return m_firstpos;
        /*set<int> ret = child1->firstpos();*/
        /*set<int> tmp = child2->firstpos();*/
        /*ret.insert(tmp.begin(), tmp.end());*/
        /*return ret;*/
    }

    virtual set<int>& lastpos_()
    {
        m_lastpos = child1->lastpos();
        m_lastpos.insert(child2->lastpos());
        return m_firstpos;
        /*set<int> ret = child1->lastpos();*/
        /*set<int> tmp = child2->lastpos();*/
        /*ret.insert(tmp.begin(), tmp.end());*/
        /*return ret;*/
    }

    virtual std::ostream& output(std::ostream& os)
    {
        return _output(os, "OR");
    }
};

template <typename TokenType>
struct star_node : re_ast_node<TokenType> {
    using re_ast_node<TokenType>::m_firstpos;
    using re_ast_node<TokenType>::m_lastpos;
    using re_ast_node<TokenType>::child1;
    using re_ast_node<TokenType>::_output;
    star_node(re_ast_node<TokenType>* a)
        : re_ast_node<TokenType>(a)
    {}

    virtual void followpos_rule(re_ast_node_vector<TokenType>& table)
    {
        set<int>& lp1 = child1->lastpos();
        set<int>& fp1 = child1->firstpos();
        /*set<int>::iterator i, j;*/
        /*i = lp1.begin();*/
        /*j = lp1.end();*/
        /*for (; i != j; ++i) {*/
        for (auto pos: lp1) {
            table[pos]->followpos.insert(fp1);
        }
#if 0
        for (auto ptr: table) {
            std::vector<int> tlp;
            auto plp = ptr->lastpos();
            std::set_intersection(fp1.begin(), fp1.end(), plp.begin(), plp.end(), tlp.begin());
            if (tlp.size()) {
                ptr->followpos.insert(fp1.begin(), fp1.end());
            }
        }
#endif
    }

    virtual bool nullable()
    {
        return true;
    }

    virtual set<int>& firstpos_()
    {
        m_firstpos = child1->firstpos();
        return m_firstpos;
    }

    virtual set<int>& lastpos_()
    {
        m_lastpos = child1->firstpos();
        m_lastpos.insert(child1->lastpos());
        return m_lastpos;
        /*set<int> ret = child1->firstpos();*/
        /*set<int> tmp = child1->lastpos();*/
        /*ret.insert(tmp.begin(), tmp.end());*/
        /*return ret;*/
    }

    virtual std::ostream& output(std::ostream& os)
    {
        return _output(os, "STAR");
    }
};


template <typename TokenType>
static inline std::ostream& operator << (std::ostream& os, re_ast_node<TokenType>* a)
{
    return a->output(os);
}



template <typename TokenType>
struct DFA {
    struct state {
        int next[256];
        std::vector<TokenType> tokens;
        /*set<int> name;*/

        static const int no_transition = -1;

        state() : tokens()/*, name()*/ { ::memset(next, 0xFF, sizeof(int) * 256); }
        state(const set<int>& n) : tokens()/*, name(n)*/ { ::memset(next, 0xFF, sizeof(int) * 256); }

    };

    std::vector<state> states;

    static bool is_alnum(char c)
    {
        return (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9')
            || (c == '_');
    }

    std::unordered_map<TokenType, size_t>
        operator () (const char* _buf, size_t offset, size_t size)
        {
            const unsigned char* buf = reinterpret_cast<const unsigned char*>(_buf);
            const unsigned char* end = buf + size;
            const unsigned char* cur = buf + offset;
            std::unordered_map<TokenType, size_t> ret;
            int state = 0;
            while (cur <= end && states[state].next[(size_t)*cur] != -1) {
#if 0
                if (states[state].next['\b']) {
                    /* perl-ish hack : assert word boundary */
                    if (cur == buf
                            || (cur + 1) == end
                            || (is_alnum(*(cur - 1)) ^ is_alnum(*cur))) {
                        state = states[state].next['\b'];
                    } else {

                    }
                } else {
                }
#endif
                auto& S = states[state];
                /*std::cout << "state=" << state << " char=" << (*cur) << " tokens=" << S.tokens.size() << std::endl;*/
                if (S.tokens.size()) {
                    size_t ofs = cur - buf;
                    for (auto tok: S.tokens) {
                        ret[tok] = ofs;
                    }
                }
                state = S.next[(size_t)*cur++];
            }
            if (states[state].tokens.size()) {
                size_t ofs = cur - buf;
                for (auto tok: states[state].tokens) {
                    ret[tok] = ofs;
                }
            }
            return ret;
        }
};


template <typename TokenType>
std::ostream& operator << (std::ostream& os, const DFA<TokenType>& dfa)
{
    size_t n = dfa.states.size();
    size_t w = 1;
    while (n > 10) {
        ++w;
        n /= 10;
    }
    n = 0;
    os << std::setw(w + 1) << '|';
    for (int i = 0; i < MAX_CHAR; ++i) {
        os << std::setw(w) << (i < 32 ? '.' : ((char)i)) << ' ';
    }
    os << std::endl;
    for (auto& st: dfa.states) {
        os << std::setw(w) << (n++) << '|';
        for (int i = 0; i < MAX_CHAR; ++i) {
            if (st.next[i] != -1) {
                os << std::setw(w) << st.next[i] << '|';
            } else {
                os << std::setw(w + 1) << '|';
            }
        }
        /*os << st.name;*/
        if (st.tokens.size()) {
            os << " ACCEPT";
        }
        os << std::endl;
    }
    return os;
}


template <typename TokenType>
struct builder {
    DFA<TokenType> output;
    std::map<set<int>, int> state_map;
    re_ast_node_vector<TokenType> table;
    std::deque<set<int>> unmarked;

    set<int> firstpos(const set<int>& nodes)
    {
        set<int> ret;
        for (int i: nodes) {
            auto fp = table[i]->firstpos();
            ret.insert(fp);
        }
        return ret;
    }

    set<int> lastpos(const set<int>& nodes)
    {
        set<int> ret;
        for (int i: nodes) {
            auto fp = table[i]->lastpos();
            ret.insert(fp);
        }
        return ret;
    }

    set<int> followpos(const set<int>& nodes)
    {
        set<int> ret;
        for (int i: nodes) {
            ret.insert(table[i]->followpos);
        }
        return ret;
    }

    set<int> followpos(const set<int>& nodes, re_char_t symbol)
    {
        set<int> ret;
        for (int i: nodes) {
            for (int p: table[i]->followpos) {
                auto& ccls = table[p]->character_class;
                /*if (ccls.find(symbol) != ccls.end()) {*/
                if (ccls.find(symbol)) {
                    ret.insert(p);
                }
            }
        }
        return ret;
    }

    int new_state(const set<int>& name)
    {
        /*std::cout << "new_state " << name << std::endl;*/
        int& num = state_map[name];
        if (num == 0) {
            num = output.states.size();
            /*std::cout << "  created! " << num << std::endl;*/
            output.states.emplace_back();
            /*output.states.emplace_back(name);*/
            unmarked.push_back(name);
            /*for (int i: name) {*/
                /*if (table[i]->accept_token) {*/
                    /*output.states.back().tokens.push_back(table[i]->accept_token);*/
                /*}*/
            /*}*/
            /*set<int> follow = followpos(name);*/
            /*std::cout << "accepting tokens :" << std::endl;*/
            /*for (int i: follow) {*/
                /*if (table[i]->accept_token) {*/
                    /*output.states.back().tokens.push_back(table[i]->accept_token);*/
                    /*std::cout << table[i]->accept_token << std::endl;*/
                /*}*/
            /*}*/
        }
        return num;
    }

    set<int> filter_by_symbol(const set<int>& follow, re_char_t c)
    {
        set<int> ret;
        for (int i: follow) {
            auto& ccls = table[i]->character_class;
            /*if (ccls.find(c) != ccls.end()) {*/
            if (ccls.find(c)) {
                ret.insert(i);
            }
        }
        return ret;
    }

    character_class_t all_symbols(const set<int>& follow)
    {
        character_class_t ret;
        for (int i: follow) {
            ret.insert(table[i]->character_class);
        }
        /*std::cout << "follow symbols: name=" << follow << " symbols=" << ret << std::endl;*/
        return ret;
    }

    builder(re_ast_node<TokenType>* ast)
        : output()
        , state_map()
        , table()
    {
        if (!ast) {
            return;
        }
        /*std::cout << "AST@" << ((void*)ast) << std::endl;*/
        table.push_back(new initial_node<TokenType>());
        ast->reorder(table);
        /*ast->firstpos();*/
        /*ast->lastpos();*/
        ast->compute_followpos(table);
        output.states.reserve(table.size());
        for (auto ptr: table) {
            set<int> tmp;
            for (int f: ptr->followpos) {
                if (table[f]->nullable()) {
                    tmp.insert(table[f]->followpos);
                }
            }
            ptr->followpos.insert(tmp);
        }
        ast->propagate_followpos();
        /*std::cout << "AST = " << std::endl << ast << std::endl;*/

        table.front()->followpos = ast->firstpos();

        new_state({0});

        while (unmarked.size()) {
            /* while there is an unmarked state S */
            set<int> name = unmarked.front();
            /* mark S */
            unmarked.pop_front();
            int num = state_map[name];
            /*std::cout << "=============== state #" << num << std::endl;*/
            /*std::cout << "transitioning symbols " << all_symbols(name) << std::endl;*/
            set<int> follow = followpos(name);
            for (int i: follow) {
                if (table[i]->accept_token) {
                    output.states[num].tokens.push_back(table[i]->accept_token);
                    /*std::cout << table[i]->accept_token << std::endl;*/
                }
            }
            for (re_char_t symbol: all_symbols(follow)) {
                /*std::cout << "* on symbol " << symbol << std::endl;*/
                /*set<int> next_p = followpos(name, symbol);*/
                set<int> next_p = filter_by_symbol(follow, symbol);
                int next_num = new_state(next_p);
                output.states[num].next[symbol] = next_num;
            }
            /*std::cout << "============================" << std::endl;*/
        }
    }
};

namespace re_parser {
template <typename TokenType>
    re_ast_node<TokenType>* _cat(re_ast_node<TokenType>* a, re_ast_node<TokenType>* b)
    {
        if (a) {
            return new cat_node<TokenType>(a, b);
        } else {
            return b;
        }
    }

template <typename TokenType>
    re_ast_node<TokenType>* _or(re_ast_node<TokenType>* a, re_ast_node<TokenType>* b)
    {
        if (!(a && b)) {
            /* error! */
        }
        return new or_node<TokenType>(a, b);
    }

template <typename TokenType>
    re_ast_node<TokenType>* _star(re_ast_node<TokenType>* a)
    {
        if (!a) {
            /* error! */
        }
        return new star_node<TokenType>(a);
    }

template <typename TokenType>
    re_ast_node<TokenType>* pop(re_ast_node_vector<TokenType>& stack)
    {
        re_ast_node<TokenType>* ret = stack.back();
        stack.pop_back();
        return ret;
    }

template <typename TokenType>
    void _cat(re_ast_node_vector<TokenType>& stack)
    {
        re_ast_node<TokenType>* b = pop<TokenType>(stack);
        re_ast_node<TokenType>* a = pop<TokenType>(stack);
        stack.push_back(_cat(a, b));
    }

template <typename TokenType>
    void _cat_reduce(re_ast_node_vector<TokenType>& stack)
    {
        while (stack.size() >= 2) {
            _cat<TokenType>(stack);
        }
    }

template <typename TokenType>
    void _or(re_ast_node_vector<TokenType>& stack)
    {
        re_ast_node<TokenType>* b = pop<TokenType>(stack);
        re_ast_node<TokenType>* a = pop<TokenType>(stack);
        stack.push_back(_or(a, b));
    }

template <typename TokenType>
    void _or_reduce(re_ast_node_vector<TokenType>& stack)
    {
        while (stack.size() >= 2) {
            _or<TokenType>(stack);
        }
    }

template <typename TokenType>
    void _star(re_ast_node_vector<TokenType>& stack)
    {
        re_ast_node<TokenType>* a = _star<TokenType>(pop<TokenType>(stack));
        stack.push_back(a);
    }

template <typename TokenType>
    std::pair<re_ast_node<TokenType>*, const char*>
    parse_rec(const char* ptr, char delim, re_ast_node<TokenType>* start = NULL)
    {
        re_ast_node_vector<TokenType> stack;
        std::pair<re_ast_node<TokenType>*, const char*> ret;
        /*re_ast_node* ast = start;*/
        stack.push_back(start);
        re_ast_node<TokenType>* tmp;
        bool neg;
        while (*ptr != delim) {
            switch (*ptr) {
                case '\\':
                    ++ptr;
                    tmp = new re_ast_node<TokenType>();
                    switch(*ptr) {
                        case '*':
                        case '?':
                        case '+':
                        case '[':
                        case '(':
                        case ']':
                        case ')':
                        case '|':
                        case '.':
                            tmp->character_class.insert(*ptr);
                            break;
                        case 'b':
                            tmp->character_class.insert('\b');
                            break;
                        case 't':
                            tmp->character_class.insert('\t');
                            break;
                        case 'n':
                            tmp->character_class.insert('\t');
                            break;
                        case 'r':
                            tmp->character_class.insert('\r');
                            break;
                        default:
                            /* error! */
                            ;
                    };
                    /*ast = _cat(ast, tmp);*/
                    stack.push_back(tmp);
                    ++ptr;
                    break;
                case '*':
                    _star<TokenType>(stack);
                    ++ptr;
                    break;
                case '?':
                    stack.push_back(new re_ast_node<TokenType>());
                    _or<TokenType>(stack);
                    ++ptr;
                    break;
                case '+':
                    stack.push_back(stack.back());
                    _star<TokenType>(stack);
                    _cat<TokenType>(stack);
                    ++ptr;
                    break;
                case '|':
                    ++ptr;
                    ret = parse_rec<TokenType>(ptr, delim);
                    ptr = ret.second;
                    _cat_reduce<TokenType>(stack);
                    stack.push_back(ret.first);
                    _or<TokenType>(stack);
                    break;
                case '[':
                    tmp = new re_ast_node<TokenType>();
                    ++ptr;
                    if (*ptr == '^') {
                        tmp->character_class.set_all();
                        neg = true;
                        ++ptr;
                    } else {
                        tmp->character_class.reset_all();
                        neg = false;
                    }
                    while (*ptr && *ptr != ']') {
                        if (*ptr == '\\') {
                            ++ptr;
                            switch (*ptr) {
                                case '\\':
                                case ']':
                                    break;
                                case '^':
                                    if (!neg) {
                                        break;
                                    }
                                default:
                                    /* error! */
                                    ;
                            };
                        }
                        if (*ptr == '-') {
                            if (*(ptr + 1) == ']') {
                                tmp->character_class.toggle(*ptr);
                            } else {
                                char a = *(ptr - 1) + 1;
                                ++ptr;
                                char b = *ptr;
                                if (neg) {
                                    for (; a <= b; ++a) {
                                        tmp->character_class.erase(a);
                                    }
                                } else {
                                    for (; a <= b; ++a) {
                                        tmp->character_class.insert(a);
                                    }
                                }
                            }
                        } else {
                            tmp->character_class.insert(*ptr);
                        }
                        ++ptr;
                    };
                    if (!*ptr) {
                        /* error! */
                    }
                    stack.push_back(tmp);
                    ++ptr;
                    break;
                case '(':
                    ret = parse_rec<TokenType>(ptr + 1, ')');
                    ptr = ret.second;
                    ret.first->grouping = true;
                    /*ast = _cat(ast, ret.first);*/
                    stack.push_back(ret.first);
                    ++ptr;
                    break;
                case '.':
                    tmp = new re_ast_node<TokenType>();
                    for (re_char_t i = 0; i <= MAX_CHAR; ++i) {
                        tmp->character_class.insert(i);
                    }
                    stack.push_back(tmp);
                    ++ptr;
                    break;
                default:
                    tmp = new re_ast_node<TokenType>();
                    tmp->character_class.insert(*ptr);
                    /*ast = _cat(ast, tmp);*/
                    stack.push_back(tmp);
                    ++ptr;
            };
        }
        while (stack.size() >= 2) {
            re_ast_node<TokenType>* c2 = stack.back();
            stack.pop_back();
            re_ast_node<TokenType>* c1 = stack.back();
            stack.pop_back();
            stack.push_back(_cat<TokenType>(c1, c2));
        }
        return std::pair<re_ast_node<TokenType>*, const char*>(stack.size() ? stack.front() : NULL, ptr);
    }

template <typename TokenType>
    re_ast_node<TokenType>* parse(const char* pattern, TokenType token)
    {
        const char* ptr = pattern;
        /*std::pair<re_ast_node*, const char*> ret = parse_rec(ptr, 0, new initial_node());*/
        std::pair<re_ast_node<TokenType>*, const char*> ret = parse_rec<TokenType>(ptr, 0);
        if (*ret.second) {
            /* error! didn't parse everything */
        }
        re_ast_node<TokenType>* final = new final_node<TokenType>(token);
        return _cat<TokenType>(ret.first, final);
    }
}

struct translation {
    static void re_escape(std::stringstream& s, char c)
    {
        switch(c) {
            case '[':
            case ']':
            case '(':
            case ')':
            case '+':
            case '*':
            case '?':
            case '.':
                s << '\\';
            default:
                s << c;
        };
    }

    static void re_escape(std::stringstream& s, const std::string& str)
    {
        for (char c: str) {
            re_escape(s, c);
        }
    }

    static std::string re_escape(const std::string& str)
    {
        std::stringstream s;
        re_escape(s, str);
        return s.str();
    }

    static void re_class(std::stringstream& s, char c, bool positive)
    {
        s << '[';
        if (!positive) {
            s << '^';
        }
        switch (c) {
            case '^':
                if (positive) {
                    s << "\\^";
                    break;
                }
            case ']':
            case '\\':
                s << '\\';
            default:
                s << c;
        };
        s << ']';
    }

    static std::string no_match(const std::string& str)
    {
        std::stringstream s;
        s << '(';
        for (size_t i = 0; i < str.size(); ++i) {
            for (size_t k = 0; k < i; ++k) {
                re_escape(s, str[k]);
            }
            re_class(s, str[i], false);
            s << '|';
        }
        s << "\\\\";
        for (size_t k = 0; k < str.size(); ++k) {
            re_escape(s, str[k]);
        }
        s << ')';
        return s.str();
    }

    static std::string str_to_pattern(const std::string& begin, const std::string& end)
    {
        std::stringstream s;
        re_escape(s, begin);
        s << no_match(end) << '*';
        re_escape(s, end);
        return s.str();
    }
};


#if 0
struct token_to_re_ast : grammar::visitors::dummy_filter<re_ast_node*> {
    virtual re_ast_node* eval(item::token::Str* x)
    {
        return re_parser::parse(translation::str_to_pattern(x->delim_start, x->delim_end));
    }
    virtual re_ast_node* eval(item::token::Re* x)
    {
        return re_parser::parse(x->pattern());
    }
    virtual re_ast_node* eval(item::token::Epsilon*) { return new re_ast_node(); }
    virtual re_ast_node* eval(item::token::Eof*) { return new re_ast_node(); /* FIXME!! */ }
    virtual re_ast_node* eval(item::token::T* x)
    {
        return re_parser::parse(translation::re_escape(x->str());
    }
    virtual re_ast_node* eval(item::token::AddToBag* x) { return re_parser::parse(x->pattern); }
    virtual re_ast_node* eval(item::token::Bow* x)
    {
        std::stringstream s;
        std::function<void(const char*)> lambd = [&] (const char* x)
        {
            if (s.str().size()) { s << '|'; }
            translation::re_escape(s, x);
        };

        trie_enumerate(*x, lambd);
        
        return re_parser::parse(s.str().c_str(), s.str().size());
        /*return 0;*/
    }
};
#endif


template <typename TokenType>
struct lexer {
    std::unordered_map<TokenType, re_ast_node<TokenType>*> tokens;
    DFA<TokenType> dfa;

    std::unordered_map<TokenType, size_t> match(const char* buffer, size_t size)
    {
    }

    void compile()
    {
        auto i = tokens.begin(), j = tokens.end();
        re_ast_node<TokenType>* ast = i->second;
        for (++i; i != j; ++i) {
            ast = new or_node<TokenType>(ast, i->second);
        }
        dfa = builder<TokenType>(ast).output;
#if 0
        re_ast_node_vector<TokenType> stack(tokens.size());
        for (auto& kv: tokens) {
            stack.push_back(kv.second);
            std::cout << ((void*)stack.back()) << std::endl;
        }
        re_parser::_or_reduce<TokenType>(stack);
        std::cout << ((void*)stack.back()) << std::endl;
        dfa = builder<TokenType>(stack.front()).output;
#endif
    }

    void add_token(TokenType t, re_ast_node<TokenType>* ast)
    {
        tokens[t] = ast;
    }

    std::unordered_map<TokenType, size_t>
        operator () (const char* buf, size_t offset, size_t size)
        {
            return dfa(buf, offset, size);
        }
};



#endif

