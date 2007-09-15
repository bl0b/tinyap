#include "../tinyap.h"
#include "pilot_manager.h"
#include "walker.h"

typedef struct _interp_t {
	int stack[256];
	int sp;
}* interp_t;

static void dump(interp_t i) {
	int k;
	for(k=0;k<=i->sp;k+=1) {
		printf("%i, ",i->stack[k]);
	}
	printf("\n");
}

static void init(interp_t* i) {
//	printf("init with %p\n",*i);
	if(*i==NULL) {
		*i = (interp_t) malloc(sizeof(struct _interp_t));
		memset(*i,0,sizeof(struct _interp_t));
		(*i)->sp=-1;
	}
//	dump(*i);
}

static void push(interp_t i, int n) {
	i->sp += 1;
//	printf("push %i at %i\n",n,i->sp);
	i->stack[i->sp]=n;
//	dump(i);
}

static int peek(interp_t i) {
	return i->stack[i->sp];
}

static int pop(interp_t i) {
	int ret = i->stack[i->sp];
//	printf("pop %i\n",ret);
	i->sp -= 1;
//	dump(i);
	return ret;
}


void* ape_test_init(void* init_data) {
	interp_t i=(interp_t)init_data;
	init(&i);
	return i;
}

void* ape_test_result(interp_t i) {
//	printf("fetch result [%i]\n",i->stack[0]);
	return i->stack;
}

void ape_test_free(interp_t data) {
	free(data);
}

WalkDirection ape_test_default(wast_t node, interp_t this) {
	printf("unknown node %s\n",wa_op(node));
	return Done;
}


WalkDirection ape_test_number(wast_t node, interp_t this) {
/*	printf(tinyap_serialize_to_string(node));*/
//	printf("got %s\n",wa_op(wa_opd(node,0)));
	push(this,atoi(wa_op(wa_opd(node,0))));
	return Done;
}

WalkDirection ape_test_m_add(wast_t node, interp_t this) {
	int a;
	int b;
	do_walk(wa_opd(node,0),"test",this);
	do_walk(wa_opd(node,1),"test",this);
	b = pop(this);
	a = pop(this);
	push(this,a+b);

	return Done;
}
WalkDirection ape_test_m_sub(wast_t node, interp_t this) {
	int a;
	int b;
	do_walk(wa_opd(node,0),"test",this);
	do_walk(wa_opd(node,1),"test",this);
	b = pop(this);
	a = pop(this);
	push(this,a-b);

	return Done;
}
WalkDirection ape_test_m_mul(wast_t node, interp_t this) {
	int a;
	int b;
	do_walk(wa_opd(node,0),"test",this);
	do_walk(wa_opd(node,1),"test",this);
	b = pop(this);
	a = pop(this);
	push(this,a*b);

	return Done;
}
WalkDirection ape_test_m_div(wast_t node, interp_t this) {
	int a;
	int b;
	do_walk(wa_opd(node,0),"test",this);
	do_walk(wa_opd(node,1),"test",this);
	b = pop(this);
	a = pop(this);
	push(this,a/b);

	return Done;
}
WalkDirection ape_test_m_minus(wast_t node, interp_t this) {
	do_walk(wa_opd(node,0),"test",this);
	push(this,-pop(this));
	return Done;
}

WalkDirection ape_test_m_expr(wast_t node, interp_t this) {
	return Down;
}

