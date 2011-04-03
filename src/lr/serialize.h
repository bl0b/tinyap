/* Tinya(J)P : this is not yet another (Java) parser.
 * Copyright (C) 2007 Damien Leroux
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _TINYAP_SERIALIZE_H_
#define _TINYAP_SERIALIZE_H_
#include "config.h"

#include <string.h>

struct _esc_chr {
	char escaped;
	char unescaped;
	int  context;
};
extern const struct _esc_chr escape_characters[];

/* unescape first character in *src, put it in *dest, and advance pointers */
static inline void unescape_chr(char**src,char**dest, int context, int delim) {
	/* index to search for character escaping combination */
	int i, c;
	char ret=**src;
	if(!ret) {
		**dest=0;
		return;
	}
	*src+=1;
	if(ret=='\\') {
		if(**src==delim) {
			ret=**src;
			*src+=1;
		} else {
			i=0;
			/* there may be an escaped character following */
			while(escape_characters[i].escaped!=0&&**src!=escape_characters[i].escaped) {
				i+=1;
			}
			c = escape_characters[i].context;
			if(escape_characters[i].escaped && (context&c)==context) {
				/* if we do have an escaped character, swallow it before returning */
	//			debug_writeln("unescaping \\%c",escape_characters[i].escaped);
				ret=escape_characters[i].unescaped;
				*src+=1;
			}
		}
	}
	/* either ret is not \ or there's no valid escaped character following, thus we push raw ret in dest */
	**dest=ret;
	*dest+=1;
}

static inline void unescape_chr_l(char**src,char**dest, int context, const char* long_delim) {
	/* index to search for character escaping combination */
	int i, c;
	size_t ldlen = strlen(long_delim);
	char ret=**src;
	if(!ret) {
		**dest=0;
		return;
	}
	*src+=1;
	if(ret=='\\') {
		if(!strncmp(*src, long_delim, ldlen)) {
			/*ret=**src;*/
			/**src+=1;*/
			strcpy(*dest, long_delim);
			*src+=1+ldlen;
			*dest+=ldlen;
			return;
		} else {
			i=0;
			/* there may be an escaped character following */
			while(escape_characters[i].escaped!=0&&**src!=escape_characters[i].escaped) {
				i+=1;
			}
			c = escape_characters[i].context;
			if(escape_characters[i].escaped && (context&c)==context) {
				/* if we do have an escaped character, swallow it before returning */
	//			debug_writeln("unescaping \\%c",escape_characters[i].escaped);
				ret=escape_characters[i].unescaped;
				*src+=1;
			}
		}
	}
	/* either ret is not \ or there's no valid escaped character following, thus we push raw ret in dest */
	**dest=ret;
	*dest+=1;
}



/*static inside_lisp = 0;*/


/* escape first character in *src, put it in *dest, and advance pointers */
static inline void escape_chr(char**src,int(*func)(int,void*),void*param, int context) {
	/* index to search for character escaping combination */
	int i=0, c;
	char ret=**src;
	if(!ret) {
		func(0,param);
		return;
	}

	/* search for an escaping combination for this character */
	while(escape_characters[i].unescaped!=0&&**src!=escape_characters[i].unescaped) {
		i+=1;
	}

	/*#define lisp_ok(_ec_) ( ((_ec_).lisp & inside_lisp) == (_ec_).lisp )*/
	/*if(escape_characters[i].unescaped!=0 && lisp_ok(escape_characters[i])) {*/

	c = escape_characters[i].context;
	if(escape_characters[i].unescaped!=0 && (context&c)==context) {
		/*dump_contexts(escape_characters[i].unescaped, c, context);*/
		/* have to escape character, two bytes will be pushed onto *dest */
		func('\\',param);
		func(escape_characters[i].escaped,param);
	} else {
		/* push raw **src into **dest */
		func(ret,param);
	}
	/* now dest is all set up, advance source */
	*src+=1;
}





#endif

