#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int check (struct Card* hand)
{
    size_t ranks = strlen (rank),
           pairs_count = 0,
           last_pair_rank = 0;

    for (size_t i = 0; i < ranks; ++i) {
        size_t card_count = 0;

        for (struct Card* p = hand; p->rank != 0; p++) {
            if (p->rank == rank[i]) {
                ++card_count;
            }
        }

        if (card_count == 2) {
            ++pairs_count;
            last_pair_rank = i;
        }
    }

    if (pairs_count == 2) {
        return last_pair_rank;
    } else {
        return -1;
    }
}
