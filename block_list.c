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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "globals.h"

typedef struct BLOCK_NODE block_node;

/* Block node */

struct BLOCK_NODE
{
	rsize_t size;
	void * block_start; 
	block_node * next;
};

static block_node * top = NULL;
static block_node * traverse_block = NULL;

block_node * get_next_block(void * block_start);
block_node * get_prev_block(void * block_start);

void * first_fit(rsize_t size, rsize_t data_size, void * data_start);

void * new_block_list()
{
	block_node * head = (block_node *)malloc(sizeof(block_node));
	head->size = 0;
	head->block_start = NULL;
	head->next = NULL;

	return head;
}

block_node * add_block(rsize_t block_size, void * list_top, rsize_t data_size, void * data_start)
{
	assert(0 == block_size % BLOCK_ALIGNMENT);
	assert(NULL != list_top);
	assert(0 < data_size);
	assert(NULL != data_start);

	top = list_top;
	block_node * new_block = (block_node *)malloc(sizeof(block_node));
	assert(NULL != new_block);
	block_node * prev_block;

	boolean success = 0 == block_size % BLOCK_ALIGNMENT
		&& NULL != list_top
		&& 0 < data_size
		&& NULL != data_start
		&& NULL != new_block;

	if (success)
	{
		new_block->size = block_size;

		if (NULL == top->next)
		{
			new_block->block_start = data_start;
			new_block->next = top->next;
			top->next = new_block;
		}
		else
		{
			new_block->block_start = first_fit(block_size, data_size, data_start);

			if (NULL != new_block->block_start)
			{
				new_block->next = get_next_block(new_block->block_start);

				prev_block = get_prev_block(new_block->block_start);
				prev_block->next = new_block;
			}
			else
			{
				free(new_block);
				new_block = NULL;
				assert(NULL == new_block);
			}
		}
	}

	return new_block;
}

block_node * find_block(void * block_start, void * list_top)
{
	assert(NULL != block_start);
	assert(NULL != list_top);

	if (NULL != block_start && NULL != list_top)
	{
		traverse_block = list_top;

		while (NULL != traverse_block && block_start != traverse_block->block_start)
		{
			traverse_block = traverse_block->next;
		}
	}

	return traverse_block;
}

boolean delete_block(block_node * target, void * list_top)
{
	boolean success = NULL != target && NULL != list_top;
	block_node * prev_block;

	if (success)
	{
		top = list_top;
		prev_block = get_prev_block(target->block_start);
		prev_block->next = target->next;

		free(target);
		target = NULL;
		success = NULL == target;
		assert(success);
	}

	return success;
}

boolean destroy_block_list(void * list_top)
{
	assert(NULL != list_top);
	top = list_top;
	boolean success = false;

	block_node * prev_block;

	if (NULL != top)
	{
		success = NULL != top->next;

		if (success)
		{
			prev_block = top->next;
			traverse_block = prev_block->next;

			prev_block->next = NULL;
			success = prev_block->next == NULL;
			assert(success);

			free(prev_block);
			prev_block = NULL;
			top->next = NULL;
			success = prev_block == NULL && top->next == NULL;
			assert(success);

			while (NULL != traverse_block)
			{
				prev_block = traverse_block;
				traverse_block = traverse_block->next;

				prev_block->next = NULL;
				success = prev_block->next == NULL;
				assert(success);

				free(prev_block);
				prev_block = NULL;
				assert(prev_block == NULL);
			}

			success = prev_block == NULL && traverse_block == NULL;
			assert(success);

			if (success)
			{
				free(top);
				top = NULL;
				success = top == NULL;
				assert(success);
			}
		}
		else
		{
			free(top);
			top = NULL;
			success = top == NULL;
			assert(success);
		}

	}

	return success;
}

void * first_fit(rsize_t block_size, rsize_t data_size, void * data_start)
{
	assert(0 == block_size % BLOCK_ALIGNMENT);
	assert(0 < block_size);
	assert(0 < data_size);
	assert(NULL != data_start);

	traverse_block = top->next;
	block_node * prev_block = top;
	void * block_start = NULL;
	unsigned int prev_size;

	boolean success = 0 == block_size % BLOCK_ALIGNMENT
		&& 0 < block_size
		&& 0 < data_size
		&& NULL != data_start;

	if (success)
	{
		prev_size = (unsigned int)data_start;

		while (traverse_block != NULL &&
				( (unsigned int)traverse_block->block_start
				  - prev_size < block_size) )
		{
			prev_block = traverse_block;
			traverse_block = traverse_block->next;
			prev_size = (unsigned int)prev_block->block_start
				+ (unsigned int)prev_block->size;
		}

		if (0 == prev_block->size) /* In this case, prev_block is the dummy node */
		{
			block_start = data_start;
		}
		else if (data_size - (prev_size - (unsigned int)data_start) >= block_size)
		{
			block_start = prev_block->block_start + prev_block->size;
		}
	}

	return block_start;
}

block_node * first_block(void * list_top)
{
	top = list_top;
	block_node * current_block = NULL;

	if (NULL != list_top)
	{
		current_block = top->next;
	}

	return current_block;
}

block_node * get_next_block(void * target)
{
	assert(NULL != target);

	if (NULL != target)
	{
		traverse_block = top;

		while (NULL != traverse_block && (traverse_block->block_start < target))
		{
			traverse_block = traverse_block->next;
		}
	}

	return traverse_block;
}

block_node * get_prev_block(void * target)
{
	assert(NULL != target);
	block_node * prev = NULL;

	if (NULL != target)
	{
		traverse_block = top;

		while (NULL != traverse_block && traverse_block->block_start < target)
		{
			prev = traverse_block;
			traverse_block = traverse_block->next;
		}

		assert(prev->block_start < target);
	}

	return prev;
}

