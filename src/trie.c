#include <malloc.h>
#include "tinyap_alloc.h"
#include "string_registry.h"

typedef struct _trie_node_t* trie_t;

struct _trie_node_t {
	char* radix;
	trie_t next;
	trie_t follow;
	int is_leaf;
};


void trie_dump(trie_t t, int indent) {
	printf("%*.*s%p %s %s\n", indent*3, indent*3, "", t, t->radix, t->is_leaf?"LEAF":"");
	if(t->follow) {
		trie_dump(t->follow, indent+1);
	}
	if(t->next) {
		trie_dump(t->next, indent);
	}
}

static inline trie_t trie_alloc() {
	return tinyap_alloc(struct _trie_node_t);
}

trie_t trie_new() {
	trie_t ret = trie_alloc();
	ret->radix=NULL;
	ret->next=NULL;
	ret->follow=NULL;
	ret->is_leaf=1;
	return ret;
}

void trie_free(trie_t t) {
	trie_t tmp;
	while(t->follow) {
		tmp = t->follow;
		t->follow = tmp->follow;
		trie_free(tmp);
	}
	while(t->next) {
		tmp = t->next;
		t->next = tmp->next;
		trie_free(tmp);
	}
	tinyap_free(struct _trie_node_t, t);
}


unsigned long match_prefix(const char*s1, const char*s2) {
	const char*base=s1;
	while(*s1&&*s1==*s2) { s1+=1; s2+=1; }
	return s1-base;
}


unsigned long trie_match(trie_t t, const char*s) {
	trie_t tmp=t->follow;
	const char* backup = s;
	unsigned long ret=0;
	int l;
	while(tmp) {
		if((l=match_prefix(tmp->radix, s))) {
			if(tmp->radix[l]==0) {
				if(s[l]==0&&tmp->is_leaf) {
					ret = s+l-backup;
				}
				tmp=tmp->follow;
				s+=l;
			} else {
				return 0;
			}
		} else {
			tmp=tmp->next;
		}
	}
	return ret;
}


void trie_insert(trie_t t, const char*s) {
	trie_t cur=t->follow, prev=t;
	int l;

	if(!s) { return; }

	while(*s&&cur) {
		while(cur&&!(l=match_prefix(cur->radix, s))) {
			cur=cur->next;
		}
		if(!cur) {
			trie_t n = trie_alloc();
			n->follow=NULL;
			n->next=prev->follow;
			prev->follow=n;
			n->radix=_strdup(s);
			n->is_leaf=1;
			return;
		} else if(cur->radix[l]==0) {
			s+=l;
			prev=cur;
			cur=cur->follow;
		} else {
			trie_t split = trie_alloc();
			trie_t n = trie_alloc();
			n->follow=NULL;
			n->radix=_strdup(s+l);
			n->next=cur->follow;
			n->is_leaf=1;
			split->is_leaf=cur->is_leaf;
			split->radix = _strdup(cur->radix+l);
			split->next=n;
			split->follow=cur->follow;
			cur->radix[l]=0;
			/*cur->next=NULL;*/
			cur->is_leaf=0;
			cur->follow=split;
			return;
		}
	}
	if(*s&&!cur) {
		trie_t n = trie_alloc();
		n->radix=_strdup(s);
		n->next=prev->follow;
		n->follow=NULL;
		n->is_leaf=1;
		prev->follow=n;
	}
}


int main(int argc, char**argv) {
	trie_t t = trie_new();
	trie_insert(t, "dog");
	trie_insert(t, "dawg");
	trie_insert(t, "car");
	trie_insert(t, "category");
	trie_insert(t, "dawg");
	trie_dump(t, 0);
	printf("dog => %li\n", trie_match(t, "dog"));
	trie_free(t);
	return 0;
}


