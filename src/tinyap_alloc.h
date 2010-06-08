/* TinyaML
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

#ifndef __TINYAP_ALLOC_H__
#define __TINYAP_ALLOC_H__


#define BLOC_COUNT_4W  ((2048*1024)-2)
#define BLOC_COUNT_8W  ((1024*1024)-1)
#define BLOC_COUNT_16W  ((512*1024)-1)

void init_tinyap_alloc();
void term_tinyap_alloc();

void*_tinyap_alloc_4w();
void _tinyap_free_4w(void*);

void*_tinyap_alloc_8w();
void _tinyap_free_8w(void*);

void*_tinyap_alloc_16w();
void _tinyap_free_16w(void*);

#define _tinyap_alloc(__type) \
	(	sizeof(__type)<=(4*sizeof(void*))?\
	 		(__type*)_tinyap_alloc_4w():\
			sizeof(__type)<=(8*sizeof(void*))?\
				(__type*)_tinyap_alloc_8w():\
				sizeof(__type)<=16*sizeof(void*)?\
					(__type*)_tinyap_alloc_16w():\
					(__type*)malloc(sizeof(__type))\
	)
				
#define _tinyap_free(__type,__p) \
	sizeof(__type)<=(4*sizeof(void*))?\
		_tinyap_free_4w(__p):\
		sizeof(__type)<=(8*sizeof(void*))?\
			_tinyap_free_8w(__p):\
			sizeof(__type)<=(16*sizeof(void*))?\
				_tinyap_free_16w(__p):\
				free(__p)





#endif

