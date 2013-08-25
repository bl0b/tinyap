#include "regex.h"

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "usage: " << argv[0] << " [pattern...] [text]" << std::endl;
    }
    --argc;
#if 0
    re_ast_node<int>* ast = NULL;
    for (int i = 1; i < argc; ++i) {
        re_ast_node<int>* tmp = re_parser::parse(argv[i], i);
        if (!ast) {
            ast = tmp;
        } else {
            ast = new or_node<int>(ast, tmp);
        }
    }
    DFA<int> dfa = builder<int>(ast).output;
    std::cout << dfa;
    std::string text(argv[argc]);
    for (auto tok_ofs: dfa(text.c_str(), 0, text.size())) {
        std::cout << "recognized @" << ((void*)tok_ofs.first) << " ofs=" << tok_ofs.second << std::endl;
    }
#else
    lexer<int> lex;
    for (int i = 1; i < argc; ++i) {
        lex.add_token(i, re_parser::parse(argv[i], i));
    }
    lex.compile();
    /*std::cout << lex.dfa;*/
    std::string text(argv[argc]);
    for (auto tok_ofs: lex(text.c_str(), 0, text.size())) {
        std::cout << "recognized @" << tok_ofs.first << " ofs=" << tok_ofs.second << std::endl;
    }
#endif
    std::cout << "not 'end' => " << translation::no_match("end") << std::endl;
    std::cout << "~([,])~ => " << translation::str_to_pattern("([", "])") << std::endl;
    return 0;
}

