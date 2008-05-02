#include "config.h"

#include "tinyap.h"
#include "ast.h"
#include "tokenizer.h"

#include "tinyape.h"

static char sub_re_str[10][1024];
static regex_t* sub_re[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static char sub_str[10][1024];
static size_t sub_ofs[10][2];
static size_t n_sub;

static char buffy[4096];


char* unrepl(const char* re, const char* repl, const char* token) {
	size_t base = 0;
	size_t rec = 0;
	char* p_re = (char*) re;
	char* p_repl = (char*) repl;
	char* p_tok = (char*) token;

	n_sub = 1;

	/* clear everything */
	memset(sub_re_str, 0, 10240);
	memset(sub_str, 0, 10240);
	memset(sub_ofs, 0, 20*sizeof(size_t));

	/* init group offsets */

	sub_ofs[0][0] = p_re-re;
	sub_ofs[0][1] = strlen(re)-1;

	/*for(base=0;base<10;base+=1) {*/
		/*if(sub_re[base]) {*/
			/*regfree(sub_re[base]);*/
			/*sub_re[base]=NULL;*/
		/*}*/
	/*}*/


	/*printf("got regex \"%s\" replacement \"%s\" token \"%s\"\n", re, repl, token);*/

	while(*p_re) {
		if(*p_re=='(') {
			n_sub += 1;
			rec += 1;
			sub_ofs[base+rec][0] = p_re - re +1;
		} else if(*p_re==')') {
			sub_ofs[base+rec][1] = p_re - re;
			rec -= 1;
			if(rec==0) {
				base = n_sub;
			}
		}
		p_re += 1;
	}

	/* init sub regex'es */

	for(base=1;base<=n_sub;base+=1) {
		strncpy(sub_re_str[base], re+sub_ofs[base][0], sub_ofs[base][1]-sub_ofs[base][0]);
		sub_re[base] = token_regcomp(sub_re_str[base]);
		if(sub_re[base] == NULL) {
			printf("regex compilation failed : %s\n", sub_re_str[base]);
		}
	}

	/* parse repl string */

	while(*p_repl&&*p_tok) {
		if(*p_repl=='\\') {
			regmatch_t match;
			match.rm_so=match.rm_eo=0;
			p_repl += 1;
			base = *p_repl - '0';
			p_repl += 1;
			/*printf("exec re /%s/ against %s", sub_re_str[base], p_tok);*/
			if(regexec(sub_re[base], p_tok, 1, &match, 0)!=REG_NOMATCH && match.rm_so==0) {
				strncpy(sub_str[base], p_tok, match.rm_eo+1);
				p_tok += match.rm_eo+1;
				/*printf("matched ! end ofs = %u\n", match.rm_eo);*/
			}
		} else {
			assert(*p_repl==*p_tok);
			p_repl += 1;
			p_tok += 1;
		}
	}

	/* parse re string */
	/* FIXME : currently, everything outside groups is considered immediate strings. Any better ? */

	p_repl = buffy;
	p_re = (char*) re;
	while(*p_re) {
		if(*p_re == '(') {
			size_t ofs = p_re - re;
			for( base = 0 ; sub_ofs[base][0] != (ofs+1); base += 1 );
			p_re = (char*)re + sub_ofs[base][1] + 1;
			strcpy(p_repl, sub_str[base]);
			p_repl += strlen(p_repl);
		} else {
			*p_repl = *p_re;
			p_repl += 1;
			p_re += 1;
		}
	}
	*p_repl = 0;

	for(base=1;base<=n_sub;base+=1) {
		if(sub_re[base]) {
			regfree(sub_re[base]);
			free(sub_re[base]);
			sub_re[base]=NULL;
		}
	}

	return buffy;
}

