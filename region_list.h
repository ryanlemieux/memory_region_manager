//      Copyright (c) 2013, Ryan Lemieux
//
//      Permission to use, copy, modify, and/or distribute this software for any purpose
//      with or without fee is hereby granted, provided that the above copyright notice
//      and this permission notice appear in all copies.
//
//      THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
//      TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
//      NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
//      DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
//      IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//      CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef _REGIONLIST_H
#define _REGIONLIST_H

typedef struct REGION_NODE region_node;

struct REGION_NODE
{
	char * name;
	rsize_t size;
	rsize_t bytes_used;
	void * data;
	void * block_list;
	region_node * next;
};

region_node * insert();
boolean delete_region(const char * target);
boolean search_region(const char * target);
region_node * return_region(const char * target);
region_node * first_region();
region_node * next_region();

#endif
