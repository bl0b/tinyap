#include <stdlib.h>
#include <string.h>
#include "stack.h"

stack_t new_stack() {
	stack_t ret = (stack_t) malloc(sizeof(struct _stack_t));
	memset(ret,0,sizeof(struct _stack_t));
	ret->sp=-1;
	return ret;
}

void push(stack_t s, wast_t w) {
	s->sp += 1;
	if(s->sz == s->sp) {
		s->sz+=16;
		s->stack = (wast_t*) realloc(s->stack, s->sz*sizeof(wast_t));
	}
	s->stack[s->sp] = w;
}

wast_t pop(stack_t s) {
	wast_t ret = s->stack[s->sp];
	s->sp -= 1;
	return ret;
}

wast_t peek(stack_t s) {
	return s->stack[s->sp];
}

void free_stack(stack_t s) {
	if(s->stack) {
		free(s->stack);
	}
	free(s);
}


