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
#include "tinyap.h"
//#include "bootstrap.h"
//#include "tokenizer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>


#define TINYAP_ABOUT	"This is not yet another parser.\n" \
			"(c) 2007 Damien 'bl0b' Leroux\n\n"


void ast_serialize(const ast_node_t ast,char**output);
void print_rules(ast_node_t rs);

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


void node_pool_flush();


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
		} else if(cmp_param(1,"--output","-o")) {
			i+=1;
			if(tinyap_parsed_ok(parser)) {
				tinyap_serialize_to_file(tinyap_get_output(parser),argv[i]);
			} else {
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",tinyap_get_error_row(parser),tinyap_get_error_col(parser),tinyap_get_error(parser));
			}
		} else if(cmp_param(1,"--parse","-p")) {
			tinyap_parse(parser);
			if(tinyap_parsed_ok(parser)) {
				tinyap_serialize_to_file(tinyap_get_output(parser),argv[i]);
			} else {
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",tinyap_get_error_row(parser),tinyap_get_error_col(parser),tinyap_get_error(parser));
			}
		} else if(cmp_param(1,"--parse-as-grammar","-pag")) {
			tinyap_parse_as_grammar(parser);
			if(tinyap_parsed_ok(parser)) {
				tinyap_serialize_to_file(tinyap_get_output(parser),argv[i]);
			} else {
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",tinyap_get_error_row(parser),tinyap_get_error_col(parser),tinyap_get_error(parser));
			}
		} else if(cmp_param(0,"--print-grammar","-pg")) {
			print_rules(tinyap_get_grammar_ast(parser));
			fputc('\n',stdout);
		} else if(cmp_param(0,"--version","-v")) {
			printf(TINYAP_ABOUT);
			printf("version " TINYAP_VERSION "\n" );
		} else if(cmp_param(0,"--help","-h")) {
			printf(TINYAP_ABOUT);
			printf("Usage : %s [--input,-i [inputFile]] [--output,-o [outputFile]] [--grammar,-g [grammarFile]] [--parse,-p] [--parse-as-grammar,-pag] [--walk, -w [pilotName]] [--help,-h]\n",argv[0]);
			printf("\n\t--grammar,-g name\tuse this grammar to parse input\n");
			printf("\t\t\"" GRAMMAR_EXPLICIT "\"\t(default) selects explicit variant\n");
			printf("\t\t\"" GRAMMAR_CAMELCASING "\"\tselects CamelCasing variant\n");
			printf("\t\tany other string is a filename to read grammar from\n");
			printf("\n\t--print-grammar,-pg name\toutput the grammar in explicit dialect of Blob's Noise-Filtering Form\n");
			printf("\t\targument is the same as above\n");
			printf("\n\t--input,-i name \ttext source to use\n");
			printf("\t\t- (default)\tselects standard input\n");
			printf("\t\tany other string is a filename to read from\n");
			printf("\n\t--output,-o name\tredirect serialized AST output\n");
			printf("\t\t- (default)\tselects standard output\n");
			printf("\t\tany other string is a filename to write to\n");
			printf("\n\t--parse,-p\t\tparse input text\n");
			printf("\n\t--parse-as-grammar,-pag\tparse input text and use output AST as new grammar\n");
			printf("\n\t--walk,-w name\t\twalk the current output tree using named ape\n\t\t\t\t(try prettyprint !)\n");
			printf("\n\t--help,-h\t\tdisplay this text\n\n");
			exit(0);
		} else if(cmp_param(1,"--walk","-w")) {
			i+=1;
			wast_t wa = tinyap_make_wast(tinyap_list_get_element(tinyap_get_output(parser),0));
			tinyap_walk(wa,argv[i],NULL);
			tinyap_free_wast(wa);
		}
	}

	tinyap_delete(parser);

	return 0;
}



extern volatile int _node_alloc_count;

int main(int argc, char**argv) {
	return do_args(argc,argv);
}




void print_rule_elem(ast_node_t e) {
	const char*tag;
	//ast_node_t t;

	if(!e) {
		return;
	}

	if(tinyap_node_is_string(e)) {
		printf("[OUPS L'ATOME %s] ",tinyap_node_get_string(e));
	}

	assert(tinyap_node_is_list(e));

	tag=tinyap_node_get_operator(e);

	if(!strcmp(tag,"OperatorRule")) {
		const char*id=tinyap_node_get_string(tinyap_node_get_operand(e,0));
		
		printf("%s ::= ",id);
		print_rule_elem(tinyap_node_get_operand(e,1));
		printf(".\n");
	} else if(!strcmp(tag,"TransientRule")) {
		const char*id=tinyap_node_get_string(tinyap_node_get_operand(e,0));
		printf("%s = ",id);
		print_rule_elem(tinyap_node_get_operand(e,1));
		printf(".\n");
	} else if(!strcmp(tag,"Seq")) {
		int n=tinyap_node_get_operand_count(e);
		int i;
		for(i=0;i<n;i++) {
			print_rule_elem(tinyap_node_get_operand(e,i));
		}
		printf(" ");
	} else if(!strcmp(tag,"Alt")) {
		int n=tinyap_node_get_operand_count(e);
		int i;
		printf("( ");
		print_rule_elem(tinyap_node_get_operand(e,0));
		for(i=1;i<n;i++) {
			printf("| ");
			print_rule_elem(tinyap_node_get_operand(e,i));
		}
		printf(") ");
	} else if(!strcmp(tag,"RE")) {
		const char*esc=tinyap_node_get_string(tinyap_node_get_operand(e,0));
		printf("/%s/ ",esc);
		free((char*)esc);
	} else if(!strcmp(tag,"NT")) {
		const char*esc=tinyap_node_get_string(tinyap_node_get_operand(e,0));
		printf("<%s> ",esc);
		free((char*)esc);
	} else if(!strcmp(tag,"T")) {
		const char*esc=tinyap_node_get_string(tinyap_node_get_operand(e,0));
		if(!strcmp(esc,"\"")) {
			fputs("\"\\\"\" ",stdout);
		} else {
			printf("\"%s\" ",esc);
		}
		free((char*)esc);
	} else if(!strcmp(tag,"EOF")) {
		printf("EOF ");
	} else {
		printf("[NOT IMPLEMENTED [%s]] ",tag);
	}

}


void print_rules_sub(ast_node_t rs) {
	int n=tinyap_node_get_operand_count(rs);
	int i;
	assert(!strcmp(tinyap_node_get_operator(rs),"Grammar"));
	for(i=0;i<n;i++) {
		print_rule_elem(tinyap_node_get_operand(rs,i));
	}
}

void print_rules(ast_node_t rs) {
	if(!rs) return;

	print_rules_sub(tinyap_list_get_element(rs,0));
}


