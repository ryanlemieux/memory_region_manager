//	Copyright (c) 2013, Ryan Lemieux
//
//	Permission to use, copy, modify, and/or distribute this software for any purpose
//	with or without fee is hereby granted, provided that the above copyright notice
//	and this permission notice appear in all copies.
//
//	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
//	TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
//	NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
//	DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
//	IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//	CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef _BLOCKLIST_H
#define _BLOCKLIST_H

typedef struct BLOCK_NODE block_node;

struct BLOCK_NODE
{
	rsize_t size;
	void * block_start;
	block_node * next;
};

void * new_block_list();
block_node * add_block(rsize_t block_size, void * list_top, rsize_t data_size, void * data_start);
block_node * find_block(void * block_start, void * list_top);
boolean delete_block(void * block_start, void * list_top);
boolean destroy_block_list(void * list_top);
block_node * first_block(void * list_top);

#endif
