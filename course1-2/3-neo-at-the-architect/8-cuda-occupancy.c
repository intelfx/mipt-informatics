#include <stdio.h>
#include <math.h>

const unsigned total_threads = 1536,
    total_memory = 49152,
    total_regs = 32768,
    total_blocks = 8,
    granularity_threads_per_block = 32,
    granularity_memory_per_block = 128, granularity_regs_per_thread = 2;

unsigned round_up_to_granularity(unsigned value, unsigned granularity)
{
	unsigned tmp = value + granularity - 1;
	return tmp - (tmp % granularity);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MIN4(a, b, c, d) MIN(MIN(MIN(a, b), c), d)

int main()
{
	freopen("cuda.in", "r", stdin);
	freopen("cuda.out", "w", stdout);

	unsigned block_threads, thread_regs, block_memory;
	scanf("%u %u %u", &block_threads, &thread_regs, &block_memory);

	unsigned allocated_threads_per_block =
	    round_up_to_granularity(block_threads,
				    granularity_threads_per_block),
	    allocated_regs_per_block =
	    round_up_to_granularity(thread_regs,
				    granularity_regs_per_thread) *
	    allocated_threads_per_block, allocated_memory_per_block =
	    round_up_to_granularity(block_memory, granularity_memory_per_block);

	unsigned allowed_blocks_by_threads =
	    allocated_threads_per_block ? (total_threads /
					   allocated_threads_per_block) :
	    total_blocks, allowed_blocks_by_regs =
	    allocated_regs_per_block ? (total_regs /
					allocated_regs_per_block) :
	    total_blocks, allowed_blocks_by_memory =
	    allocated_memory_per_block ? (total_memory /
					  allocated_memory_per_block) :
	    total_blocks;

	unsigned allowed_blocks = MIN4(allowed_blocks_by_threads,
				       allowed_blocks_by_regs,
				       allowed_blocks_by_memory,
				       total_blocks);

	unsigned used_threads = allowed_blocks * block_threads;

	long useful_load =
	    lroundl(used_threads * 100 / (long double)total_threads);
	printf("%ld\n", useful_load);
}
