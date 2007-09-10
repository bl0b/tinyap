#ifndef __TINYAP_WALKABLE_AST_H__
#define __TINYAP_WALKABLE_AST_H__

#include "../ast.h"

typedef struct _walkable_ast_t* wast_t;

wast_t wa_new(const char* op);
void wa_del(wast_t);
wast_t wa_father(wast_t);
const char* wa_op(wast_t);
int wa_opd_count(wast_t);
wast_t wa_opd(wast_t,const unsigned int);
void wa_add(wast_t,wast_t);

wast_t make_wast(ast_node_t a);

#endif

