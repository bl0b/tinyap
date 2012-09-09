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
#ifndef _TINYAP_PDA_IMPL_H_
#define _TINYAP_PDA_IMPL_H_

/*#include "config.h"*/
#include "tinyap.h"
#include "ast.h"
#include "tinyap_alloc.h"
#include "string_registry.h"
#include "serialize.h"
/*#include "stack.h"*/
#include "hashtab.h"
/*#include "pda_stack.h"*/

#define DEBUG


#define OP_EOF       1 // immed
#define OP_RE        2 // immed, append or fail
#define OP_T         3 // immed, skip or fail
#define OP_RTR       4 // enter, produce subseq, leave, append or fail
#define OP_ROP       5 // enter, get tag, produce, make [tag], leave, append or fail
#define OP_PREFX     6 // enter, produce car, produce cadr, leave, prefix, append or fail
#define OP_NT        7 // select rule, goto OP_RTR or OP_ROP, append or fail
#define OP_SEQ       8 // SEQUENCE. produce all elements or fail, append
#define OP_ALT       9 // BREAKPOINT. push breakpoint, enter, produce or fail, append
#define OP_POSTFX   10 // enter, produce car, produce cadr, leave, postfix, append or fail
#define OP_RAWSEQ   11 // SEQUENCE. (no whitespace trimming)
#define OP_REP_0N   12 // enter, try produce any times, leave, append
#define OP_REP_01   13 // enter, try produce, leave, append
#define OP_REP_1N   14 // enter, produce, produce any times, leave, append or fail
#define OP_EPSILON  15 // skip
#define OP_RPL      16 // OBSOLETED.
#define OP_STR      17 // immed, append or fail
#define OP_BOW      18 // immed, keep ? append : skip
#define OP_ADDTOBAG 19 // immed, keep ? append : skip
#define OP_BKEEP    20 // .

typedef enum ProductionState_ {
	_STATE_BASE=0,
	PS_FAIL=_STATE_BASE, /* fail, pop back to last fork */
	PS_DONE, /* pop state, continue */
	PS_PRODUCE,
	PS_PRODUCE_EOF,
	PS_PRODUCE_EPSILON,
	PS_PRODUCE_RE,
	PS_PRODUCE_T,
	PS_PRODUCE_STR,
	PS_PRODUCE_BOW,
	PS_SKIP,
	PS_APPEND,
	PS_SET_TAG,
	PS_NEXT,
	PS_WHILE,
	PS_MAKE_OP,
	PS_FORK,
	PS_LOOP,
	PS_PRODUCE_NT,
	PS_PREFIX,
	PS_POSTFIX,
	PS_ADDTOBOW,
	PS_OBSOLETE,
	PS_PUBLISH,
	PS_CHECK_OR_FAIL,
	PS_CHECK_EMPTY,
	_PS_COUNT,
	_PS_MASK=0xFFFF,
	
	_FLAG_BASE=0x10000,
	FLAG_EMPTY=_FLAG_BASE,
	FLAG_RAW=_FLAG_BASE<<1,
	_FLAG_MASK=(_FLAG_BASE<<4)-1-_PS_MASK,

	_COND_BASE=0x1000000,
	COND_ITER_VALID=_COND_BASE,
	COND_ITER_NULL=_COND_BASE<<1,
	COND_SUCCEEDED=_COND_BASE<<2,
	COND_FAILED=_COND_BASE<<3,
	_COND_MASK=(_COND_BASE<<4)-1-_FLAG_MASK-_PS_MASK
} ProductionState;

extern ProductionState* prods[];
extern ProductionState RTR_leftrec[], ROP_leftrec[];


#define PDA_STATUS_SUCCEEDED 1
#define PDA_STATUS_RAW 2
#define PDA_STATUS_ITER_VALID 4
#define PDA_STATUS_CAN_FAIL 8
#define PDA_STATUS_FAILED 16

#define PDA_FLAG_FULL_PARSE 1
#define PDA_FLAG_INPUT_IS_CLEAN 2

#define STEP_NEXT 0
#define STEP_FAIL 1
#define STEP_DOWN 2
#define STEP_UP 3



struct _pda_state {
	struct _pda_state* next;
	ast_node_t gram_iter;
	ProductionState* state_iter;
	const char* tag;
	unsigned long flags;
	ProductionState* while_;
	/*unsigned long prod_sp_backup;*/
	struct _nt_cache_entry* nt;
	ast_node_t productions;
};




struct _nt_cache_entry {
	ast_node_t productions; /* left-recursive rules are rewritten into pair { (non-recursive productions), (Rep0N (recursive productions, first element skipped)) } */
	ast_node_t original_node;
	ProductionState* steps; /* either RTR or ROP... */
	unsigned long current_offset; /* to prevent cycles */
};



struct _fork_entry {
	struct _fork_entry* next;
	long productions_sp;
	long states_sp;
	unsigned long offset;
	/*ast_node_t alt_iter;*/
	ast_node_t alternatives;
	ProductionState* iter;
	/*tinyap_stack_t productions_backup;*/
	/*tinyap_stack_t states_backup;*/
	struct _pda_state* state;
};




struct _pda {
	/* buffer info */
	const char* source;
	unsigned int size;
	/* parser configuration */
	ast_node_t grammar;
	RE_TYPE garbage;
	unsigned int flags;
	/* parse state */
	unsigned int ofs;
	unsigned int status;
	/*tinyap_stack_t states;*/
	/*tinyap_stack_t productions;*/
	/*tinyap_stack_t forks;*/
	struct _fork_entry* forks;
	struct _pda_state* states;
	struct _pos_cache_t pos_cache;
	/* output */
	ast_node_t outputs;
	/* error handling */
	unsigned int farthest;
	ast_node_t expected;
	ast_node_t current_gram_node;
	/* misc */
	hashtab_t nt_cache;
	struct _hashtable bows;
};


/*typedef struct _pda* pda_t;*/



typedef int(*pda_step_t)(struct _pda*, int flags);

extern pda_step_t funcs [_PS_COUNT][2];




static inline int re_exec(const RE_TYPE re, pda_t pda, int* matches, int sz) {
	int ret = pcre_exec(re, NULL, pda->source+pda->ofs, pda->size-pda->ofs, 0, PCRE_ANCHORED, matches, sz);
	/*printf("RE DEBUG : match %p against \"%10.10s\"\n", re, pda->source+pda->ofs);*/
	if(ret<0) {
		/*printf("PCRE error %i\n", ret);*/
		return 0;
	}
	if(matches[0]!=0) {
		printf("PCRE match not at start (%i)\n", matches[0]);
		return 0;
	}
	return 1;
}


static inline struct _pda_state* pda_state(pda_t pda) {
	/*return peek(struct _pda_state*, pda->states);*/
	return pda->states;
}

extern ProductionState s_init[];



static inline void pda_push_production(pda_t pda, ast_node_t p) {
	pda_state(pda)->productions = newPair(p, pda_state(pda)->productions, 0, 0);
}

static inline ast_node_t pda_pop_production(pda_t pda) {
	ast_node_t p = pda_state(pda)->productions, car;
	if(!p) {
		return NULL;
	}
	pda_state(pda)->productions = Cdr(p);
	car = Car(p);
	Car(p)=NULL;
	Cdr(p)=NULL;
	/*delete_node(p);*/
	return car;
}

static inline ast_node_t pda_peek_production(pda_t pda) {
	return pda_state(pda)->productions ? Car(pda_state(pda)->productions) : NULL;
}

static inline void pda_poke_production(pda_t pda, ast_node_t p) {
	if(pda_state(pda)->productions) {
		Car(pda_state(pda)->productions) = p;
	} else {
		pda_state(pda)->productions = newPair(p, NULL, 0, 0);
	}
}




static inline struct _pda_state* pda_push_state(pda_t pda, ast_node_t gram_node, ProductionState* steps, struct _nt_cache_entry* nt) {
	/*struct _pda_state* s = pda_state_push(pda->*/
}



#include <stdarg.h>
static inline int debug_printf(int depth, int cross, char* fmt, ...) {
	va_list va;
	int ret;
	while(depth>!!cross) {
		fprintf(stderr, "|  ");
		depth-=1;
	}
	if(depth>0) {
		fprintf(stderr, "+- ");
	}

	va_start(va, fmt);
	ret = vfprintf(stderr, fmt, va);
	va_end(va);
	return ret;
}


#endif

