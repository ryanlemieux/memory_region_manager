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

#include "regions.h"
#include "block_list.h"

void test_init();
void check(int result);
void test_typical_cases();
void test_edge_cases();
void test_special_cases();
int free_remaining_blocks(void * blocks[]);
void print_results();

static int tests_failed;

int main()
{
	test_init();

	test_typical_cases();

	test_edge_cases();

	test_special_cases();

	print_results();

	printf("\nEnd of Processing.\n");

	return EXIT_SUCCESS;
}

void test_init()
{
	printf("Initiating Test.\n");

	tests_failed = 0;
}

void check(int result)
{
	if (0 == result)
	{
		tests_failed++;
	}
}

void test_typical_cases()
{
	int size = 1024;
	char region_name[12];
	void * blocks[128];
	block_node * ptr;
	unsigned short location;
	int i;
	int j;

	printf("\n====== Begin Testing Typical Cases ======\n");

	printf("\nNon-multiple-of-8 region size and ralloc size.\n");

	// non multiple of 8 region size and ralloc size (test rounding)
	check(rinit("Non 8", 1));
	blocks[0] = ralloc(1);
	check(rsize(blocks[0]) == 8);

	rdestroy("Non 8");
	check(rchosen() == NULL);

	printf("\nMake several regions and rchoose() between them.\n");

	// Try choosing regions and chooisng after deleting regions

	check(rinit("Region A", 112));
	check(rinit("Region B", 136));
	check(rinit("Region C", 352));

	check(rchoose("Region A"));
	check(strcmp(rchosen(), "Region A") == 0);

	check(rchoose("Region C"));
	check(strcmp(rchosen(), "Region C") == 0);

	rdestroy("Region A");

	check(rchoose("Region B"));
	check(strcmp(rchosen(), "Region B") == 0);

	rdestroy("Region B");
	rdestroy("Region C");

	check(rchosen() == NULL);

	printf("\nMake a typical region and fill with 8 byte blocks\n");

	// Make 1 typical region

	check(rinit("Typical 1", size));
	check(strcmp(rchosen(), "Typical 1") == 0);

	// Fill region with 8 byte blocks

	for (i = 0; i < size / 8; i++)
	{
		blocks[i] = ralloc(1);
		check(NULL != blocks[i]);
		check(rsize(blocks[i]) == 8);
	}

	printf("\nTry to add to a full region.\n");

	// Try to add to a full region

	check(ralloc(1) == NULL);

	printf("\nDelete two blocks in the middle and make a 16 byte block.\n");

	// Delete two blocks in middle, and make 16 byte block

	check(rfree(blocks[63]));
	check(rsize(blocks[63]) == 0);
	check(rfree(blocks[64]));
	check(rsize(blocks[64]) == 0);

	blocks[63] = ralloc(16);
	check(rsize(blocks[63]) == 16);

	printf("\nDelete a block so the region has 8 bytes free, "
			"and try to add a block of size 16 (should fail).\n");

	// Delete a block so the region has 8 bytes free
	// and try to add a block of size 16 (should fail)

	check(rfree(blocks[65]));
	check(rsize(blocks[65]) == 0);

	check(ralloc(16) == NULL);

	printf("\nFree the remaining blocks in the region.\n");

	// Free the remaining blocks in the region

	check(free_remaining_blocks(blocks));

	printf("\nDestroy the region and check that the chosen region is set to NULL.\n");

	// Destroy region and check that chosen region
	// is set to NULL

	rdestroy("Typical 1");
	check(rchosen() == NULL);

	printf("\nInitialize 10 regions with different sizes, choose each region in turn, "
			"then fill all regions with 8 byte blocks, then delete all regions.\n");

	// Initialize 10 regions with different sizes

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);
		check(rinit(region_name, size * (i + 1)));
		check(strcmp(rchosen(), region_name) == 0);
	}

	// Choose each region in turn

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);
		check(rchoose(region_name));
		check(strcmp(rchosen(), region_name) == 0);
	}

	// Fill all regions with 8 byte blocks

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);
		check(rchoose(region_name));

		for (j = 0; j < size / 8; j++)
		{
			ralloc(1);
		}
	}

	// Destroy each region

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);
		check(rchoose(region_name));

		rdestroy(region_name);
		check(!rchoose(region_name));
		check(rchosen() == NULL);
	}

	printf("\nTest the first fit algorithm.\n");

	// Test first_fit

	check(rinit("Simple Region", 512));
	check(strcmp(rchosen(), "Simple Region") == 0);

	blocks[0] = ralloc(128);
	blocks[1] = ralloc(256);
	blocks[2] = ralloc(128);	// Region is now full
	check(rsize(blocks[0]) == 128);
	check(rsize(blocks[1]) == 256);
	check(rsize(blocks[2]) == 128);

	check(!ralloc(1));		// Not enough space

	check(rfree(blocks[1]));	// Free up 256 bytes

	blocks[1] = ralloc(1);		// Inserted right after block 0
	check(rsize(blocks[1]) == 8);

	check(!ralloc(256));		// Not enough contiguous space

	blocks[3] = ralloc(8);		// Inserted after block 1
	check(rsize(blocks[3]) == 8);

	blocks[4] = ralloc(240);	// Inserted right after block 3
	check(rsize(blocks[4]) == 240);

	check(!ralloc(1));		// Region is full

	check(rfree(blocks[0]));	// Free 128 bytes at beginning
	check(rfree(blocks[2]));	// Free 128 bytes at end

	blocks[0] = ralloc(32);		// Insert 32 bytes at beginning of region
	check(rsize(blocks[0]) == 32);
	blocks[2] = ralloc(112);	// Insert 112 bytes after block 4
	check(rsize(blocks[2]) == 112);

	blocks[5] = ralloc(88);		// Insert 88 bytes after block 0
	check(rsize(blocks[5]) == 88);

	blocks[6] = ralloc(16);		// Insert 16 bytes after block 2
	check(rsize(blocks[6]) == 16);

	blocks[7] = ralloc(1);		// Insert 8 bytes after block 5
	check(rsize(blocks[1]) == 8);

	check(!ralloc(1));		// Region is full

	rdestroy("Simple Region");

	printf("\nCheck to see if when a block is freed then another of "
			"the same size is created, that they have the same "
			"starting address in memory.\n");

	check(rinit("Quud", 256));
	check(ralloc(64) != NULL);
	ptr = ralloc(64);
	check(NULL != ptr);
	check(ralloc(128) != NULL);

	location = (unsigned short)ptr->block_start;

	check(rfree(ptr));
	ptr = NULL;

	ptr = ralloc(64);
	check(NULL != ptr);

	check(location == (unsigned short)ptr->block_start);

	rdestroy("Quud");
	check(rchosen() == NULL);
}

void test_edge_cases()
{
	int i;
	int j;
	char region_name[12];
	void * blocks[8191];
	block_node * ptr;

	printf("\n====== Begin Testing Edge Cases. ======\n");

	printf("\nTest rinit edge cases.\n");
	// rinit edge cases

	check(rinit("", 1));
	check(strcmp(rchosen(), "") == 0);
	rdestroy("");
	check(rchosen() == NULL);

	check(rinit("Negative", -300));
	check(strcmp(rchosen(), "Negative") == 0);
	rdestroy("Negative");
	check(rchosen() == NULL);

	check(rinit("This is an extremely long string, "
				"or at the very least, quite long indeed!", 1));

	check(strcmp(rchosen(), "This is an extremely long string, "
				"or at the very least, quite long indeed!") == 0);

	rdestroy("This is an extremely long string, "
			"or at the very least, quite long indeed!");

	check(rchosen() == NULL);

	printf("\nTry to use rchoose() when no region exists.\n");

	// rchoose when no regions exist

	check(!rchoose("Does not exist"));

	printf("\nRun rinit(-1), resulting in a max size of short (65535), "
			"then ralloc(-1) to fill the entire region with a huge block.\n");

	// rinit(-1), resulting in max size of short (65535)
	// and ralloc the entire size of the region with ralloc(-1)

	check(rinit("Foo", -1));
	check(strcmp(rchosen(), "Foo") == 0);
	ptr = ralloc(-1);
	check(ptr != NULL);
	check(rsize(ptr) == 65528);

	rdestroy("Foo");
	check(rchosen() == NULL);

	printf("\nRalloc with max size (e.g. ralloc(-1)) in a much smaller region.\n");

	// Ralloc with max size in smaller region

	check(rinit("Bar", 128));
	check(strcmp(rchosen(), "Bar") == 0);

	check(ralloc(-1) == NULL); // much too large for region

	rdestroy("Bar");
	check(rchosen() == NULL);

	printf("\nCreate 10 regions of max size (63328 bytes) and "
			"fill them with 8 byte blocks.\n");

	// Create 10 regions of max size
	// and fill them with 8 byte blocks

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);

		check(rinit(region_name, -1));
		check(strcmp(rchosen(), region_name) == 0);

		for (j = 0; j < 65528 / 8; j++)
		{
			check(ralloc(1) != NULL);
		}
	}

	printf("\nrchoose() all regions, thus including beginning and end regions "
			"(extremes). Then destroy all regions.\n");

	// rchoose all regions, thus including beginning
	// and end regions

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);

		check(rchoose(region_name));
	}

	// destroy all regions

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);

		rdestroy(region_name);
	}

	check(rchosen() == NULL);

	printf("\nMake 10,000 tiny (8 byte) regions, rchoose all regions "
			"from beginning to end, then destroy all 10,000 regions.\n");

	// Make 10,000 tiny regions

	for (i = 0; i < 10000; i++)
	{
		sprintf(region_name, "Region %d", i);

		check(rinit(region_name, 1));
	}

	// rchoose all regions, thus including beginning
	// and end regions

	for (i = 0; i < 10; i++)
	{
		sprintf(region_name, "Region %d", i);

		check(rchoose(region_name));
	}

	// destroy all 10,000 regions

	for (i = 0; i < 10000; i++)
	{
		sprintf(region_name, "Region %d", i);

		rdestroy(region_name);
	}

	check(rchosen() == NULL);

	printf("\nCreate one max size (66528 byte) region and fill it "
			"with 8 byte blocks, then rfree the blocks manually.\n");

	// Create one max size region and fill with
	// 8 byte blocks, and rfree the blocks manually

	check(rinit("rfree test", -1));

	for (i = 0; i < 65528 / 8; i++)
	{
		blocks[i] = ralloc(1);
		check(rsize(blocks[i]) == 8);
	}

	for (i = 0; i < 65528 / 8; i++)
	{
		check(rfree(blocks[i]));
	}

	rdestroy("rfree test");
	check(rchosen() == NULL);

	printf("\nTry destroying a non-existent region.\n");

	// Destroy non-existent region

	rdestroy("FooBar");

	printf("\nCreate an empty region, rchoose() it then rdestroy() it.\n");

	check(rinit("Baz", 12));
	check(rchoose("Baz"));
	check(strcmp(rchosen(), "Baz") == 0);
	rdestroy("Baz");
	check(rchosen() == NULL);

	printf("\nCreate a full region, rchoose() it then rdestroy() it.\n");

	check(rinit("Qud", 1024));
	check(ralloc(128) != NULL);
	check(ralloc(128) != NULL);
	check(ralloc(128) != NULL);
	check(ralloc(128) != NULL);
	check(ralloc(512) != NULL);
	check(ralloc(1) == NULL);	// Region is full

	check(rchoose("Qud"));
	check(strcmp(rchosen(), "Qud") == 0);
	rdestroy("Qud");
	check(rchosen() == NULL);
}

void test_special_cases()
{

#ifdef NDEBUG

	printf("\n====== Begin Testing Special Cases. ======\n");

	printf("\nRunning DNDEBUG only tests.\n");

	printf("\nCreate a duplicate region.\n");

	// create duplicate region

	check(rinit("Dupe", 16));
	check(!rinit("Dupe", 16));

	rdestroy("Dupe");

	printf("\nSend abusive parameters to functions (would normally "
			"trip assertions).\n");

	// Send abusive parameters to functions

	check(!rinit(NULL, 1));
	check(!rinit(NULL, 0));
	check(!rinit("Foo", 0));

	check(!rchoose(NULL));

	check(ralloc(0) == NULL);

	check(rsize(NULL) == 0);

	check(!rfree(NULL));

	rdestroy(NULL);

	printf("\nTry ralloc(1) when no region exists.\n");

	// Chosen region is NULL. Try:

	check(rchosen() == NULL);
	check(ralloc(1) == NULL);

#endif

	printf("\nrdump: (should print nothing)\n");

	rdump(); // Should print nothing
}

int free_remaining_blocks(void * blocks[])
{
	int i = 0;
	int success = 1;

	while (NULL != blocks[i++])
	{
		if (rsize(blocks[i - 1]) > 0)
		{
			check(rfree(blocks[i - 1]));
		}

		if (rsize(blocks[i - 1]) != 0)
		{
			success = 0;
		}
	}

	return success;
}

void print_results()
{
	printf("\nTests Failed: %d\n", tests_failed);
}

