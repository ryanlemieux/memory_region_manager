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
#include <assert.h>
#include <string.h>

#include "globals.h"
#include "region_list.h"
#include "block_list.h"

#define RSIZE_T_MAX 65528
#define ONE_HUNDRED 100

static region_node * chosen_region = NULL;

rsize_t round_to_block(rsize_t input);
void zero_block_data(block_node * block);

boolean rinit(const char * region_name, rsize_t region_size)
{
	assert(NULL != region_name);
	assert(!search_region(region_name));
	assert(0 < region_size);

	rsize_t rounded_size;
	boolean success = false;

	if (NULL != region_name && !search_region(region_name) && 0 < region_size)
	{
		chosen_region = insert();
		assert(NULL != chosen_region);

		if (NULL != chosen_region)
		{
			rounded_size = round_to_block(region_size);

			chosen_region->name = (char *)malloc(strlen(region_name) + 1);
			assert(NULL != chosen_region->name);

			chosen_region->size = rounded_size;

			chosen_region->bytes_used = 0;

			chosen_region->data = malloc(rounded_size);
			assert(NULL != chosen_region->data);

			chosen_region->block_list = new_block_list();

			if (NULL != chosen_region->name && NULL != chosen_region->data
					&& NULL != chosen_region->block_list)
			{
				strcpy(chosen_region->name, region_name);
				assert(strcmp(region_name, chosen_region->name) == 0);
				success = true;
			}
			else
			{
				if (NULL != chosen_region->block_list)
				{
					destroy_block_list(chosen_region->block_list);
					assert(NULL == chosen_region->block_list);
				}

				free(chosen_region->name);
				free(chosen_region->data);
				free(chosen_region);
				chosen_region = NULL;
				assert(NULL == chosen_region);
			}
		}
	}

	return success;
}

boolean rchoose(const char * region_name)
{
	assert(NULL != region_name);

	region_node * chosen_one;
	boolean success = false;

	if (NULL != region_name)
	{
		chosen_one = return_region(region_name);

		if (NULL != chosen_one)
		{
			chosen_region = chosen_one;
			success = true;
		}
	}

	return success;
}

const char * rchosen()
{
	const char * chosen_name = NULL;

	if (chosen_region != NULL)
	{
		assert(NULL != chosen_region->name);

		if (NULL != chosen_region->name)
		{
			chosen_name = chosen_region->name;
			assert(strcmp(chosen_name, chosen_region->name) == 0);
		}
	}

	return chosen_name;
}

void * ralloc(rsize_t block_size)
{
	assert(0 < block_size);
	assert(NULL != chosen_region);
	assert(NULL != chosen_region->block_list);
	assert(NULL != chosen_region->data);

	boolean success = 0 < block_size
		&& NULL != chosen_region
		&& NULL != chosen_region->block_list
		&& NULL != chosen_region->data;

	block_node * new_block = NULL;
	void * block_data_start = NULL;
	rsize_t rounded_size = round_to_block(block_size);

	if (success && rounded_size <= (chosen_region->size - chosen_region->bytes_used))
	{
		new_block = add_block(rounded_size, chosen_region->block_list,
				chosen_region->size, chosen_region->data);

		if (NULL != new_block)
		{
			chosen_region->bytes_used += rounded_size;

			zero_block_data(new_block);

			block_data_start = new_block->block_start;
		}
	}

	return block_data_start;
}

rsize_t rsize(void * block_ptr)
{
	assert(NULL != chosen_region);
	block_node * search_block;
	rsize_t block_size = 0;

	if (NULL != block_ptr && NULL != chosen_region)
	{
		search_block = find_block(block_ptr, chosen_region->block_list);

		if (NULL != search_block)
		{
			assert(0 < search_block->size);

			if (0 < search_block->size)
			{
				block_size = search_block->size;
			}
		}
	}

	return block_size;
}

boolean rfree(void * block_ptr)
{
	assert(NULL != block_ptr);
	assert(NULL != chosen_region);
	boolean success = false;
	region_node * current_region;
	block_node * target = NULL;
	int block_size;

	if (NULL != chosen_region && NULL != block_ptr)
	{
		current_region = first_region();

		while (NULL != current_region && NULL == target)
		{
			target = find_block(block_ptr, current_region->block_list);
			current_region = next_region();
		}

		if (NULL != target)
		{
			block_size = target->size;
		}

		success = delete_block(target, chosen_region->block_list);

		if (success)
		{
			chosen_region->bytes_used -= block_size;
		}
	}

	return success;
}

void rdestroy(const char * region_name)
{
	assert(NULL != region_name);
	boolean success;
	region_node * target_region;

	if (NULL != region_name)
	{
		target_region = return_region(region_name);

		if (NULL != target_region)
		{
			success = strcmp(region_name, target_region->name) == 0;

			if (success)
			{
				success = destroy_block_list(target_region->block_list);
				assert(success);

				if (success)
				{
					success = delete_region(region_name);
					assert(success);

					if (success)
					{
						if (target_region == chosen_region)
						{
							chosen_region = NULL;
							assert(chosen_region == NULL);
						}

						target_region = NULL;
						assert(target_region == NULL);
					}
				}
			}
		}
	}
}

void rdump()
{
	region_node * current_region = first_region();
	block_node * current_block;
	float percent;

	while (NULL != current_region)
	{
		printf("REGION NAME: \t%s\n", current_region->name);
		printf("SIZE (BYTES): \t%d\n", current_region->size);
		printf("USED (BYTES): \t%d\n", current_region->bytes_used);

		percent = ONE_HUNDRED - (((float)current_region->bytes_used /
					(float)current_region->size) * ONE_HUNDRED);

		printf("FREE SPACE: \t%.2f %%\n\n", percent);

		current_block = first_block(current_region->block_list);

		if (NULL != current_block)
		{
			printf("\tBLOCKS:\n\n");
		}

		while (NULL != current_block)
		{
			printf("\t\t%p\n", current_block->block_start);
			printf("\t\t%d bytes\n\n", current_block->size);

			current_block = current_block->next;
		}

		printf("\n");

		current_region = next_region();
	}
}

rsize_t round_to_block(rsize_t input)
{
	assert(0 < input);
	rsize_t rounded = input;

	if (RSIZE_T_MAX < input)
	{
		rounded = RSIZE_T_MAX;
	}
	else if (0 < input && input % BLOCK_ALIGNMENT != 0)
	{
		rounded += BLOCK_ALIGNMENT - (input % BLOCK_ALIGNMENT);
	}

	assert(rounded % BLOCK_ALIGNMENT == 0);

	return rounded;
}

void zero_block_data(block_node * block)
{
	assert(NULL != block);
	unsigned char * ptr;

	if (NULL != block)
	{
		for (ptr = block->block_start; (void *)ptr < block->block_start + block->size; ptr++)
		{
			*ptr = 0;
			assert(*ptr == 0);
		}
	}
}

