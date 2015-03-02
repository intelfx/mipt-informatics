#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define XX_PAGE_SHIFT (32 - 6)
#define XX_PAGE_MASK ((1 << XX_PAGE_SHIFT) - 1)

int main()
{
	size_t page_count;
	uint32_t *page_table, logical_address;

	scanf("%zu", &page_count);
	page_table = malloc(sizeof(*page_table) * page_count);

	for (size_t i = 0; i < page_count; ++i) {
		scanf("%" SCNx32, &page_table[i]);
	}

	scanf("%" SCNx32, &logical_address);

	size_t page_idx = logical_address >> XX_PAGE_SHIFT;
	if (page_idx < page_count) {
		uint32_t page_offset = logical_address & XX_PAGE_MASK;
		printf("%" PRIx32 "\n", page_table[page_idx] + page_offset);
	} else {
		printf("error\n");
	}
}
