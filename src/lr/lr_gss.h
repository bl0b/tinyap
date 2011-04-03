#ifndef _LR_GSS_H_
#define _LR_GSS_H_

namespace lr {
	template<typename OFS>
		struct escaping_ostream { OFS&_; escaping_ostream(OFS&o) : _(o) {} };

	template <typename OFS, typename X>
		const escaping_ostream<OFS>& operator << (const escaping_ostream<OFS>& eo, const X& x) {
			std::stringstream tmp;
			tmp << x;
			std::string ts = tmp.str();
			std::string::const_iterator i=ts.begin(), j=ts.end();
			for(;i!=j;++i) {
				if(*i=='"') {
					eo._ << '\\';
				}
				eo._ << *i;
			}
			return eo;
		};

	template <typename OFS>
		escaping_ostream<OFS> escaper(OFS&_) {
			return escaping_ostream<OFS>(_);
		}

	class gss {
		public:
			/* node merging happens on "producer P led to state S at offset O" identity */
			struct node_id {
				grammar::item::base* P;
				unsigned int O;
				state* S;
				node_id(grammar::item::base*b, unsigned int i, state*s)
					: P(b), O(i), S(s)
				{
					/*std::clog << "gss::node::id("<<O<<','<<((void*)S)<<')'<<std::endl;*/
				}
				node_id& operator=(const node_id&n) {
					P = n.P;
					O = n.O;
					S = n.S;
					/*std::clog << "gss::node::id=("<<O<<','<<((void*)S)<<')'<<std::endl;*/
					return *this;
				}
			};

			struct node_id_hash {
				size_t operator()(const node_id&n) const {
					return (0xb4dc0d3 * (1+n.O)) ^ (((char*)n.S) - ((char*)n.P));
					/*return (0xb4dc0d3 * (1+n->O)) * (1+(off_t)(n->S));*/
				}
			};

			struct node_id_compare {
				size_t operator()(const node_id&a, const node_id&b) const {
					return a.S==b.S && a.O==b.O && (a.P==b.P || (a.P && b.P && a.P->is_same(b.P)));
					/*return a->S==b->S && a->O==b->O;*/
				}
			};

			struct node {
				node_id id;
				bool active;
				ast_node_t ast;					/* production */
				//node* pred;						/* first ancestor */
				//node* link;						/* next in ancestor list of some node */
				std::list<node*> preds;
				node* reduction_end;
				node() : id(0, 0, 0), active(0), ast(0), preds(), reduction_end(0) {}
				void add_pred(node*p) {
					preds.push_back(p);
					/*p->link = pred;*/
					/*pred = p;*/
				}
				ast_node_t get_state_ast() const {
					return preds.front()->ast;
				}
				node* get_prev_state() const {
					return preds.front()->preds.front();
				}
				std::string to_dot() const {
					std::stringstream ret;
					if(ast) {
						/* ast node */
						ret << 'n' << (ptrdiff_t)this << " [shape=ellipse,label=\"";
						escaper(ret) << ast;
						ret << "\"];" << std::endl;
					} else {
						/* state node */
						ret << 'n' << (ptrdiff_t)this << " [shape=rectangle,label=\"{" << id.S->id << "}\"];" << std::endl;
					}
					std::list<node*>::const_iterator i = preds.begin(), j=preds.end();
					std::stringstream tmp;
					grammar::visitors::debugger d(tmp);
					for(;i!=j;++i) {
						ret << 'n' << (ptrdiff_t)*i << " -> n" << (ptrdiff_t)this << "[label=\"";
						id.P->accept(&d);
						escaper(ret) << tmp.str();
						ret << "\"];" << std::endl;
					}
					if(reduction_end) {
						ret << 'n' << (ptrdiff_t) reduction_end << " -> n" << (ptrdiff_t)this << " [arrowhead=odot];" << std::endl;
					}
					return ret.str().c_str();
				}

			};

			struct node_segment {
#				define NODE_SEGMENT_SIZE 1024
				node mypool[NODE_SEGMENT_SIZE];
				node_segment(node_segment* prev=NULL) {
					mypool[0].preds.push_back(prev?prev->mypool:NULL);
					/*mypool[0].pred = prev?prev->mypool:NULL;*/
					for(int i=1;i<NODE_SEGMENT_SIZE;++i) {
						/*mypool[i].pred = &mypool[i-1];*/
						mypool[i].preds.push_back(&mypool[i-1]);
						/*mypool[i].link = NULL;*/
						mypool[i].ast = NULL;
						mypool[i].id = node_id(0,0,0);
					}
				}
				node* first() { return mypool; }
				friend std::ostream& operator<<(std::ostream&o, node_segment& s) {
					for(int i=0;i<NODE_SEGMENT_SIZE;++i) {
						if(s.mypool[i].ast || s.mypool[i].id.S) {
							o << s.mypool[i].to_dot();
						}
					}
					return o;
				};
			};

			struct node_allocator {
				unsigned int alloc_count;
				node* free_;
				std::vector<node_segment*> segments;
				node_allocator() : alloc_count(0), free_(NULL), segments() {}
				~node_allocator() {
					std::vector<node_segment*>::iterator i, j=segments.end();
					for(i=segments.begin();i!=j;++i) {
						delete *i;
					}
				}
				node* alloc() {
					/*static unsigned int target=0;*/
					node* ret;
					if(!free_) {
						node_segment* ns = new node_segment();
						free_ = ns->first();
						segments.push_back(ns);
					}
					ret = free_;
					free_ = free_->preds.front();
					++alloc_count;
					/*if(alloc_count>target) {*/
						/*std::clog << "gss:: alloc'ed " << alloc_count << " nodes" << std::endl;*/
						/*target+=100;*/
					/*}*/
					return ret;
				}
				void free(node* n) {
					n->preds.push_back(free_);
					free_ = n;
					--alloc_count;
				}
				friend std::ostream& operator<<(std::ostream&o, node_allocator& s) {
					for(unsigned int i=0;i<s.segments.size();++i) {
						o << *s.segments[i];
					}
					return o;
				};
			} alloc;

			node root;
			std::queue<node*> active;
			item initial;
			ast_node_t accepted;
			unsigned int size;

			typedef ext::hash_map<node_id, node*, node_id_hash, node_id_compare> gss_node_registry;

			gss_node_registry registry;

			/* FIXME without unicity, it's a tree stack
			 * FIXME but to make a proper gss, the ast and states must be pushed separately
			 * FIXME (merging is on state and not on output, and stack reduction is to follow all possible paths)
			 * FIXME is there a way to cleanly postpone reductions ?
			 *
			 * maybe just merging the ASTs and re-using the FOREST flag is enough
			 * or use a std::vector
			 */
			node* alloc_node(node_id& id) {
				node* ret =
					//NULL;
					id.S ? registry[id] : NULL;
					//registry[&id];
				if(!ret) {
					ret = alloc.alloc();
					ret->id = id;
					ret->ast = NULL;
					/*ret->pred = ret->link = NULL;*/
					ret->preds.clear();
					/*if(id.S) { registry[&ret->id] = ret; }*/
				/*} else {*/
					/*std::clog << "gss::merging node!" << std::endl;*/
				}
				return ret;
			}

			void free_node(node*n) { registry.erase(n->id); alloc.free(n); }

			gss(item ini, unsigned int sz) : alloc(), root(), active(), initial(ini), accepted(0), size(sz), registry() {}

			friend std::ostream& operator<<(std::ostream& o, gss& g) {
				return o << g.alloc;
			}

			/* TODO : merge only when shifting a non-terminal ? */

			node* shift(node* p, grammar::item::base* producer, state* s, ast_node_t ast, unsigned int offset, node* red_end) {
				if(!p) {
					p=&root;
				}
				/* ast node has no id */
				node_id noid(producer, offset, NULL);
				/* state node has id */
				node_id id(producer, offset, s);
				/* push ast node */
				node* n = alloc_node(noid);
				n->ast = ast;
				/*std::cout << "pushed ast node with " << ast << std::endl;*/
				n->add_pred(p);
				p = n;
				/* push state node */
				n = alloc_node(id);
				n->ast = NULL;
				n->add_pred(p);
				n->reduction_end = red_end;
				/*std::cout << "pushed state node with #" << s->id << std::endl;*/
				activate(n);
				return n;
			}

			typedef grammar::rule::internal::append appender;

			/* in stack reduction, i is always at_end() to start with */
			node* reduce(node* n, item i, unsigned int offset) {
				/*unsigned int offset = n->id.O;*/
				/*appender append;*/
				ast_node_t ast;
				node* backup = n;

#if 0
				/* this is a helper routine which helps track the item path back to the start of the rule in the stack */
				struct {
					bool operator()(node*&p, const grammar::item::base*k) {
						grammar::visitors::lr_item_debugger d;
						/*if(p->link) {*/
							/*std::cerr << "NOT A TREE STACK ?" << std::endl;*/
						/*}*/
						while(p) {
							/*std::clog << "comparing "; ((grammar::item::base*)p->id.P)->accept(&d); std::clog << " and "; ((grammar::item::base*)k)->accept(&d); std::clog << std::endl;*/
							if(k->is_same(p->id.P)) {
								/*std::clog << " found matching predecessor !" << std::endl;*/
								return true;
							}
							/*std::clog << " didn't find any matching predecessor." << std::endl;*/
							p = p->link;
						}
						return false;
					}
				} find_pred;
#endif
				

				/* special treatment for epsilon rules */
				if(i.at_start()) {
					/* epsilon rule : reduce ZERO node, and shift from current node. */
					grammar::visitors::reducer red(PRODUCTION_OK_BUT_EMPTY, offset);
					ast_node_t output = red(((grammar::rule::base*)i.rule()));
					/*std::clog << "epsilon reduction" << std::endl;*/
					return shift(n, grammar::item::gc(new grammar::item::token::Nt(i.rule()->tag())), n->id.S->transitions.from_stack[i.rule()->tag()], output, offset, backup);
				}

				/* now for the general case, track down the start of the rule in stack */
				const grammar::rule::base* R = i.rule();
				const bool drop_empty = !R->keep_empty();
				--i;
				ast = n->get_state_ast();//n->pred->ast;
				/*std::clog << ast << std::endl;*/
				/*while(find_pred(n, *i) && (!i.at_start())) {*/
				while(!i.at_start()) {
					--i;
					/*n = n->pred->pred;*/
					n = n->get_prev_state();
					/*std::clog << ((void*)n->preds.front()) << ' ' << ((void*)n->get_state_ast()) << std::endl;*/
					ast = grammar::rule::internal::append()(n->get_state_ast(), ast, drop_empty);
					/*std::clog << ast << std::endl;*/
				}
				const bool must_cleanup = ast!=backup->get_state_ast();

				/* if we reach start, we good */
				//if(i.at_start()/*&&find_pred(n, *i)*/) {	/* if tracking failed, n is NULL, because tracking failed BECAUSE n became NULL. */
				bool reduction_ok =
					i.at_start()
						&&
					(n->id.P	? n->id.P->is_same(*i)
					 			: !strcmp(i.rule()->tag(), "_start"));		/* FIXME : define the rule name globally */
				if(reduction_ok) {	/* if tracking failed, n is NULL, because tracking failed BECAUSE n became NULL. */
					if(initial==i) {
						if(offset != size) {
							std::clog << "can't accept at offset " << offset << " because size is " << size << std::endl;
							if(must_cleanup) { delete_node(ast); }
							return NULL;
						}
						/* accept */
						char* acc_ast = (char*)ast_serialize_to_string(ast); std::clog << "ACCEPT ! " << acc_ast << " @" << ((void*)ast) << std::endl; free(acc_ast);
						if(ast) {
							grammar::visitors::reducer red(ast, offset);
							ast_node_t output = red.process((grammar::rule::base*)R);
							accepted = grammar::rule::internal::append()(output, accepted);
						}
						return NULL;
					} else {
						grammar::visitors::reducer red(ast, offset);
						ast_node_t redast = red((grammar::rule::base*)R);
						grammar::item::base* nt = grammar::item::gc(new grammar::item::token::Nt(R->tag()));
						/*std::clog << "Reducing " << ast << " into " << redast << std::endl;*/
						state* Sprime = n->get_prev_state()->id.S->transitions.from_stack[R->tag()];
						if(!Sprime) {
							std::clog << "I don't know where to go with a ";
							grammar::visitors::lr_item_debugger d;
							((grammar::rule::base*)R)->accept(&d);
							std::clog << " on top of stack from state : " << std::endl << n->get_prev_state()->id.S << std::endl;
							/*throw "coin";*/
							if(must_cleanup) { delete_node(ast); }
							return NULL;
						}
						/*delete_node(ast);*/
						return shift(n->get_prev_state(), nt, Sprime, redast, offset, backup);
					}
				} else {
					if(must_cleanup) { delete_node(ast); }
					return NULL;
				}
			}
			void activate(node*n) {
				if(!n->active) {
					/*std::clog << "activating node " << n << std::endl;*/
					n->active = true;
					active.push(n);
				}
			}
			node* consume_active() {
				node* ret = active.front();
				ret->active = false;
				active.pop();
				return ret;
			}
	};
}

#endif

