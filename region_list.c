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

typedef struct REGION_NODE region_node;

/* Memory region node */

struct REGION_NODE
{
	char * name;
	rsize_t size;
	rsize_t bytes_used;
	void * data; 
	void * block_list;
	region_node * next;
};

static region_node * top = NULL;
static region_node * traverse_region = NULL;

region_node * insert()
{
	region_node * new_region = (region_node *)malloc(sizeof(region_node));
	assert(NULL != new_region);

	if (NULL != new_region)
	{
		new_region->next = top;
		top = new_region;
	}

	return new_region;
}

boolean search_region(const char * target)
{
	assert(NULL != target);

	boolean found = false;
	region_node * current_region;

	if (NULL != target)
	{
		current_region = top;

		while (NULL != current_region && NULL != current_region->name && !found)
		{
			if (strcmp(target, current_region->name) == 0)
			{
				assert(strcmp(target, current_region->name) == 0);
				found = true;
			}

			current_region = current_region->next;
		}
	}

	return found;
}

boolean delete_region(const char * target)
{
	assert(NULL != target);

	boolean success;
	boolean deleted = false;
	region_node * current_region;
	region_node * previous_region;

	if (NULL != target)
	{
		current_region = top;
		previous_region = NULL;

		success = NULL != current_region &&
			NULL != current_region->name &&
			NULL != current_region->data;

		assert(success);

		while (success && strcmp(target, current_region->name) != 0)
		{
			previous_region = current_region;
			current_region = current_region->next;

			success = NULL != current_region &&
				NULL != current_region->name &&
				NULL != current_region->data;
		}

		if (success && strcmp(target, current_region->name) == 0)
		{
			if (previous_region != NULL)
			{

				previous_region->next = current_region->next;
			}
			else
			{
				top = current_region->next;
			}

			free(current_region->name);
			current_region->name = NULL;
			success = current_region->name == NULL;
			assert(success);

			free(current_region->data);
			current_region->data = NULL;
			success = success && current_region->data == NULL;
			assert(success);

			free(current_region);
			current_region = NULL;
			success = success && current_region == NULL;
			assert(success);

			if (success)
			{
				deleted = !search_region(target);
				assert(deleted);
			}
		}
	}

	return deleted;
}

region_node * return_region(const char * target)
{
	assert(NULL != target);

	region_node * current_region;
	region_node * found = NULL;

	if (NULL != target)
	{
		current_region = top;

		while (NULL != current_region && NULL != current_region->name && NULL == found)
		{
			if (strcmp(target, current_region->name) == 0)
			{
				assert(strcmp(target, current_region->name) == 0);
				found = current_region;
				assert(NULL != found);
			}

			else
			{
				assert(strcmp(target, current_region->name) != 0);
				current_region = current_region->next;
			}
		}
	}

	return found;
}

region_node * first_region()
{
	if (NULL != top)
	{
		traverse_region = top;
	}
	else
	{
		traverse_region = NULL;
		assert(traverse_region == NULL);
	}

	return traverse_region;
}

region_node * next_region()
{
	if (NULL != traverse_region)
	{
		traverse_region = traverse_region->next;
	}

	return traverse_region;
}

