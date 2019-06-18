/* tinyap : this is not yet another parser.
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

#ifndef TINYAP_TRIE_H
#define TINYAP_TRIE_H

#include "tinyap_alloc.h"
#include "string_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _trie_node_t* trie_t;

trie_t trie_new();
void trie_dump(trie_t t, int indent);
void trie_free(trie_t t);
unsigned long trie_match(trie_t t, const char*s);
unsigned long trie_match_prefix(trie_t t, const char*s);
void trie_insert(trie_t t, const char*s);

#ifdef __cplusplus
}
#endif


#endif

