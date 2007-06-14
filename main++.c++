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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>


#define TINYAP_ABOUT	"This is not yet another parser.\n" \
			"(c) 2007 Damien 'bl0b' Leroux\n\n"

using TinyaP::Parser;
using TinyaP::AstNode;

void print_rules(AstNode* rs);

//ast_node_t grammar,ast;

//FILE*inputFile,*outputFile;
//const char*grammarSource="explicit";	/* default : explicit NFGDG */
//int print_grammar=0;
//const char*textSource="-";		/* default : input from stdin */
//const char*outputDest="-";		/* default : output to stdout */

#define cmp_param(_n,_arg_str_long,_arg_str_short) (i<(argc-_n) && (	\
		(_arg_str_long && !strcmp(_arg_str_long,argv[i]))	\
					||				\
		(_arg_str_short && !strcmp(_arg_str_short,argv[i]))	\
	))

int do_args(int argc,char*argv[]) {
	int i;
	//tinyap_t parser = tinyap_new();
	TinyaP::Parser parser;
	
	for(i=1;i<argc;i+=1) {
		if(cmp_param(1,"--grammar","-g")) {
			i+=1;
			parser.setGrammar(argv[i]);
		} else if(cmp_param(1,"--input","-i")) {
			i+=1;
			parser.setSourceFile(argv[i]);
		} else if(cmp_param(1,"--output","-o")) {
			i+=1;
			if(parser.parsedOK()) {
				parser.getOutput()->toFile(argv[i]);
			}
		} else if(cmp_param(0,"--parse","-p")) {
			if(!parser.parse()) {
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",parser.getErrorRow(),parser.getErrorCol(),parser.getError());
			}
		} else if(cmp_param(0,"--parse-as-grammar","-pag")) {
			if(!parser.parseAsGrammar()) {
				fprintf(stderr,"parse error at line %i, column %i\n%s\n",parser.getErrorRow(),parser.getErrorCol(),parser.getError());
			}
		} else if(cmp_param(0,"--print-grammar","-pg")) {
			print_rules(parser.getGrammarAst());
			fputc('\n',stdout);
		} else if(cmp_param(0,"--version","-v")) {
			printf(TINYAP_ABOUT);
			printf("version " TINYAP_VERSION "\n" );
		} else if(cmp_param(0,"--help","-h")) {
			printf(TINYAP_ABOUT);
			printf("FIXME: Usage is not up-to-date\n\nUsage : %s [--input,-i [inputFile]] [--output,-o [outputFile]] [--grammar,-g [grammarFile]] [--help,-h]\n",argv[0]);
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
			printf("\n\t--help,-h\t\tdisplay this text\n\n");
			exit(0);
		}
	}

	return 0;
}



extern volatile int _node_alloc_count;

int main(int argc, char**argv) {
	
	return do_args(argc,argv);
}




void print_rule_elem(AstNode*e) {
	const char*tag;
	//ast_node_t t;

	if(!e) {
		return;
	}

	if(e->isString()) {
		printf("[OUPS L'ATOME %s] ",e->getString());
	}

	assert(e->isList());

	tag=e->getOperator();

	if(!strcmp(tag,"OperatorRule")) {
		const char*id=e->getOperand(0)->getString();
		
		printf("%s ::= ",id);
		print_rule_elem(e->getOperand(1));
		printf(".\n");
	} else if(!strcmp(tag,"TransientRule")) {
		const char*id=e->getOperand(0)->getString();
		printf("%s = ",id);
		print_rule_elem(e->getOperand(1));
		printf(".\n");
	} else if(!strcmp(tag,"Seq")) {
		int n=e->getOperandCount();
		int i;
		for(i=0;i<n;i++) {
			print_rule_elem(e->getOperand(i));
		}
		printf(" ");
	} else if(!strcmp(tag,"Alt")) {
		int n=e->getOperandCount();
		int i;
		printf("( ");
		print_rule_elem(e->getOperand(0));
		for(i=1;i<n;i++) {
			printf("| ");
			print_rule_elem(e->getOperand(i));
		}
		printf(") ");
	} else if(!strcmp(tag,"RE")) {
		const char*esc=e->getOperand(0)->getString();
		printf("/%s/ ",esc);
		free((char*)esc);
	} else if(!strcmp(tag,"NT")) {
		const char*esc=e->getOperand(0)->getString();
		printf("<%s> ",esc);
		free((char*)esc);
	} else if(!strcmp(tag,"T")) {
		const char*esc=e->getOperand(0)->getString();
		printf("\"%s\" ",esc);
		free((char*)esc);
	} else if(!strcmp(tag,"EOF")) {
		printf("EOF ");
	} else {
		printf("[NOT IMPLEMENTED [%s]] ",tag);
	}

}


void print_rules_sub(AstNode* rs) {
	int n=rs->getOperandCount();
	int i;
	assert(!strcmp(rs->getOperator(),"Grammar"));
	for(i=0;i<n;i++) {
		print_rule_elem(rs->getOperand(i));
	}
}

void print_rules(AstNode* rs) {
	if(!rs) return;

	print_rules_sub(rs->getElement(0));
}


