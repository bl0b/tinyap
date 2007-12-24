#ifndef __TINYAPE_H__
#define __TINYAPE_H__

#include "tinyap.h"

#ifndef _WAST_DEFINED
	typedef struct _walkable_ast_t* wast_t;
	#define _WAST_DEFINED
#endif

wast_t wa_new(const char* op);
void wa_del(wast_t);
wast_t wa_father(wast_t);
const char* wa_op(wast_t);
int wa_opd_count(wast_t);
wast_t wa_opd(wast_t,const unsigned int);
void wa_add(wast_t,wast_t);

wast_t make_wast(ast_node_t a);
ast_node_t make_ast(wast_t a);

typedef enum {
	Up,
	Down,
	Next,
	Done,
	Error
} WalkDirection;


typedef struct _pilot_t* pilot_t;


#endif
