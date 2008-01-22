/*! \page Tutorial First steps with Tinyap
 * This page explains how to fetch, install, run, and use tinyap.
 * \section t_toc  Contents
 * <div style="background-color:#E8E8E8; border:solid 1px #808080; width:300px; padding:6px;">
 *	\ref t_intr <br/>
 *	\ref t_inst <br/>
 *	\ref t_gram <br/>
 *	\ref t_usag <br/>
 *	\ref t_srlz <br/>
 *	\ref t_walk <br/>
 *	\ref t_api <br/>
 * </div>
 * \endsection
 * 
 * \section t_intr Introduction
 * <div align="center" style="margin:16px;"><em>&ldquo;One build to parse them all.&rdquo;</em></div>
 *
 * Basically, tinyap is a <a href="http://en.wikipedia.org/wiki/Recursive_descent_parser">recursive descent parser with backup</a>. Thus, it's able to recognize any LL(k) language.
 * When a parse is successful, tinyap outputs an <a href="http://en.wikipedia.org/wiki/Abstract_syntax_tree">Abstract Syntax Tree (AST)</a> that represents the input text in a structured manner.
 * 
 * Unlike most parsers, its code isn't factory-calibrated for a particular grammar. Instead, the grammar is data. Tinyap uses an
 * AST that represents a grammar to parse its input text. This grammar AST is structured according to \ref t_gram "tinyap's grammar description language".
 *
 * The factory default for the grammar AST is the \ref t_gram "grammar description language" itself. So one can parse a grammar description and
 * directly use the parse output to parse some other text written in the described language. Tinyap also features a plugin mechanism for grammars,
 * which allows for dynamic modular grammars. The ASTs tinyap produces can be serialized to and deserialized from buffers and files.
 *
 * Finally, tinyap provides an interface to walk down the ASTs and makes it easy to write external plugins to visit the nodes,
 * e.g. a compiler, an interpreter, a static evaluator, a renderer...
 *
 * You can have a look at an extensive use of tinyap at http://code.google.com/p/tinyaml where tinyap is used to parse compile and extend a base meta-language.
 *
 * \endsection
 *
 * \section t_inst I. Installation
 * <p>First, get the source code.</p>
 * <ul>
 * <li><b>download a tarball</b> :
 * (version 1.1 in used the example below, you can download the latest at http://code.google.com/p/tinyap/downloads/list).
 * \code
 * $ wget http://tinyap.googlecode.com/files/tinyap-1.1.tar.gz
 * $ tar xzf tinyap-1.1.tag.gz
 * \endcode
 * <li><b>using SVN</b> :
 * \code
 * $ svn checkout http://tinyap.googlecode.com/svn/trunk/ tinyap-read-only
 * \endcode
 * </ul>
 * <p>Now you have your tinyaml source distribution at hands, build it.</p>
 * \code
 * $ cd tinyap
 * $ CFLAGS=-O3 ./configure -C --prefix=/my/install/prefix/if/not/slash/usr
 * $ make all
 * $ make install
 * \endcode
 * You might need root privileges to run make install.
 * \li 
 *
 * \endsection
 *
 * \section t_gram II. Meta-Grammar
 * <div align="center" style="margin:16px;"><em>&ldquo;The language is that of BNF, which I will not utter here. In the common tongue it says...&rdquo;</em></div>
 *
 * The way the parser produces its output is driven by the grammar it uses. A grammar is a set of production rules, each rule associates
 * a symbolic name and one or more elements that the rule will produce. Each element successfully parsed is converted in an AST node and the
 * full AST is built after the recursive descent. When a rule successfully produces its elements, it can evaluate to a new operator node and
 * enclose its produced nodes (these rules are called operator rules), or evaluate to its produced nodes silently (these rules are called
 * transient rules). Transient rules are useful to implement loops and split rules.
 *
 * The parser distinguishes information from garbage. A special rule defines the garbage characters to strip from the input between two elements.
 * Terminal strings don't contain information and are not included in the output.
 *
 * Elements
 *
 * - \c \<non-terminal\> : the parser will try to produce the rule named "non-terminal", and append the result to output.
 * - \c "terminal" : the parser will try to produce the string "terminal" at its current position, and discard it from output.
 * - \c /reg-exp/ : the parser will search for a match at its current position, and append the match string to output.
 *
 * Expressions
 * - Sequence \code expression expression... \endcode produce all children from left to right
 * - Selection \code ( expression | expression | ... ) \endcode try all children from left to right until successful
 * - Prefix \code [ expression ] <non-terminal> \endcode produce expression, produce non-terminal, insert expression at head of non-terminal's children, evaluate to non-terminal
 * - Postfix (much useless) \code { expression } <non-terminal> \endcode same as prefix, but insert expression at tail of non-terminal's children
 *
 * Rules
 * - Operator rules \code symbol ::= expression . \endcode produce an operator node labeled "symbol"
 * - Transient rules \code symbol = expression . \endcode produce expression
 * - _start : tinyap searches for this rule to start a parse.
 * - _whitespace : tinyap searches for a string or a regexp in the right-hand side expression, and uses it as the garbage characters pattern.
 *
 *
 * Special symbols
 * - \c epsilon : produce the empty string (never fails)
 * - \c EOF : produce the end of file
 *
 * Special constructs
 *
 * - Handling trivial left-recursive rules \code lefty = ( <lefty> <A> | <B> ). \endcode (works for both operator and transient rules) tinyap recognizes such rules and handles them correctly. Use with care. Remember it's an LL parser, though. Tinyap doesn't handle ALL left-recursive rules. You're warned.
 * - Grammar plugins \code plugin_rule = ( <A> | <B> | ... ). \endcode tinyap can plug new alternatives in rules where the right-hand side expression is a selection (see \ref t_api).
 *
 * Here is the full grammar description language expressed with itself : \include ".explicit.grammar"
 *
 * Below is a condensed grammar to parse arithmetic expressions. It will be used in the following examples : \include "examples/math.gram"
 * Notice that number, m_expr, m_mul, m_sub, m_add, m_div are operator rules. These are the labels we'll have in the nodes of the parse results.
 *
 * \endsection
 *
 * \section t_usag III. Usage
 * <div align="center" style="margin:16px;"><em>&ldquo;First shalt thou take out the Holy Command Line, then shalt thou count to three, no more, no less.<br/>Three shall be the number thou shalt count, and the number of the counting shall be three.&rdquo;</em></div>
 *
 * When invoked, tinyap executes each of its command-line arguments from left to right.
 *
 * - \code --help, -h [name] \endcode Display version and usage.
 * - \code --grammar, -g [file name] \endcode Set the grammar to be used. If the name is \c explicit or \c CamelCasing, the according grammar dialect is used. Otherwise, the named file must contain a serialized "Grammar" AST.
 * - \code --pring-grammar, -pg \endcode Output the current grammar to stdout, using explicit dialect.
 * - \code --input, -i [file name] \endcode Set the file to be parsed. \c - sets input to stdin.
 * - \code --output, -o [file name] \endcode Output serialized AST to named file. \c - sets output to stdout.
 * - \code --parse, -p \endcode Parse the input file using the current grammar.
 * - \code --parse-as-grammar, -pag \endcode Parse the input file using the current grammar and set grammar to the newly output AST if parse succeeded. Parser output is NULL after this.
 * - \code --walk, -w [name] \endcode Walk over the current parser output with the named walker.
 *
 * Examples : (assuming you're in ./examples/)
 *
 * - first use : \code $ tinyap -h \endcode
 * - display the default meta-grammar : \code $ tinyap -pg \endcode
 * - parse a grammar : \code $ tinyap -i math.gram -pag \endcode
 * - parse something and display the result tree : \code $ tinyap -i math.gram -p -w prettyprint \endcode
 * - parse a grammar and a text file : \code $ tinyap -i math.gram -pag -i test.math -p \endcode
 * - evaluate the operations in test2.math : \verbatim $ gcc -shared ape_tinycalc.o -o libape_tinycalc.so # you may need to build the shared object
$ LD_LIBRARY_PATH=. tinyap -i math.gram -pag -i test2.math -p -w tinycalc \endverbatim
 *
 * \endsection
 *
 * \section t_srlz IV. (Un)Serialization of ASTs
 * Tinyap makes it easy writing and reading ASTs to and from files and buffers. Serialized ASTs can be directly used as grammars :
 * \verbatim
 $ tinyap -i math.gram -p -o math.gram.srlz                 # parse and serialize math.gram
 $ tinyap -g math.gram.srlz -i test2.math -p -w tinycalc    # using math.gram, parse test2.math and evaluate the operations \endverbatim
 *
 * The \ref t_api "C API" provides more means to serialize and unserialize ASTs to/from files and buffers. Such interactions are pointless in command line.
 *
 * \endsection
 *
 * \section t_walk V. These apes are made for walking...
 * <div align="center" style="margin:16px;"><em>&ldquo;...And that's just what they'll do : one of these days these apes are gonna walk 'ver the output.&rdquo;</em></div>
 * Tinyap provides an interface to easily write AST evaluators (thus the name, tinyap evaluator -> tinyape -> ape).
 *
 * An evaluator has to provide four functions :
 * - init : receives data under the form of a \c void* and has to initialize the ape.
 * - term : terminate the ape.
 * - result : has to return the result of the ape's evaluation.
 * - default : this visit method is called whenever no specific visit method has been defined for an operator node.
 *
 * Visit methods take the current node as an argument and return the direction for the next step in walk. Directions can be :
 * - Up : to parent node
 * - Down : walk on each child node (operands)
 * - Next : walk to next sibling node (or is equivalent to "Up-Next" on the last child of a node)
 * - Done : evaluation is over and successful.
 * - Error : evaluation failed.
 *
 * Each method of an evaluator must be named as follows :
 * \code ape_<em>evaluator_name</em>_<em>method</em> \endcode
 * <em>method</em> is \c init, \c term, \c result, \c default, or an operator label.
 *
 * Tinyap handles the actual walking depending on evaluator's direction returns, and invokes the proper evaluator's visit method at each step.
 *
 * Here is the code for the pretty-printer (invoke it in command line with "-w prettyprint") : \include "src/ape_prettyprint.c"
 *
 * And here is the code for the tiny calculator "tinycalc" (seen in the examples above) : \include "examples/ape_tinycalc.c"
 *
 * \endsection
 *
 * \section t_api VI. C API
 * 
 * \see tinyap.h
 * \see tinyape.h
 *
 * \endsection
 *
 * \endpage
 */

