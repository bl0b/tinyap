#include "lr.h"
#include "static_init.h"

namespace lr {
	gss::node* gss::shift(node* p, grammar::item::base* producer, state* s, ast_node_t ast, unsigned int offset, node* red_end) {
				if(!p) {
					p=&root;
				}
				++gss_shifts;
				/* ast node has no id */
				node_id noid(producer, offset, NULL);
				/* state node has id */
				node_id id(producer, offset, s);
				/* push ast node */
				node* n = alloc_node(noid);
				n->ast = ast;
				/*if(ast) {*/
					/*ast->raw.ref++;*/
				/*}*/
				n->reduction_end = red_end;
				/*std::clog << "pushed ast node with " << ast << std::endl;*/
				n->add_pred(p);
				p = n;
				/* push state node */
				n = alloc_node(id);
				n->ast = NULL;
				n->add_pred(p);
				/*std::clog << "pushed state node with #" << s->id << std::endl;*/
				activate(n);
				return n;
			}

	void gss::do_reduction(node* red_end, node* tail, item i, unsigned int offset, ast_node_t accum) {
				const grammar::rule::base* R = i.rule();
				if(initial==i) {
					if(!(accept_partial || offset == size)) {
						/*std::clog << "can't accept at offset " << offset << " because size is " << size << std::endl;*/
						/*delete_node(accum);*/
						return;
					}
					/* accept */
#if 1
					std::clog << "ACCEPT ! " << accum << " @" << ((void*)accum) << std::endl;
#endif
					if(accum) {
						grammar::visitors::reducer red(accum, offset);
						Ast output = red.process((grammar::rule::base*)R);
						Ast old = accepted;
						accepted = grammar::rule::internal::append()(output, accepted);
						/*accepted->raw.ref++;*/
						/*delete_node(old);*/
						/*delete_node(accum);*/
						/*delete_node(output);*/
					}
					return;
				} else {
					state* Sprime = tail->id.S->transitions.from_stack[R->tag()];
					if(!Sprime) {
						std::clog << std::endl << "I don't know where to go with a ";
						grammar::visitors::lr_item_debugger d;
						((grammar::rule::base*)R)->accept(&d);
						std::clog << " on top of stack from state : " << std::endl << tail->id.S << std::endl;
						/*throw "coin";*/
						/*delete_node(accum);*/
						return;
					}
					/* do reduction NOW ! */
					grammar::visitors::reducer red(accum, offset);
					Ast redast = red((grammar::rule::base*)R);
					/*redast->raw.ref++;*/
					grammar::item::base* nt = grammar::item::token::Nt::instance(R->tag());
					/*std::clog << "Reducing " << accum << " into " << redast << std::endl;*/
					shift(tail, nt, Sprime, redast, offset, red_end);
					/*if(redast!=accum) {*/
						/*delete_node(redast);*/
					/*}*/
					/*delete_node(accum);*/
					/*delete_node(redast);*/
					/*std::clog << "one reduction done" << std::endl;*/
					return;
				}
			}
}
