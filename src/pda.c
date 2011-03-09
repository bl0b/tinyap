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
#include "pda.h"
#include "pda_impl.h"
#include "token_utils.h"

char* ast_serialize_to_string(ast_node_t);
size_t hash_str(hash_key k);
int pda_step_FAIL(pda_t pda, int flags);
int pda_compute_conditions(pda_t pda, int conds);

pda_t pda_new(ast_node_t grammar, const char* whitespace) {
	pda_t ret = (pda_t)malloc(sizeof(struct _pda));
	memset(ret, 0, sizeof(struct _pda));
	ret->nt_cache = (hashtab_t)malloc(sizeof(struct _hashtable));
	init_hashtab(ret->nt_cache, hash_str, (compare_func)strcmp);
	ret->garbage = token_regcomp(whitespace);
	ret->grammar = grammar;
	return ret;
}

void pda_free(pda_t ret) {
	free_stack(ret->states);
	free_stack(ret->productions);
	free_stack(ret->forks);
	clean_hashtab(ret->nt_cache, NULL);
	free(ret->nt_cache);
	free(ret);
}

const char* step_stack_to_string(tinyap_stack_t s);

ast_node_t pda_parse(pda_t pda, const char* source, unsigned long size, ast_node_t start, int pda_flags) {
	int status;
	ProductionState step;
	unsigned long flags, conditions;
	unsigned long step_count = 0;
	ast_node_t begin = newPair(newAtom(Value(Car(Cdr(start))), 0, 0), NULL, 0, 0);
	struct _pda_state* s = tinyap_alloc(struct _pda_state);
	ast_node_t ret = NULL;

	pda->states = new_stack();
	pda->productions = new_stack();
	pda->forks = new_stack();
	pda->source = source;
	pda->ofs = 0;
	pda->size = size;
	pda->farthest = 0;

	fprintf(stderr, "<< %s >>\n", pda->source);

	s->gram_iter = begin;
	s->state_iter = s_init+1;
	/*s->tag = NULL;*/
	s->while_ = NULL;
	s->prod_sp_backup = pda->productions->sp;
	push(pda->states, s);

	do {
		while(!((pda->status&PDA_STATUS_FAILED)||is_empty(pda->states))) {
#define FLAG_IS_SET(_v, _b) (!!((_v)&(_b)))
			int g = !!pda_state(pda)->gram_iter;
			int s = FLAG_IS_SET(pda->status,PDA_STATUS_SUCCEEDED);
			/*DBG*/ProductionState backup;
			step = *pda_state(pda)->state_iter;
			backup = step;
			conditions = 0; /*pda_compute_conditions(pda, step&_COND_MASK);*/
			conditions += ( (step&COND_SUCCEEDED ? !s : 0)
								+
							(step&COND_FAILED ? s : 0)
								+
							(step&COND_ITER_VALID ? !g : 0)
								+
							(step&COND_ITER_NULL ? g : 0));
			conditions = !!conditions;
			flags = (step&_FLAG_MASK);
			step &= _PS_MASK;
#if 1
			fprintf(stderr, "# %-8.8lu  :%u:%lu=%c%c%c%c:%c%c:%c%c:   %s\n",
					step_count,
					step,
					conditions,
					backup&COND_SUCCEEDED? s?'S':'s' : '_',
					backup&COND_FAILED? s?'f':'F' : '_',
					backup&COND_ITER_VALID? g?'V':'v' : '_',
					backup&COND_ITER_NULL? g?'n':'N' : '_',
					/*backup&~_PS_MASK,*/
					backup&FLAG_RAW?'R':'_',
					backup&FLAG_EMPTY?'E':'_',
					pda_state(pda)->flags&FLAG_RAW?'r':'_',
					pda_state(pda)->flags&FLAG_EMPTY?'e':'_',
					step_stack_to_string(pda->states));
#endif
			status = funcs[step][conditions](pda, flags);
			step_count+=1;
			if(not_empty(pda->states)) {
				if(status) {
					if(pda_state(pda)->flags&FLAG_EMPTY) {
						fprintf(stderr, "   IGNORE FAILURE\n");
						pda->productions->sp = pda_state(pda)->prod_sp_backup;
						push(pda->productions, PRODUCTION_OK_BUT_EMPTY);
						(void)_pop(pda->states);
						pda->status&=~PDA_STATUS_SUCCEEDED;
						pda_state(pda)->state_iter+=1;
					} else {
						fprintf(stderr, "   FAILURE\n");
						pda_step_FAIL(pda, 0);
					}
				} else {
					pda_state(pda)->state_iter+=1;
				}
			}
		}
		fprintf(stderr, "Now forks.sp = %li\n", pda->forks->sp);
		if(pda->ofs==pda->size) {
			ret = newPair( peek(ast_node_t, pda->productions), ret, 0, 0);
		}
	} while(!pda_fork_next(pda));
	/*return pda->ofs!=pda->size?NULL:peek(ast_node_t, pda->productions);*/
	return ret;
}


int pda_parse_error_count(pda_t pda) {
	return pda->farthest!=pda->size;
}

int pda_parse_error_row(pda_t pda, int err) {
	return -1;
}

int pda_parse_error_col(pda_t pda, int err) {
	return -1;
}

const char* pda_parse_error(pda_t pda, int err) {
	return "TODO";
}

ast_node_t pda_parse_error_expected(pda_t pda, int err) {
	return pda->expected;
}









void update_pos_cache(pda_t pda) {
	int ln=pda->pos_cache.row;		/* line number */
	size_t ofs=pda->pos_cache.last_ofs;
	size_t end=pda->ofs;
	size_t last_nlofs=pda->pos_cache.last_nlofs;

	if(ofs==end) {
		return;
	}

	if(pda->ofs<pda->pos_cache.last_ofs) {
		while(ofs>end) {
			if(pda->source[ofs]=='\n') {
				--ln;
			}
			--ofs;
		}
		while(ofs>0&&pda->source[ofs]!='\n') {
			--ofs;
		}
		last_nlofs=ofs+(!!ofs);	/* don'pda skip character if at start of buffer */
	} else {
		while(ofs<end) {
			if(pda->source[ofs]=='\n') {
				++ln;
				last_nlofs=ofs+1;
			}
			++ofs;
		}

	}

	if(ln>pda->pos_cache.row) {
		pda->pos_cache.row=ln;
		pda->pos_cache.col=1+end-last_nlofs;
		pda->pos_cache.last_ofs=end;
		pda->pos_cache.last_nlofs=last_nlofs;
		/*node_cache_clean(pda->cache, &pda->pos_cache);*/
	}
	pda->pos_cache.row=ln;
	pda->pos_cache.col=1+end-last_nlofs;
	pda->pos_cache.last_ofs=end;
	pda->pos_cache.last_nlofs=last_nlofs;
	/*printf("update_pos_cache now ofs=%i/%i (%i lines)\n", end, pda->size, pda->pos_cache.row);*/
}


const char* parse_error(pda_t pda) {
	static char err_buf[4096];
	size_t last_nlofs=0;
	size_t next_nlofs=0;
	size_t tab_adjust=0;
	const char* expected;

	pda->ofs=pda->farthest;
	update_pos_cache(pda);
	last_nlofs=pda->ofs-pda->pos_cache.col+1;
	
	next_nlofs=last_nlofs;
	while(pda->source[next_nlofs]&&pda->source[next_nlofs]!='\n') {
		if(pda->source[next_nlofs]=='\t') {
			tab_adjust+=8-((next_nlofs-last_nlofs)&7);	/* snap to tabsize 8 */
		}
		next_nlofs+=1;
	}

	err_buf[0]=0;

	expected = ast_serialize_to_string(pda->expected);

	sprintf(err_buf+strlen(err_buf),"%*.*s\n%*.*s^\nExpected one of %s",
		(int)(next_nlofs-last_nlofs),
		(int)(next_nlofs-last_nlofs),
		pda->source+last_nlofs,
		(int)(pda->farthest-last_nlofs+tab_adjust),
		(int)(pda->farthest-last_nlofs+tab_adjust),
		"",
		expected
	);

	free((char*)expected);

	return err_buf;
}

int parse_error_column(pda_t pda) {
	pda->ofs=pda->farthest;
	update_pos_cache(pda);
	return pda->pos_cache.col;
}

int parse_error_line(pda_t pda) {
	pda->ofs=pda->farthest;
	update_pos_cache(pda);
	return pda->pos_cache.row;
}

