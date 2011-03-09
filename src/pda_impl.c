#include "pda.h"
#include "pda_impl.h"

#include "token_utils.h"

const char* funcs_names[_PS_COUNT];

const char* step2str(ProductionState*iter);


void* clone_state(void* s) {
	void* ret = tinyap_alloc(struct _pda_state);
	memcpy(ret, s, sizeof(struct _pda_state));
	return ret;
}


/* silent garbage filter, used before each tokenization */
static inline void _filter_garbage(pda_t t) {
	int token[3];
	if((!(t->flags&PDA_FLAG_INPUT_IS_CLEAN))&&re_exec(t->garbage, t, token, 3)) {
		t->ofs+=token[1];
		update_pos_cache(t);
	}
	/*t->flags|=PDA_FLAG_INPUT_IS_CLEAN;*/
}



void pda_error_expected(pda_t pda) {
	ast_node_t tmp;
	if(pda->farthest>pda->ofs) {
		return;
	}
	if(pda->farthest<pda->ofs) {
		if(pda->expected) {
			delete_node(pda->expected);
		}
		pda->expected=NULL;
		pda->farthest=pda->ofs;
	}
	for(tmp=pda->expected;tmp&&node_compare(Car(tmp), pda->current_gram_node);tmp=Cdr(tmp));
	if(!(tmp&&Car(tmp))) {
		pda->expected = newPair(copy_node(pda->current_gram_node), pda->expected, 0, 0);
	}
}




#define FLAG_IS_SET(_v, _b) (!!((_v)&(_b)))
#define _test_cond(_c, _a, _b)  (FLAG_IS_SET(_c, _b)^(_a))
int pda_compute_conditions(pda_t pda, int conds) {
	if(!conds) {
		return 0;
	} else {
		int g = !!pda_state(pda)->gram_iter;
		int s = FLAG_IS_SET(pda->status,PDA_STATUS_SUCCEEDED);
		int ret = 0;
		if(conds&COND_SUCCEEDED) {
			ret |= s;
		}
		if(conds&COND_FAILED) {
			ret |= !s;
		}
		if(conds&COND_ITER_VALID) {
			ret |= g;
		}
		if(conds&COND_ITER_NULL) {
			ret |= !g;
		}
		return !ret;
	}
	/*return	_test_cond(conds, s,	COND_SUCCEEDED)*/
			/*|*/
			/*( _test_cond(conds, g, COND_ITER_VALID) || FLAG_IS_SET(conds, COND_ITER_NULL) );*/
}


char step_buffer[42];

#ifdef DEBUG
#define PDA_TRACE_STEP(_step) \
	do { \
		char*s = (char*)(pda->source+pda->ofs); \
		size_t _slen = strlen(s); \
		char*d = step_buffer; \
		escape_ncpy(&d, &s, _slen<40?_slen:40, -1); \
		fprintf(stderr, "     %5.5lu     %-80.80s     %10.10s    %80.80s     %u:<< %40s%s >>\n", pda->states->sp, tinyap_serialize_to_string(pda_state(pda)->gram_iter), #_step, tinyap_serialize_to_string(peek(ast_node_t, pda->productions)), pda->ofs, step_buffer, _slen>=40?"...":""); \
	} while(0)
#else
#define PDA_TRACE_STEP(_step) ((void)0)
#endif

int pda_step_DUMMY(pda_t pda, int flags) {
	/*char*s = (char*)(pda->source+pda->ofs);*/
	/*size_t _slen = strlen(s);*/
	/*char*d = step_buffer;*/
	/*escape_ncpy(&d, &s, _slen<40?_slen:40, -1);*/
	/*fprintf(stderr, "     %5.5lu     %-80.80s   ((%10.10s))   %80.80s     %u:<< %40s%s >>\n", pda->states->sp, tinyap_serialize_to_string(pda_state(pda)->gram_iter), funcs_names[(*pda_state(pda)->state_iter)&_PS_MASK], tinyap_serialize_to_string(peek(ast_node_t, pda->productions)), pda->ofs, step_buffer, _slen>=40?"...":"");*/
	return 0;
}




int pda_step_NEXT(pda_t pda, int flags) {
	PDA_TRACE_STEP(NEXT);
	pda_state(pda)->gram_iter = Cdr(pda_state(pda)->gram_iter);
	/*if(!pda_state(pda)->gram_iter) {*/
		/*pda->status |= PDA_STATUS_ITER_VALID;*/
	/*} else {*/
		/*pda->status &= ~PDA_STATUS_ITER_VALID;*/
	/*}*/
	return 0;
}



static inline unsigned int hack_node_tag(ast_node_t node) {
	if(node->node_flags&ATOM_IS_NOT_STRING) {
		return (unsigned int)Value(Car(node));
	}
	/*fprintf(stderr, "   optimizing tag %s\n", Value(Car(node)));*/
	Value(Car(node)) = (char*)string2op(Value(Car(node)));
	node->node_flags|=ATOM_IS_NOT_STRING;
	return (unsigned int) Value(Car(node));
}


int pda_step_PRODUCE(pda_t pda, int flags) {
	ast_node_t elem = Car(pda_state(pda)->gram_iter);
	unsigned int op = hack_node_tag(elem);
	struct _pda_state* s = tinyap_alloc(struct _pda_state);
	s->prod_sp_backup = pda->productions->sp;
	s->gram_iter = Cdr(elem);
	PDA_TRACE_STEP(PRODUCE);
	/*fprintf(stderr, "\nENTER op=%i  steps=%p\n", op, prods[op]);*/
	s->state_iter = prods[op];
	s->flags = flags;
	/*s->tag = NULL;*/
	s->while_ = NULL;
	pda->current_gram_node = elem;
	push(pda->states, s);
	/*pda_step_NEXT(pda, flags);*/
	if(!(pda_state(pda)->flags&FLAG_RAW)) {
		_filter_garbage(pda);
	}
	return 0;
}




int pda_step_SKIP(pda_t pda, int flags) {
	PDA_TRACE_STEP(SKIP);
	return 0;
}




int pda_step_SET_TAG(pda_t pda, int flags) {
	PDA_TRACE_STEP(SET_TAG);
	pda_state(pda)->tag = Value(Car(pda_state(pda)->gram_iter));
	/*push(pda->productions, Car(pda_state(pda)->gram_iter));*/
	return pda_step_NEXT(pda, flags);
}




int pda_step_WHILE(pda_t pda, int flags) {
	PDA_TRACE_STEP(WHILE);
	pda_state(pda)->while_ = pda_state(pda)->state_iter;
	return 0;
}




int pda_step_LOOP(pda_t pda, int flags) {
	PDA_TRACE_STEP(LOOP);
	pda_state(pda)->state_iter = pda_state(pda)->while_;
	return 0;
}




int pda_step_SKIP_TO_LOOP(pda_t pda, int flags) {
	PDA_TRACE_STEP(SKIP_TO_LOOP);
	do {
		pda_state(pda)->state_iter+=1;
	} while(*pda_state(pda)->state_iter != PS_LOOP);
	return 0;
}




int pda_step_DONE(pda_t pda, int flags) {
	PDA_TRACE_STEP(DONE);
	/* pop step, copy if beyond innermost fork */
	/*struct _pda_state*s;*/
	/*struct _fork_entry*f;*/
	/*if(not_empty(pda->forks)*/
		/*&&*/
	   /*pda->states->sp==((f=peek(struct _fork_entry*, pda->forks))->states_sp-1))*/
	/*{*/
		/*f->states_backup = stack_clone(pda->states, clone_state);*/
		/*f->states_backup = stack_dup(pda->states);*/
	/*}*/
	_pop(pda->states);
	pda->status |= PDA_STATUS_SUCCEEDED;
	/*fprintf(stderr, "pop state\n");*/
	return 0;
}


int pda_fork_next(pda_t pda, int flags) {
	while(not_empty(pda->forks)) {
		struct _fork_entry* f = peek(struct _fork_entry*, pda->forks);
		/*if(pda_state(pda)->gram_iter&&Cdr(pda_state(pda)->gram_iter)) {*/
		/*{FIXME}*/
		while(pda->states->sp > f->states_sp) {
			struct _pda_state*s = peek(struct _pda_state*, pda->states);
			if(s->flags&FLAG_EMPTY) {
				return 0;
			}
			(void)_pop(pda->states);
		}
		if(f->productions_backup) {
			free_stack(pda->productions);
			pda->productions = stack_dup(f->productions_backup);
			/* FIXME mustn't discard other parse trees in the end */
		}
		pda->productions->sp = f->productions_sp;
		/*if((pda->states->sp < f->states_sp) && f->states_backup) {*/
		if(f->states_backup) {
			free_stack(pda->states);
			/*pda->states = stack_dup(f->states_backup);*/
			pda->states = stack_clone(f->states_backup, clone_state);
			/* FIXME must COPY states too */
		}
		pda->states->sp = f->states_sp;
		pda_state(pda)->state_iter = f->iter;
		pda_state(pda)->gram_iter = f->alternatives;
		if(f->alternatives) {
			char*s = (char*)(pda->source+f->offset);
			size_t _slen = strlen(s);
			char*d = step_buffer;
			escape_ncpy(&d, &s, _slen<20?_slen:20, -1);
			/*fprintf(stderr, "FORK:%lu:SWITCH TO %s at <<%s>>\n", pda->forks->sp, tinyap_serialize_to_string(Car(f->alternatives)), step_buffer);*/
			pda->ofs = f->offset;
			f->alternatives = Cdr(f->alternatives);
			return 0;
		} else if(pda_state(pda)->flags&FLAG_EMPTY) {
			_pop(pda->states);
			pda->status &= ~PDA_STATUS_SUCCEEDED;
			if(f->state==pda_state(pda)) {
				_pop(pda->forks);
			}
			return 0;
		} else {
			_pop(pda->forks);
		}
	}
	return 1;
}




int pda_step_FAIL(pda_t pda, int flags) {
	PDA_TRACE_STEP(FAIL);
	if(pda_fork_next(pda, flags)&&!(pda_state(pda)->flags&FLAG_EMPTY)) {
		/*abort();*/
		pda->status|=PDA_STATUS_FAILED;
		pda->status&=~PDA_STATUS_SUCCEEDED;
	}
}



void pda_check_fork_productions(pda_t pda, int n_elems) {
	if(not_empty(pda->forks)) {
		struct _fork_entry* f = peek(struct _fork_entry*, pda->forks);
		if(f->productions_sp==(pda->productions->sp+1-n_elems)) {
			f->productions_backup=stack_dup(pda->productions);
			/*fprintf(stderr, "BACKUP PRODUCTION STACK IN FORK #%li\n", pda->forks->sp);*/
		}
	}
}





int pda_step_MAKE_OP(pda_t pda, int flags) {
	PDA_TRACE_STEP(MAKE_OP);
	ast_node_t body/*, head*/;
	pda_check_fork_productions(pda, 1);
	body = pop(ast_node_t, pda->productions);
	/*head = pop(ast_node_t, pda->productions);*/
	push(pda->productions, 
		newPair(
			newPair(
				/*copy_node(head),*/
				newAtom(pda_state(pda)->tag, pda->pos_cache.row, pda->pos_cache.col),
				body==PRODUCTION_OK_BUT_EMPTY?NULL:body,
				pda->pos_cache.row, pda->pos_cache.col),
			NULL,
			pda->pos_cache.row, pda->pos_cache.col));
	return 0;
}




int pda_step_APPEND(pda_t pda, int flags) {
	PDA_TRACE_STEP(APPEND);
	ast_node_t tail, head;
	pda_check_fork_productions(pda, 2);
	tail = pop(ast_node_t, pda->productions);
	head = pop(ast_node_t, pda->productions);
	push(pda->productions, SafeAppend(head, tail));
	return 0;
}




int pda_step_DISCARD(pda_t pda, int flags) {
	PDA_TRACE_STEP(DISCARD);
	pda_check_fork_productions(pda, 1);
	(void)_pop(pda->productions);
	return 0;
}




int pda_step_PREFIX(pda_t pda, int flags) {
	ast_node_t pfx, ret;
	PDA_TRACE_STEP(PREFIX);
	pda_check_fork_productions(pda, 2);
	ret = pop(ast_node_t, pda->productions);
	/*PDA_TRACE_STEP(PREFIX);*/
	pfx = pop(ast_node_t, pda->productions);
	/*PDA_TRACE_STEP(PREFIX);*/

	/*fprintf(stderr, "have prefix %s\n",tinyap_serialize_to_string(pfx));*/
	/*fprintf(stderr, "have expr %s\n",tinyap_serialize_to_string(ret));*/
	if(ret==PRODUCTION_OK_BUT_EMPTY) {
		push(pda->productions, pfx);
	} else if(pfx&&pfx!=PRODUCTION_OK_BUT_EMPTY &&
		  ret && ret->pair._car) {
		ast_node_t tail;
		/* these copies are necessary because of structural hack.
		 * Not copying botches the node cache.
		 */
		ret=copy_node(ret);
		pfx=copy_node(pfx);
		/*fprintf(stderr, "\thave prefix %s\n",tinyap_serialize_to_string(pfx));*/
		/*fprintf(stderr, "\thave expr %s\n",tinyap_serialize_to_string(ret));*/

		tail=pfx;
		//ret->pair._car->pair._cdr = Append(pfx,ret->pair._car->pair._cdr);
		/* FIXME ? Dirty hack. */
		while(tail->pair._cdr) {
			tail = tail->pair._cdr;
		}
		tail->pair._cdr = ret->pair._car->pair._cdr;
		ret->pair._car->pair._cdr = pfx;
		fprintf(stderr, "\nhave merged into %s\n\n",tinyap_serialize_to_string(ret));
		push(pda->productions, ret);
	}
	return 0;
}




int pda_step_POSTFIX(pda_t pda, int flags) {
	PDA_TRACE_STEP(POSTFIX);
	ast_node_t pfx, ret;
	pda_check_fork_productions(pda, 2);
	ret = pop(ast_node_t, pda->productions);
	if(ret==PRODUCTION_OK_BUT_EMPTY) {
		return 0;
	}
	pfx = pop(ast_node_t, pda->productions);
	if(pfx!=PRODUCTION_OK_BUT_EMPTY) {
		ret->pair._car = Append(ret->pair._car,copy_node(pfx));
	}
	push(pda->productions, ret);
	return 0;
}




int pda_step_FORK(pda_t pda, int flags) {
	if(is_empty(pda->forks)||peek(struct _fork_entry*, pda->forks)->states_sp!=pda->states->sp) {
		struct _fork_entry*f = tinyap_alloc(struct _fork_entry);
		PDA_TRACE_STEP(FORK);
		f->productions_sp = pda->productions->sp;
		f->productions_backup = NULL;
		f->states_sp = pda->states->sp;
		/*f->states_backup = NULL;*/
		f->states_backup = stack_clone(pda->states, clone_state);
		f->iter = pda_state(pda)->state_iter;
		f->offset = pda->ofs;
		f->state = pda_state(pda);
		f->alternatives = pda_state(pda)->gram_iter;
		push(pda->forks, f);
		/*fprintf(stderr, "FORK:%lu %s\n", pda->forks->sp, tinyap_serialize_to_string(pda_state(pda)->gram_iter));*/
		pda_fork_next(pda, flags);
	}
	/*pda_step_PRODUCE(pda, flags);*/
	return 0;
}




int pda_step_PRODUCE_T(pda_t pda, int flags) {
	PDA_TRACE_STEP(_T);
	/*const char*token=Value(Car(Cdr(Car(pda_state(pda)->gram_iter))));*/
	const char*token=Value(Car(pda_state(pda)->gram_iter));
	size_t slen=strlen(token);
	if(!strncmp(pda->source+pda->ofs,token,slen)) {
		pda->ofs+=slen;
		push(pda->productions, PRODUCTION_OK_BUT_EMPTY);
		pda->status |= PDA_STATUS_SUCCEEDED;
		update_pos_cache(pda);
		return 0;
	} else {
		/* TODO : ERROR HANDLING */
		pda->status &= ~PDA_STATUS_SUCCEEDED;
		pda_error_expected(pda);
		return 1;
	}
}




int pda_step_PRODUCE_RE(pda_t pda, int flags) {
	PDA_TRACE_STEP(_RE);
	int token[3];
	ast_node_t re = Car(pda_state(pda)->gram_iter);
	const RE_TYPE expr;
	if(!re->raw._p2) {
		re->raw._p2=token_regcomp(Value(re));
	}
	expr  = re->raw._p2;
	if(re_exec(expr, pda, token, 3)) {
		char*lbl=match2str(pda->source+pda->ofs,0,token[1]);
		push(pda->productions, newPair(newAtom(lbl, pda->pos_cache.row, pda->pos_cache.col), NULL, pda->pos_cache.row, pda->pos_cache.col));
		pda->ofs+=token[1];
		update_pos_cache(pda);
		return 0;
	} else {
		/* TODO : ERROR HANDLING */
		pda->status &= ~PDA_STATUS_SUCCEEDED;
		pda_error_expected(pda);
		return 1;
	}
}




int pda_step_PRODUCE_STR(pda_t pda, int flags) {
	PDA_TRACE_STEP(_STR);
	char* ret = NULL;
	char* _src;
	char* _end;
	char* _match;
	char* prefix=Value(Car(pda_state(pda)->gram_iter));
	char* suffix=Value(Cadr(pda_state(pda)->gram_iter));
	size_t slen = strlen(prefix);
	/*printf(__FILE__ ":%i\n", __LINE__);*/
	if(*prefix&&strncmp(pda->ofs+pda->source,prefix,slen)) {
		pda_error_expected(pda);
		return 1;
	}
	/*printf(__FILE__ ":%i\n", __LINE__);*/
	_src = (char*)(pda->ofs+pda->source+slen);
	if(!*suffix) {
		/*printf(__FILE__ ":%i\n", __LINE__);*/
		_match = ret = _stralloc(pda->source+pda->size-_src+1);
		escape_ncpy(&_match, &_src, pda->source+pda->size-_src, -1);
		pda->ofs = pda->size;
	} else {
		/*printf(__FILE__ ":%i\n", __LINE__);*/
		_end = _src;
		/*printf(__FILE__ ":%i\n", __LINE__);*/
		while((_match=strchr(_end, (int)*suffix))&&_match>_end&&*(_match-1)=='\\') {
			/*printf(__FILE__ ":%i\n", __LINE__);*/
			_end = _match+1;
		}
		/*printf(__FILE__ ":%i\n", __LINE__);*/
		if(!_match) {
			/*token_expected_at(pda, str);*/
			return 1;
		}
		/*printf(__FILE__ ":%i\n", __LINE__);*/
		_end = _match;
		ret = _stralloc(_end-_src+1);
		_match = ret;
		escape_ncpy(&_match, &_src, _end-_src, (int)*suffix);
		*_match=0;
		pda->ofs = _end-pda->source+1;
	}
	/*printf(__FILE__ ":%i\n", __LINE__);*/
	push(pda->productions, newPair(newAtom(ret, pda->pos_cache.row, pda->pos_cache.col), NULL, pda->pos_cache.row, pda->pos_cache.col));
	update_pos_cache(pda);
	return 0;
}




int pda_step_PRODUCE_BOW(pda_t pda, int flags) {
	PDA_TRACE_STEP(_BOW);
	unsigned long slen = match_bow(pda, Value(Car(pda_state(pda)->gram_iter)));
	if(slen>0) {
		ast_node_t ret;
		if(!Cdr(pda_state(pda)->gram_iter)) {
			ret = PRODUCTION_OK_BUT_EMPTY;
		} else {
			char*tok = _stralloc(slen+1);
			strncpy(tok, pda->source+pda->ofs, slen);
			tok[slen]=0;
			ret = newPair(	newAtom(tok, pda->pos_cache.row, pda->pos_cache.col),
					NULL, pda->pos_cache.row, pda->pos_cache.col);
		}
		pda->ofs+=slen;
		update_pos_cache(pda);
		push(pda->productions, ret);
		return 0;
	}
	pda_error_expected(pda);
	/*token_expected_at(pda, bow);*/
	return 1;
}





int pda_step_ADDTOBOW(pda_t pda, int flags) {
	PDA_TRACE_STEP(ADDTOBOW);
	char* name = Value(Car(pda_state(pda)->gram_iter));
	token_bow_add(pda, name, Value(Car(peek(ast_node_t, pda->productions))));
	return 0;
}





int pda_step_OBSOLETE(pda_t pda, int flags) {
	PDA_TRACE_STEP(OBSOLETE);
	fprintf(stderr, "OOPS, you are using obsolete stuff.\n");
	return 1;
}





int pda_step_PRODUCE_EOF(pda_t pda, int flags) {
	PDA_TRACE_STEP(_EOF);
	if(pda->size==pda->ofs) {
		/*fprintf(stderr, "      => at EOF !\n");*/
		push(pda->productions, PRODUCTION_OK_BUT_EMPTY);
		pda->status |= PDA_STATUS_SUCCEEDED;
		return 0;
	} else {
		/* TODO : ERROR HANDLING */
		/*fprintf(stderr, "      => NOT at EOF.\n");*/
		pda->status &= ~PDA_STATUS_SUCCEEDED;
		pda_error_expected(pda);
		return 1;
	}
}




int pda_step_PRODUCE_EPSILON(pda_t pda, int flags) {
	PDA_TRACE_STEP(_EPSILON);
	push(pda->productions, PRODUCTION_OK_BUT_EMPTY);
	pda->status |= PDA_STATUS_SUCCEEDED;
	return 0;
}



int is_leftrec(const char* name, ast_node_t expr) {
	const char* tag = Value(Car(expr));
	if(!(TINYAP_STRCMP(tag, STR_Seq)
			&&
		 TINYAP_STRCMP(tag, STR_Alt)
			&&
		 TINYAP_STRCMP(tag, STR_Prefix)
			&&
		 TINYAP_STRCMP(tag, STR_Postfix)
	)) {
		return is_leftrec(name, Car(Cdr(expr)));
	}
	if(!TINYAP_STRCMP(tag, STR_NT)) {
		return !strcmp(Value(Car(Cdr(expr))), name);
	}
	return 0;
}


void split_leftrec(const char* name, ast_node_t alt, ast_node_t* non_rec, ast_node_t* rec) {
	ast_node_t seq;
	if(!alt) { return; }
	seq = Car(alt);
	split_leftrec(name, Cdr(alt), non_rec, rec);
	if(is_leftrec(name, seq)) {
		ast_node_t tmp;
		if(Cdr(Cdr(Cdr(seq)))) {
			tmp = newPair(Car(seq), Cdr(Cdr(seq)), 0, 0);
		} else {
			tmp = Car(Cdr(Cdr(seq)));
		}
		*rec = newPair(tmp, *rec, 0, 0);
	} else {
		*non_rec = newPair(copy_node(seq), *non_rec, 0, 0);
	}
}


struct _nt_cache_entry _SKIP_NT;

struct _nt_cache_entry* pda_find_nterm(pda_t pda, const char* tag) {
	if(!(TINYAP_STRCMP(tag, STR_Space)
			&&
		 TINYAP_STRCMP(tag, STR_NewLine)
		 	&&
		 TINYAP_STRCMP(tag, STR_Indent)
		 	&&
		 TINYAP_STRCMP(tag, STR_Dedent))) {
		 /*fprintf(stderr, "     skip NT \"%s\"\n", tag);*/
		 return &_SKIP_NT;
	}
	struct _nt_cache_entry* c = hash_find(pda->nt_cache, (hash_key) tag);
	int typ;
	if(!c) {
		ast_node_t non_rec=NULL, rec=NULL, rule;
		/*fprintf(stderr, "  find nt %s ?\n", tag);*/
		c = tinyap_alloc(struct _nt_cache_entry);
		rule = find_nterm(pda->grammar, tag);
		/*fprintf(stderr, "    got %s\n", tinyap_serialize_to_string(rule));*/
		typ = hack_node_tag(rule);
		if(!TINYAP_STRCMP(Value(Car(Car(Cdr(Cdr(rule))))), STR_Alt)) {
			split_leftrec(Value(Car(Cdr(rule))), Cdr(Car(Cdr(Cdr(rule)))), &non_rec, &rec);
			c->productions = newPair(newAtom(tag, 0, 0), newPair(
				Cdr(non_rec)?newPair(newAtom(STR_Alt, 0, 0), non_rec, 0, 0):Car(non_rec),
				rec	? Cdr(rec) 	? newPair(	newPair(newAtom(STR_Alt, 0, 0), rec, 0, 0),
													NULL, 0, 0)
								: rec
					: NULL,
				0, 0), 0, 0);
		} else {
			c->productions = Cdr(rule);
		}
		/*fprintf(stderr, "    productions for %s : %s\n", tag, tinyap_serialize_to_string(c->productions));*/
		c->current_offset=-1;
		c->steps = typ==OP_ROP
				?( rec ? ROP_leftrec : prods[OP_ROP] )
				:( rec ? RTR_leftrec : prods[OP_RTR] )
				;
		hash_addelem(pda->nt_cache, (hash_key)tag, (hash_elem)c);
	}
	return c;
}



int pda_step_PRODUCE_NT(pda_t pda, int flags) {
	PDA_TRACE_STEP(_NT);
	struct _nt_cache_entry* nt = pda_find_nterm(pda, Value(Car(pda_state(pda)->gram_iter)));
	if(nt==&_SKIP_NT) {
		push(pda->productions, PRODUCTION_OK_BUT_EMPTY);
		return 0;
	} else {
		struct _pda_state* s = tinyap_alloc(struct _pda_state);
		s->prod_sp_backup = pda->productions->sp;
		s->gram_iter = nt->productions;
		s->state_iter = nt->steps;
		pda->current_gram_node = nt->original_node;
		s->flags = flags;
		s->tag = NULL;
		s->while_ = NULL;
		/*fprintf(stderr, "\nENTER NT  steps=%p  prods=%s\n", nt->steps, tinyap_serialize_to_string(nt->productions));*/
		push(pda->states, s);
		/*pda_step_NEXT(pda, flags);*/
		pda_error_expected(pda);
		return 0;
	}
}

















ProductionState
	s_fail[] = {PS_FAIL, PS_FAIL },
	s_eof[] = { PS_FAIL, PS_PRODUCE_EOF, PS_DONE }, /* EOF */
	s_re[] = { PS_FAIL, PS_PRODUCE_RE, PS_DONE }, /* RE */
	s_t[] = { PS_FAIL, PS_PRODUCE_T, PS_DONE }, /* T */
	s_rtr[] = { PS_FAIL, PS_NEXT, PS_PRODUCE, PS_DONE }, /* RTR */
	s_rop[] = { PS_FAIL, PS_SET_TAG, PS_PRODUCE, PS_MAKE_OP, PS_DONE }, /* ROP */
	s_pfx[] = { PS_FAIL, PS_PRODUCE, PS_NEXT, PS_PRODUCE, PS_PREFIX, PS_DONE }, /* PREFX */
	s_nt[] = { PS_FAIL, PS_PRODUCE_NT, PS_DONE }, /* NT */
	s_seq[] = { PS_FAIL, PS_PRODUCE, PS_NEXT, PS_WHILE, PS_PRODUCE, PS_APPEND, PS_NEXT, PS_LOOP|COND_ITER_VALID, PS_DONE }, /* SEQ */
	s_alt[] = { PS_FAIL, PS_FORK, PS_PRODUCE, PS_DONE }, /* ALT */
	s_sfx[] = { PS_FAIL, PS_PRODUCE, PS_NEXT, PS_PRODUCE, PS_POSTFIX, PS_DONE }, /* POSTFX */
	s_rseq[] = { PS_FAIL, PS_PRODUCE|FLAG_RAW, PS_NEXT, PS_WHILE, PS_PRODUCE|FLAG_RAW, PS_APPEND, PS_NEXT, PS_LOOP|COND_ITER_VALID, PS_DONE }, /* RAWSEQ */
	s_r0n[] = { PS_FAIL, PS_PRODUCE|FLAG_EMPTY, PS_WHILE|COND_SUCCEEDED, PS_PRODUCE|FLAG_EMPTY, PS_APPEND|COND_SUCCEEDED, PS_LOOP|COND_SUCCEEDED, PS_DONE }, /* REP_0N */
	s_r01[] = { PS_FAIL, PS_PRODUCE|FLAG_EMPTY, PS_DONE }, /* REP_01 */
	s_r1n[] = { PS_FAIL, PS_PRODUCE, PS_WHILE, PS_PRODUCE|FLAG_EMPTY, PS_APPEND|COND_SUCCEEDED, PS_LOOP|COND_SUCCEEDED, PS_DONE }, /* REP_1N */
	s_eps[] = { PS_FAIL, PS_PRODUCE_EPSILON, PS_DONE }, /* EPSILON */
	s_rpl[] = { PS_FAIL, PS_OBSOLETE , PS_DONE }, /* RPL */
	s_str[] = { PS_FAIL, PS_PRODUCE_STR, PS_DONE }, /* STR */
	s_bow[] = { PS_FAIL, PS_PRODUCE_BOW, PS_DONE }, /* BOW */
	s_atb[] = { PS_FAIL, PS_PRODUCE, PS_NEXT, PS_ADDTOBOW, PS_DONE }, /* ADDTOBAG */
	s_bkp[] = { PS_FAIL, PS_FAIL }, /* BKEEP */
	s_init[] = { PS_FAIL, PS_PRODUCE_NT, PS_DONE } /* BKEEP */
	;


const char* prods_names[] = {
	"fail",
	"eof",
	"re",
	"t", 
	"rtr",
	"rop",
	"pfx",
	"nt",
	"seq",
	"alt",
	"sfx",
	"rseq",
	"r0n",
	"r01",
	"r1n",
	"eps",
	"rpl",
	"str",
	"bow",
	"atb",
	"bkp",
	"init",
	"rtr_lr",
	"rop_lr"
};


const char* step2str(ProductionState*iter) {
	int i, imin=0;
	ptrdiff_t min=100, tmp;
	static char buf[20];
	for(i=0;i<24;i+=1) {
		tmp = iter - prods[i];
		if(tmp<min&&tmp>=0) {
			min=tmp;
			imin=i;
		}
	}
	sprintf(buf, "%3.3s+%i", prods_names[imin], min);
	return buf;
}

const char* step_stack_to_string(tinyap_stack_t s) {
	static char buffer[4096];
	char*buf = buffer;
	int i;
	if(is_empty(s)) {
		return "(empty)";
	}
	for(i=0;i<s->sp;i+=1) {
		sprintf(buf, "%s, ", step2str(((struct _pda_state*)s->stack[i])->state_iter));
		buf+=strlen(buf);
	}
	strcpy(buf, step2str(((struct _pda_state*)s->stack[i])->state_iter));
	return buffer;
}

ProductionState* prods[] = {
	s_fail,
	s_eof,
	s_re,
	s_t, 
	s_rtr,
	s_rop,
	s_pfx,
	s_nt,
	s_seq,
	s_alt,
	s_sfx,
	s_rseq,
	s_r0n,
	s_r01,
	s_r1n,
	s_eps,
	s_rpl,
	s_str,
	s_bow,
	s_atb,
	s_bkp,
	s_init,
	RTR_leftrec,
	ROP_leftrec
};


ProductionState RTR_leftrec[] = {
	PS_FAIL,
	PS_NEXT,
	PS_PRODUCE,
	PS_NEXT|COND_SUCCEEDED,
	PS_WHILE|COND_SUCCEEDED,
		PS_PRODUCE|FLAG_EMPTY,
		PS_APPEND|COND_SUCCEEDED,
	PS_LOOP|COND_SUCCEEDED,
	PS_DONE
};

ProductionState ROP_leftrec[] = {
	PS_FAIL,
	PS_SET_TAG,
	PS_PRODUCE,
	PS_NEXT|COND_SUCCEEDED,
	PS_WHILE|COND_SUCCEEDED,
		PS_MAKE_OP,
		PS_PRODUCE|FLAG_EMPTY,
		PS_APPEND|COND_SUCCEEDED,
	PS_LOOP|COND_SUCCEEDED,
	PS_MAKE_OP,
	PS_DONE
};


pda_step_t funcs[_PS_COUNT][2] = {
	{ pda_step_FAIL,			pda_step_DUMMY },
	{ pda_step_DONE,			pda_step_DUMMY },
	{ pda_step_PRODUCE,			pda_step_DUMMY },
	{ pda_step_PRODUCE_EOF,		pda_step_DUMMY },
	{ pda_step_PRODUCE_EPSILON,	pda_step_DUMMY },
	{ pda_step_PRODUCE_RE,		pda_step_DUMMY },
	{ pda_step_PRODUCE_T,		pda_step_DUMMY },
	{ pda_step_PRODUCE_STR,		pda_step_DUMMY },
	{ pda_step_PRODUCE_BOW,		pda_step_DUMMY },
	{ pda_step_SKIP,			pda_step_DUMMY },
	{ pda_step_APPEND,			pda_step_DISCARD },
	{ pda_step_SET_TAG,			pda_step_DUMMY },
	{ pda_step_NEXT,			pda_step_DUMMY },
	{ pda_step_WHILE,			pda_step_SKIP_TO_LOOP },
	{ pda_step_MAKE_OP,			pda_step_DUMMY },
	{ pda_step_FORK,			pda_step_DUMMY },
	{ pda_step_LOOP,			pda_step_DUMMY },
	{ pda_step_PRODUCE_NT,		pda_step_DUMMY },
	{ pda_step_PREFIX,			pda_step_DUMMY },
	{ pda_step_POSTFIX,			pda_step_DUMMY },
	{ pda_step_ADDTOBOW,		pda_step_DUMMY },
	{ pda_step_OBSOLETE,		pda_step_DUMMY }
};


const char* funcs_names[_PS_COUNT] = {
		"FAIL",
		"DONE",
		"PRODUCE",
		"PRODUCE_EOF",
		"PRODUCE_EPSILON",
		"PRODUCE_RE",
		"PRODUCE_T",
		"PRODUCE_STR",
		"PRODUCE_BOW",
		"SKIP",
		"APPEND",
		"SET_TAG",
		"NEXT",
		"WHILE",
		"MAKE_OP",
		"FORK",
		"LOOP",
		"PRODUCE_NT",
		"PREFIX",
		"POSTFIX",
		"ADDTOBOW",
		"OBSOLETE"
};
