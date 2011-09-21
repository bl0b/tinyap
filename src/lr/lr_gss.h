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
					/*return (0xb4dc0d3 * (1+n.O)) ^ (((char*)n.S) - ((char*)n.P));*/
					return 0xb4dc0d3 * (1+n.O) * (1+(off_t)(n.S));
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
				Ast ast;					/* production */
				std::list<node*> preds;
				node* reduction_end;
				node() : id(0, 0, 0), active(0), ast(0), preds(), reduction_end(0) {}
				void add_pred(node*p) {
					preds.push_back(p);
					/*p->link = pred;*/
					/*pred = p;*/
				}
				~node() {
					/*std::cerr << "delete stack node; ast=" << ast << std::endl;*/
					/*delete_node(ast);*/
				}
				/*ast_node_t get_state_ast() const {*/
					/*return preds.front()->ast;*/
				/*}*/
				/*node* get_prev_state() const {*/
					/*return preds.front()->preds.front();*/
				/*}*/
				std::string to_dot() const {
					std::stringstream ret;
					if(ast) {
						/* ast node */
						ret << 'n' << (ptrdiff_t)this << " [shape=ellipse,label=\"";
						escaper(ret) << ast;
						ret << "\"];" << std::endl;
					} else {
						/* state node */
						ret << 'n' << (ptrdiff_t)this << " [shape=rectangle,label=\"{s" << id.S->id << ", " << id.O << "}\"];" << std::endl;
					}
					std::list<node*>::const_iterator i = preds.begin(), j=preds.end();
					std::stringstream tmp;
					grammar::visitors::debugger d(tmp);
					for(;i!=j;++i) {
						ret << 'n' << (ptrdiff_t)*i << " -> n" << (ptrdiff_t)this;
						if(ast) {
							ret << "[label=\"";
							if(id.P) {
								id.P->accept(&d);
							} else {
								ret << "START";
							}
							escaper(ret) << tmp.str();
							ret << "\"]";
						}
						ret << ";" << std::endl;
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
					int i;
					for(i=1;i<NODE_SEGMENT_SIZE;++i) {
						/*mypool[i].pred = &mypool[i-1];*/
						mypool[i].preds.push_back(&mypool[i-1]);
						/*mypool[i].link = NULL;*/
						mypool[i].ast = NULL;
						mypool[i].id = node_id(0,0,0);
					}
					gss_ram_size += sizeof(node_segment);
				}
				node* first() { return mypool+NODE_SEGMENT_SIZE-1; }
				friend std::ostream& operator<<(std::ostream&o, node_segment& s) {
					for(int i=0;i<NODE_SEGMENT_SIZE;++i) {
						if(s.mypool[i].ast || s.mypool[i].id.S) {
							o << s.mypool[i].to_dot();
						}
					}
					return o;
				};
				~node_segment() { gss_ram_size -= sizeof(node_segment); }
			};

			struct node_allocator {
				unsigned int alloc_count;
				node* free_;
				std::list<node_segment*> segments;
				node_allocator() : alloc_count(0), free_(NULL), segments() {}
				~node_allocator() {
					std::list<node_segment*>::iterator i, j=segments.end();
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
						++gss_allocs;
						--gss_reallocs;
					}
					++gss_reallocs;
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
					++gss_frees;
					n->preds.push_back(free_);
					/*delete_node(n->ast);*/
					free_ = n;
					--alloc_count;
				}
				friend std::ostream& operator<<(std::ostream&o, node_allocator& s) {
					for(std::list<node_segment*>::iterator i=s.segments.begin(), j=s.segments.end();i!=j;++i) {
						o << **i;
					}
					return o;
				};
			} alloc;

			node root;
			/*std::queue<node*> active;*/
			std::list<node*> active;
			item initial;
			Ast accepted;
			unsigned int size;

			~gss() {
				/*delete_node(accepted);*/
			}


			struct reduction_data {
				node* red_end;
				node* tail;
				item i;
				unsigned int offset;
				ast_node_t accum;
				reduction_data(node* re, node* t, item& i_, unsigned int o, ast_node_t a)
					: red_end(re), tail(t), i(i_), offset(o), accum(a)
				{
					/*debug();*/
				}
				reduction_data(const reduction_data& rd)
					: red_end(rd.red_end), tail(rd.tail), i(rd.i), offset(rd.offset), accum(rd.accum)
				{
					/*debug();*/
				}

				void debug() const {
					std::clog << "red_dat(" << red_end << ", " << tail << ", " << i << ", " << offset << ", " << accum << ')' << std::endl;
				}
			};

			std::list<reduction_data> pending_reductions;

			node* alloc_node(node_id& id) {
				node* ret = alloc.alloc();
				ret->id = id;
				/*delete_node(ret->ast);*/
				ret->ast = NULL;
				ret->preds.clear();
				return ret;
			}

			void free_node(node*n) { alloc.free(n); }

			gss(item ini, unsigned int sz) : alloc(), root(), active(), initial(ini), accepted(0), size(sz) {}

			friend std::ostream& operator<<(std::ostream& o, gss& g) {
				return o << g.alloc;
			}

			/* TODO : merge only when shifting a non-terminal ? */

			node* shift(node* p, grammar::item::base* producer, state* s, ast_node_t ast, unsigned int offset, node* red_end) {
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

			typedef grammar::rule::internal::append appender;

			/* in stack reduction, i is always at_end() to start with */
			node* reduce_all(node* n, item i, unsigned int offset) {
				/*unsigned int offset = n->id.O;*/
				/*appender append;*/
				/*ast_node_t ast;*/
				/*node* backup = n;*/

				/* this is a helper routine which helps track the item path back to the start of the rule in the stack */
				struct for_each_ {
					/* track path to head, depth-first. */
					unsigned int offset;
					/*ast_node_t& accepted;*/
					unsigned int count;
					node* red_end;
					gss* stack;
					bool drop_empty;
					unsigned int size;
					void rec_path(node* tail, ast_node_t accum, lr::item i) {
						std::list<node*>::iterator a = tail->preds.begin(), b = tail->preds.end();
						if(tail->id.S) {
							//std::clog << "on state node " << i << " accum = " << accum << std::endl/* << tail->id.S << std::endl*/;
							/* on state node */
							if(i.at_start()) {
								//std::clog << "found reductible path ! " << accum << std::endl;
								stack->commit_reduction(red_end, tail, i, offset, accum);
								/*delete_node(accum);*/
								return;
							} else {
								--i;
							}
						/*} else if(*i) {*/
						} else {
							/* on AST node */
							ast_node_t tmp = accum;
							accum = grammar::rule::internal::append()(tail->ast, accum, drop_empty);
							/*accum->raw.ref++;*/
							/*delete_node(tmp);*/
						}
						for(;a!=b;++a) {
								rec_path(*a, accum, i);
						}
					}

					unsigned int operator()(node* tail, lr::item i) {
						//std::clog << "trying to reduce " << i << std::endl;
						drop_empty = !i.rule()->keep_empty();
						red_end = tail;
						rec_path(tail, NULL, i);
						//std::clog << "done trying to reduce " << i << std::endl;
						return count;
					}

					for_each_(unsigned int ofs, unsigned int sz, ast_node_t ac, gss* s) : offset(ofs), /*accepted(ac),*/ count(0), stack(s), size(sz) {}
				} reduce_each_path(offset, size, accepted, this);
				

				/* special treatment for epsilon rules */
				if(i.at_start()) {
					++gss_reduces;
					/* epsilon rule : reduce ZERO node, and shift from current node. */
					grammar::visitors::reducer red(PRODUCTION_OK_BUT_EMPTY, offset);
					Ast output = red(((grammar::rule::base*)i.rule()));
					/*std::clog << "epsilon reduction" << std::endl;*/
					/*output->raw.ref++;*/
					return shift(n, grammar::item::token::Nt::instance(i.rule()->tag()), n->id.S->transitions.from_stack[i.rule()->tag()], output, offset, n);
				}

				reduce_each_path(n, i);
				return NULL;
			}


			void init_reductions() {
				pending_reductions.clear();
			}

			void commit_reduction(node* red_end, node* tail, item i, unsigned int offset, ast_node_t accum) {
				reduction_data rd(red_end, tail, i, offset, accum);
				pending_reductions.push_back(rd);
			}

			void flush_reductions() {
				std::list<reduction_data>::iterator i, j;
				for(i=pending_reductions.begin(), j=pending_reductions.end();i!=j;++i) {
					do_reduction((*i).red_end, (*i).tail, (*i).i, (*i).offset, (*i).accum);
				}
				pending_reductions.clear();
			}

			void do_reduction(node* red_end, node* tail, item i, unsigned int offset, ast_node_t accum) {
				const grammar::rule::base* R = i.rule();
				if(initial==i) {
					if(offset != size) {
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
						ast_node_t output = red.process((grammar::rule::base*)R);
						ast_node_t old = accepted;
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
					ast_node_t redast = red((grammar::rule::base*)R);
					/*redast->raw.ref++;*/
					grammar::item::base* nt = grammar::item::token::Nt::instance(R->tag());
					std::cerr << "Reducing " << accum << " into " << redast << std::endl;
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



			void activate(node*n) {
				if(!n->active) {
					/*std::clog << "activating node @" << n << " S=" << n->id.S->id << " O=" << n->id.O << " P=" << n->id.P << std::endl;*/
					n->active = true;
					active.push_back(n);
				}
			}

			struct node_less {
				bool operator()(const node*a, const node*b) const {
					return 	a->id.O < b->id.O
							||
							(a->id.O == b->id.O  &&  a->id.S < b->id.S)
							||
							(a->id.P && b->id.P && a->id.P->is_less(b->id.P));
					/*return a->S==b->S && a->O==b->O;*/
				}
			};

			void merge_active() {
				std::set<node*, node_less> uniq;
				std::list<node*>::iterator i, j, k;
				i=active.begin();
				j=active.end();
				while(i!=j) {
					std::pair<std::set<node*, node_less>::iterator, bool> isun = uniq.insert(*i);
					if(!isun.second) {
						/*std::cerr << "MERGING STATE !" << std::endl;*/
						k=i;
						++i;
						(*isun.first)->preds.insert((*isun.first)->preds.end(),
								(*k)->preds.begin(), (*k)->preds.end());
						/*if((*k)->ast) {*/
							/*delete_node((*k)->ast);*/
						/*}*/
						(*k)->ast = NULL;
						(*k)->id.S = NULL;
						active.erase(k);
					} else {
						++i;
					}
				}
			}

			node* consume_active() {
				node* ret = active.front();
				ret->active = false;
				active.pop_front();
				return ret;
			}
	};
}

#endif

