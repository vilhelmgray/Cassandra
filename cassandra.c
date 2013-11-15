#include <stdio.h>

int main(void){
        /* Each bit represents a unique card in a standard 52-card
         * playing card deck. Every 13 bits represents a suit. */
        unsigned long long deck = 0xFFFFFFFFFFFFF;

        unsigned long long river = 0x0;
        unsigned long long hand = 0x0;
        hand |= 1ULL;
        hand |= 1ULL<<13;
        hand |= 1ULL<<26;
        hand |= 1ULL<<39;

        /* check for combinations of pairs */
        /* (1<<i | 1<<(i+13) | 1<<(i+13*2) | etc.. */
        /* if true then check for three of a kind */
        /* if true then check for four of a kind */
        for(unsigned i = 0; i < 13; i++){
                unsigned club = hand & 1ULL<<i;
                unsigned diamond = (hand & 1ULL<<(i+13)) >> 13;
                unsigned heart = (hand & 1ULL<<(i+26)) >> 26;
                unsigned spade = (hand & 1ULL<<(i+39)) >> 39;

                if(club&diamond || club&heart || club&spade || diamond&heart || diamond&spade || heart&spade){
                        printf("pair\n");
                }
        }

	return 0;
}
