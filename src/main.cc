/* Tinya(J)P : this is not yet another (Java) parser.
 * Copyright (C) 2007 Damien Leroux
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "config.h"
#include "tinyap.h"
#include "ast.h"
#include <fstream>

extern "C" {
#include "tinyape.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>


#define TINYAP_ABOUT	"This is not yet another parser.\n" \
			"(c) 2007-2010 Damien 'bl0b' Leroux\n\n"

void ast_serialize(const ast_node_t ast,char**output);

ast_node_t grammar,ast;

FILE*inputFile,*outputFile;
const char*grammarSource="explicit";	/* default : explicit NFGDG */
int print_grammar=0;
const char*textSource="-";		/* default : input from stdin */
const char*outputDest="-";		/* default : output to stdout */

#define cmp_param(_n,_arg_str_long,_arg_str_short) (i<(argc-_n) && (	\
		(_arg_str_long && !strcmp(_arg_str_long,argv[i]))	\
					||				\
		(_arg_str_short && !strcmp(_arg_str_short,argv[i]))	\
	))

void ast_serialize_to_file(const ast_node_t ast,FILE*f);

void tinyap_set_output(const tinyap_t t, ast_node_t o);

void node_pool_flush();

extern int max_rec_level;
extern int tinyap_verbose;



/*ast_node_t relations_from_tree(ast_node_t gram);*/

int do_args(int argc,char*argv[]) {
	int i;
	tinyap_t parser;

	tinyap_init();

	parser = tinyap_new();

	for(i=1;i<argc;i+=1) {
		if(cmp_param(1,"--grammar","-g")) {
			i+=1;
			tinyap_set_grammar(parser,argv[i]);
		} else if(cmp_param(1,"--input","-i")) {
			i+=1;
			tinyap_set_source_file(parser,argv[i]);
		} else if(cmp_param(1,"--dump-stack","-ds")) {
			i+=1;
			tinyap_dump_stack(parser, argv[i]);
		} else if(cmp_param(0,"--print-states","-ps")) {
			i+=1;
			tinyap_print_states(parser);
		} else if(cmp_param(1,"--output","-o")) {
			i+=1;
			if(tinyap_parsed_ok(parser)&&tinyap_get_output(parser)) {
				tinyap_serialize_to_file(tinyap_get_output(parser),argv[i]);
			/*} else {*/
				/*fprintf(stderr,"parse error at line %i, column %i\n%s\n",tinyap_get_error_row(parser),tinyap_get_error_col(parser),tinyap_get_error(parser));*/
			}
		} else if(cmp_param(2,"--wordlist","-wl")) {
			trie_t bow;
			i+=1;
			const char* tag = argv[i];
			bow = tinyap_get_bow(regstr(tag));
			i+=1;
			std::ifstream wl(argv[i]);
			while(!wl.eof()) {
				std::string word;
				wl >> word;
				if(word.size()) {
					trie_insert(bow, word.c_str());
					std::clog << '~' << tag << '~' << ' ' << word << std::endl;
				}
			}
		} else if(cmp_param(0,"--parse","-p")) {
			tinyap_parse(parser, false);
			if(tinyap_parsed_ok(parser)&&tinyap_get_output(parser)) {
				/*tinyap_serialize_to_file(tinyap_get_output(parser),argv[i]);*/
				if(tinyap_verbose) {
					fprintf(stderr, "parsed %u bytes in %.3f seconds (%.3f kBps)\n",
							tinyap_get_source_buffer_length(parser),
							tinyap_get_parse_time(parser),
							tinyap_get_source_buffer_length(parser)/tinyap_get_parse_time(parser)*(1./1024));
				}
			} else {
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",tinyap_get_error_row(parser),tinyap_get_error_col(parser),tinyap_get_error(parser));
				/*fprintf(stderr,"parse error at line %i, column %i\n%s\n", -1, -1, "TODO");*/
			}
		} else if(cmp_param(0,"--full-parse","-fp")) {
			tinyap_parse(parser, true);
			if(tinyap_parsed_ok(parser)&&tinyap_get_output(parser)) {
				/*tinyap_serialize_to_file(tinyap_get_output(parser),argv[i]);*/
				if(tinyap_verbose) {
					fprintf(stderr, "parsed %u bytes in %.3f seconds (%.3f kBps)\n",
							tinyap_get_source_buffer_length(parser),
							tinyap_get_parse_time(parser),
							tinyap_get_source_buffer_length(parser)/tinyap_get_parse_time(parser)*(1./1024));
				}
			} else {
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",tinyap_get_error_row(parser),tinyap_get_error_col(parser),tinyap_get_error(parser));
				/*fprintf(stderr,"parse error at line %i, column %i\n%s\n", -1, -1, "TODO");*/
			}
		} else if(cmp_param(0,"--parse-as-grammar","-pag")) {
			tinyap_parse_as_grammar(parser);
			if(!(tinyap_parsed_ok(parser)&&tinyap_get_grammar_ast(parser))) {
				/*tinyap_serialize_to_file(tinyap_get_output(parser),argv[i]);*/
			/*} else {*/
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",tinyap_get_error_row(parser),tinyap_get_error_col(parser),tinyap_get_error(parser));
				/*fprintf(stderr,"parse error at line %i, column %i\n%s\n", -1, -1, "TODO");*/
			}
		} else if(cmp_param(0,"--append-to-grammar","-atg")) {
            tinyap_append_grammar(parser, Car(tinyap_get_output(parser)));
		} else if(cmp_param(0,"--print-grammar","-pg")) {
			/*print_rules(tinyap_get_grammar_ast(parser));*/
			/*fputc('\n',stdout);*/

			wast_t grammar, short_gram;
			const char* up;

			grammar = make_wast(parser, tinyap_list_get_element(tinyap_get_grammar_ast(parser), 0));
			Ast short_gram_ast = tinyap_get_ruleset(GRAMMAR_SHORT);
			short_gram = make_wast(parser, tinyap_list_get_element(short_gram_ast, 0));
			up = tinyap_unparse(short_gram, grammar);
			if(up) {
				fputs(up, stdout);
			} else {
				fputs("Couldn't unparse the AST ! :(\n", stderr);
			}
			free((char*)up);
			wa_del(grammar);
			wa_del(short_gram);
		} else if(cmp_param(0,"--verbose","-V")) {
			tinyap_set_verbose(1);
		} else if(cmp_param(0,"--quiet","-q")) {
			tinyap_set_verbose(0);
		} else if(cmp_param(0,"--version","-v")) {
			fprintf(stderr, TINYAP_ABOUT);
			fprintf(stderr, "version " TINYAP_VERSION "\n" );
		} else if(cmp_param(0,"--help","-h")) {

			fprintf(stderr, TINYAP_ABOUT);
			fprintf(stderr, "Usage : %s [--input,-i [inputFile]] [--output,-o [outputFile]] [--grammar,-g [grammarFile]] [--parse,-p] [--parse-as-grammar,-pag] [--walk, -w [pilotName]] [--help,-h] [--version, -v] [--verbose,-V] [--quiet,-q]\n",argv[0]);
			fprintf(stderr, "\n\t--version,-v\tdisplay version\n");
			fprintf(stderr, "\n\t--verbose,-V\toutput messages during parse\n");
			fprintf(stderr, "\n\t--quiet,-q\tdon't output messages during parse (default)\n");
			fprintf(stderr, "\n\t--grammar,-g name\tuse this grammar to parse input\n");
			fprintf(stderr, "\t\t\"" GRAMMAR_SHORT "\"\t(default) selects default meta-grammar\n");
			fprintf(stderr, "\t\tany other string is a filename to read grammar from\n");
			fprintf(stderr, "\n\t--print-grammar,-pg\toutput the current grammar in `explicit' dialect\n");
			fprintf(stderr, "\t\targument is the same as above\n");
			fprintf(stderr, "\n\t--input,-i name \ttext source to use\n");
			fprintf(stderr, "\t\t- (default)\tselects standard input\n");
			fprintf(stderr, "\t\tany other string is a filename to read from\n");
			fprintf(stderr, "\n\t--output,-o name\tredirect serialized AST output\n");
			fprintf(stderr, "\t\t- (default)\tselects standard output\n");
			fprintf(stderr, "\t\tany other string is a filename to write to\n");
			fprintf(stderr, "\n\t--parse,-p\t\tparse input text\n");
			fprintf(stderr, "\n\t--parse-as-grammar,-pag\tparse input text and use output AST as new grammar\n");
			fprintf(stderr, "\n\t--append-to-grammar,-atg\tparse input text and append result to the grammar\n");
			fprintf(stderr, "\n\t--full-parse,-fp\t\tfind all possible parse trees\n");
			fprintf(stderr, "\n\t--parse,-p\t\tparse input text favoring shift over reduce\n");
			fprintf(stderr, "\n\t--dump-stack,-ds [dotFile]\tdump the LR stack as a .dot file\n");
			fprintf(stderr, "\n\t--print-states,-ps\t\tprint the LR(0) states to standard output\n");
			fprintf(stderr, "\n\t--walk,-w name\t\twalk the current output tree using named ape\n\t\t\t\t(try prettyprint !)\n");
			fprintf(stderr, "\n\t--help,-h\t\tdisplay this text\n\n");
			exit(0);
		} else if(cmp_param(1,"--walk","-w")) {
			i+=1;
			ast_node_t one = tinyap_get_output(parser);
			while(one) {
				wast_t wa = tinyap_make_wast(parser, Car(one));
				tinyap_walk(wa,argv[i],NULL);
				tinyap_free_wast(wa);
				one = Cdr(one);
			}
		}
	}

	tinyap_delete(parser);

	/*if(tinyap_verbose) {*/
		/*fprintf(stderr, "maximum recursion level : %i\n", max_rec_level);*/
	/*}*/
	return 0;
}



extern volatile int _node_alloc_count;

int main(int argc, char**argv) {
	return do_args(argc,argv);
}

}

