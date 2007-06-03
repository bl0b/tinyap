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
#include "ast.h"
#include "bootstrap.h"
#include "tokenizer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



void ast_serialize(ast_node_t*ast,char**output);

ast_node_t*grammar,*ast;

FILE*inputFile,*outputFile;
const char*grammarSource="explicit";	/* default : explicit NFGDG */
const char*textSource="-";		/* default : input from stdin */
const char*outputDest="-";		/* default : output to stdout */

#define cmp_param(_arg_str_long,_arg_str_short) (i<(argc-1) && (	\
		(_arg_str_long && !strcmp(_arg_str_long,argv[i]))	\
					||				\
		(_arg_str_short && !strcmp(_arg_str_short,argv[i]))	\
	))

int get_opts(int argc,char*argv[]) {
	int i;
	for(i=1;i<argc;i+=1) {
		if(cmp_param("--grammar","-g")) {
			i+=1;
			grammarSource=argv[i];
		} else if(cmp_param("--input","-i")) {
			i+=1;
			textSource=argv[i];
		} else if(cmp_param("--output","-o")) {
			i+=1;
			outputDest=argv[i];
		} else if(!(strcmp("--help",argv[i])&&strcmp("-h",argv[i]))) {
			printf("This is not yet another (Java) parser.\n");
			printf("(c) Damien 'bl0b' Leroux 2007\n\n");
			printf("Usage : %s [--input,-i [inputFile]] [--output,-o [outputFile]] [--grammar,-g [grammarFile]] [--help,-h]\n",argv[0]);
			printf("\n\t--grammar,-g name\tuse this grammar to parse input\n");
			printf("\t\t\"" GRAMMAR_EXPLICIT "\"\t(default) selects explicit variant\n");
			printf("\t\t\"" GRAMMAR_CAMELCASING "\"\tselects CamelCasing variant\n");
			printf("\t\tany other string is a filename to read grammar from\n");
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


int main(int argc, char**argv) {
	token_context_t* toktext;
	char*buffer,*ptr;
	int buflen=1048576;

	get_opts(argc,argv);

	grammar=get_ruleset(grammarSource);
	if(!grammar) {
		/* spawn error */
		fprintf(stderr,"Fatal : no grammar :(\n");
		exit(-1);
	}

	if(!strcmp(textSource,"-")) {
		inputFile=stdin;
	} else {
		inputFile=fopen(textSource,"r");
	}
	if(!inputFile) {
		/* spawn error */
		fprintf(stderr,"Fatal : no input :(\n");
		exit(-1);
	}

	if(!strcmp(outputDest,"-")) {
		outputFile=stdout;
	} else {
		outputFile=fopen(outputDest,"w");
	}
	if(!outputFile) {
		/* spawn error */
		fprintf(stderr,"Fatal : no output :(\n");
		exit(-1);
	}

	/* retrieve input text */

	buffer=(char*)malloc(buflen);
	memset(buffer,0,buflen);
	if(fread(buffer,1,buflen,inputFile)<0) {
		perror("Fatal : reading text");
	}
	if(inputFile!=stdin) {
		fclose(inputFile);
	}

	/* tokenize and ast'ize */

	toktext=token_context_new(buffer,strlen(buffer),"[ \t\r\n]+",grammar,STRIP_TERMINALS);
	ast=clean_ast(token_produce_any(toktext,find_nterm(grammar,"_start"),0));

	if(ast) {
		memset(buffer,0,buflen);

		ptr=buffer;

		ast_serialize(ast,&ptr);

		fwrite(buffer,strlen(buffer),1,outputFile);

		if(outputFile!=stdout) {
			fclose(outputFile);
		}
	} else {
		fprintf(stderr,"parse error at line %i, column %i\n%s",parse_error_line(toktext),parse_error_column(toktext),parse_error(toktext));
	}

	free(buffer);

	token_context_free(toktext);

	return 0;
}



#if 0


char*test_bnf="\
number = /-?[0-9]+/ .\n\
math_mult_expr = <number>.\n\
math_mult ::= <math_mult_expr> \"*\" <math_mult_expr>.\n\
math_add_expr = (<math_mult>|<number>) .\n\
math_add ::= <math_add_expr> \"+\" <math_add_expr>.\n\
_start = <math_add> (<_start>|eof).\
";

char*j_random_foobar="23*5+2*35 3*4+5";

char ser_buf[8192];
char*sbuf=ser_buf;

void print_rules(ast_node_t*r);

int main(int argc,char**argv) {
	ast_node_t*bnf_ast = init_BNF_rules();
	token_context_t* toktext;
	ast_node_t*ast,*output;
	int i;
	
	fflush(stderr);
	fflush(stdout);

	debug_write("\n-----------------------------------------------\n");
	print_rules(getCdr(bnf_ast));
	debug_write("\n===============================================\n");

	toktext=token_context_new(test_bnf,"[ \t\r\n]*",bnf_ast,STRIP_TERMINALS);
	ast=token_produce_any(toktext,find_nterm(bnf_ast,"_start"),toktext->flags&STRIP_TERMINALS);
	debug_write("\n** AST AFTER CLEANSING PASS\n");
	ast=clean_ast(ast);
	debug_write("\n");
	if(ast) {
		printf("\n=== Foo_Ast ===\n");
		print_rules(getCdr(ast));
		printf("\n");
	} else {
		printf(parse_error(toktext));
	}

	debug_write("\n-----------------------------------------------\n");
	debug_write(j_random_foobar);
	debug_write("\n-----------------------------------------------\n");

	token_context_free(toktext);

	/* now test parsed grammar with new input */

	toktext=token_context_new(j_random_foobar,"[ \t\r\n]+",ast,STRIP_TERMINALS);
	output=clean_ast(token_produce_any(toktext,find_nterm(ast,"_start"),0));

	memset(ser_buf,0,8192);

	ast_serialize(output,&sbuf);
	printf("\n--\n%s\n--\n",ser_buf);

	sbuf=ser_buf;

	ast_serialize(bnf_ast,&sbuf);
	printf("\n--\n%s\n--\n",ser_buf);

	token_context_free(toktext);

	return 0;
}

#endif



void print_rule_elem(ast_node_t*e) {
	char*tag;
	ast_node_t*t;

	if(!e) {
		return;
	}

	if(isAtom(e)) {
		printf("[OUPS L'ATOME %s] ",Value(e));
	}

	assert(isPair(e));

	tag=Value(getCar(e));

	if(!strcmp(tag,"opr_rule")) {
		char*id=Value(getCar(getCdr(e)));
		printf("%s ::= ",id);
		print_rule_elem(getCar(getCdr(getCdr(e))));
		printf(".\n");
	} else if(!strcmp(tag,"trans_rule")) {
		char*id=Value(getCar(getCdr(e)));
		printf("%s = ",id);
		print_rule_elem(getCar(getCdr(getCdr(e))));
		printf(".\n");
	} else if(!strcmp(tag,"seq")) {
		t=getCdr(e);
		while(t) {
			print_rule_elem(getCar(t));
			t=getCdr(t);
		}
		printf(" ");
	} else if(!strcmp(tag,"alt")) {
		printf("( ");
		t=getCdr(e);
		print_rule_elem(getCar(t));
		t=getCdr(t);
		while(t) {
			printf("| ");
			print_rule_elem(getCar(t));
			t=getCdr(t);
		}
		printf(") ");
	} else if(!strcmp(tag,"RE")) {
		printf("/%s/ ",Value(getCar(getCdr(e))));
	} else if(!strcmp(tag,"NT")) {
		printf("<%s> ",Value(getCar(getCdr(e))));
	} else if(!strcmp(tag,"T")) {
		printf("\"%s\" ",Value(getCar(getCdr(e))));
	} else if(!strcmp(tag,"eof")) {
		printf("eof ");
	} else {
		printf("[NOT IMPLEMENTED [%s]] ",tag);
	}

}


void print_rules(ast_node_t*rs) {
	if(!rs) return;

	if(rs&&isAtom(getCar(rs))&&!strcmp(Value(getCar(rs)),"RuleList")) {
		print_rules(getCdr(rs));
	}

	print_rule_elem(getCar(rs));

	print_rules(getCdr(rs));
}


