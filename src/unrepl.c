#include "config.h"

#include "tinyap.h"
#include "ast.h"
#include "tokenizer.h"

#include "tinyape.h"

static char sub_re_str[10][1024];
static regex_t* sub_re[10];
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

	n_sub = 0;

	/* init group offsets */

	sub_ofs[0][0] = p_re-re;
	sub_ofs[0][1] = strlen(re)-1;

	printf("got regex \"%s\" replacement \"%s\" token \"%s\"\n", re, repl, token);

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

	for(base=1;base<n_sub;base+=1) {
		strncpy(sub_re_str[base], re+sub_ofs[base][0], sub_ofs[base][1]-sub_ofs[base][2]+1);
		sub_re[base] = token_regcomp(sub_re_str[base]);
		sub_str[base][0] = 0;
	}

	/* parse repl string */

	while(*p_repl&&*p_tok) {
		if(*p_repl=='\\') {
			regmatch_t match;
			p_repl += 1;
			base = *p_repl - '0';
			p_repl += 1;
			if(regexec(sub_re[base], p_tok, 1, &match, 0)!=REG_NOMATCH && match.rm_so==0) {
				strncpy(sub_str[base], p_tok, match.rm_eo+1);
				p_tok += match.rm_eo+1;
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

	return buffy;
}

