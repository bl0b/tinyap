#ifndef _TINYAP_PARSE_CONTEXT_H_
#define _TINYAP_PARSE_CONTEXT_H_

#include "hashtab.h"

typedef struct parse_context {
	const char* source;
	unsigned long ofs;
	/* TODO replace by ext::hash_map<> */
	struct _hashtable bows;
	unsigned long size;
	unsigned long farthest;
	struct {
		unsigned long row;
		unsigned long col;
		unsigned long last_nlofs;
		unsigned long last_ofs;
	} pos_cache;
	
}* parse_context_t;


int parse_error_column(parse_context_t t);
int parse_error_line(parse_context_t t);
void update_pos_cache(parse_context_t t);
const char* parse_error(parse_context_t t);

#endif

