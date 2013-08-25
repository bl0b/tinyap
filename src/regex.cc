#include "regex.h"

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "usage: " << argv[0] << " [pattern...] [text]" << std::endl;
    }
    --argc;
    re_ast_node* ast = NULL;
    for (int i = 1; i < argc; ++i) {
        re_ast_node* tmp = re_parser::parse(argv[i], reinterpret_cast<grammar::item::base*>(i));
        if (!ast) {
            ast = tmp;
        } else {
            ast = new or_node(ast, tmp);
        }
    }
    DFA dfa = builder(ast).output;
    std::cout << dfa;
    std::string text(argv[argc]);
    for (auto tok_ofs: dfa(text.c_str(), 0, text.size())) {
        std::cout << "recognized @" << ((void*)tok_ofs.first) << " ofs=" << tok_ofs.second << std::endl;
    }
    std::cout << "not 'end' => " << translation::no_match("end") << std::endl;
    std::cout << "~([,])~ => " << translation::str_to_pattern("([", "])") << std::endl;
    return 0;
}

