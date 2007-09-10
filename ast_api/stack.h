#ifndef __TINYAP_WAST_STACK_H__
#define __TINYAP_WAST_STACK_H__

#include "walkableast.h"

typedef struct _stack_t {
	size_t sz;
	size_t sp;
	wast_t* stack;
}* stack_t;

stack_t new_stack();
void push(stack_t s, wast_t w);
wast_t pop(stack_t s);
wast_t peek(stack_t s);
void free_stack(stack_t s);
#endif

