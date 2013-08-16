#include "parse_context.h"
#include <string.h>


const char* parse_error(parse_context_t t) {
	static char err_buf[4096];
    static char err_cursor[2048];
    char* cursor = err_cursor;
	size_t last_nlofs=0;
	size_t next_nlofs=0;
	size_t tab_adjust=0;
	/*const char* expected;*/

	t->ofs=t->farthest;
	update_pos_cache(t);
	last_nlofs=t->ofs-t->pos_cache.col+1;
	
	next_nlofs=last_nlofs;
	while(t->source[next_nlofs]&&t->source[next_nlofs]!='\n') {
        if (t->farthest > next_nlofs) {
    		if(t->source[next_nlofs]=='\t') {
                /* snap to tabsize 8 */
    			/*tab_adjust+=8-((next_nlofs-last_nlofs)&7);*/
                *cursor = '\t';
            } else {
                *cursor = ' ';
    		}
            cursor += 1;
        }
		next_nlofs+=1;
	}
    *cursor = 0;

	err_buf[0]=0;

    printf("Got cursor line <%s>\n", err_cursor);

	sprintf(err_buf+strlen(err_buf),"%*.*s\n%s^",
		(int)(next_nlofs-last_nlofs),
		(int)(next_nlofs-last_nlofs),
		t->source+last_nlofs,
        err_cursor
	);

	/*sprintf(err_buf+strlen(err_buf),"%*.*s\n%*.*s^",*/
		/*(int)(next_nlofs-last_nlofs),*/
		/*(int)(next_nlofs-last_nlofs),*/
		/*t->source+last_nlofs,*/
		/*(int)(t->farthest-last_nlofs+tab_adjust),*/
		/*(int)(t->farthest-last_nlofs+tab_adjust),*/
		/*""*/
	/*);*/

	/*free((char*)expected);*/

	return err_buf;
}

int parse_error_column(parse_context_t t) {
	t->ofs=t->farthest;
	update_pos_cache(t);
	return t->pos_cache.col;
}

int parse_error_line(parse_context_t t) {
	t->ofs=t->farthest;
	update_pos_cache(t);
	return t->pos_cache.row;
}


void update_pos_cache(parse_context_t t) {
	int ln=t->pos_cache.row;		/* line number */
	size_t ofs=t->pos_cache.last_ofs;
	size_t end=t->ofs;
	size_t last_nlofs=t->pos_cache.last_nlofs;

	if(ofs==end) {
		return;
	}

	if(t->ofs<t->pos_cache.last_ofs) {
		while(ofs>end) {
			if(t->source[ofs]=='\n') {
				--ln;
			}
			--ofs;
		}
		while(ofs>0&&t->source[ofs]!='\n') {
			--ofs;
		}
		last_nlofs=ofs+(!!ofs);	/* don't skip character if at start of buffer */
	} else {
		while(ofs<end) {
			if(t->source[ofs]=='\n') {
				++ln;
				last_nlofs=ofs+1;
			}
			++ofs;
		}

	}

	if(ln>t->pos_cache.row) {
		t->pos_cache.row=ln;
		t->pos_cache.col=1+end-last_nlofs;
		t->pos_cache.last_ofs=end;
		t->pos_cache.last_nlofs=last_nlofs;
		/*node_cache_clean(t->cache, &t->pos_cache);*/
	}
	t->pos_cache.row=ln;
	t->pos_cache.col=1+end-last_nlofs;
	t->pos_cache.last_ofs=end;
	t->pos_cache.last_nlofs=last_nlofs;
	/*printf("update_pos_cache now ofs=%i/%i (%i lines)\n", end, t->size, t->pos_cache.row);*/
}


