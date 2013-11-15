#include <stdio.h>

int main(void){
        /* Each bit represents a unique card in a standard 52-card
         * playing card deck. Every 13 bits represents a suit. */
        unsigned long long deck = 0xFFFFFFFFFFFFF;

        unsigned long long river = 0x0;
        unsigned long long hand = 0x0;
        hand |= 1;
        hand |= 1<<13;
        hand |= 1UL<<26;
        hand |= 1ULL<<39;

        unsigned club = hand & 0x1FFF;
        unsigned diamond = hand>>13 & 0x1FFF;
        unsigned heart = hand>>26 & 0x1FFF;
        unsigned spade = hand>>39 & 0x1FFF;

        /* check for combinations of pairs */
        /* (1<<i | 1<<(i+13) | 1<<(i+13*2) | etc.. */
        /* if true then check for three of a kind */
        /* if true then check for four of a kind */
        for(unsigned i = 0; i < 13; i++){
                unsigned c = club & 1<<i;
                unsigned d = diamond & 1<<i;
                unsigned h = heart & 1<<i;
                unsigned s = spade & 1<<i;

                // check for pair
                if(c & d || c & h || c & s || d & h || d & s || h & s){
                        // check for three-of-a-kind
                        if(c & d & h || c & d & s || c & h & s || d & h & s){
                                //check for four-of-a-kind
                                if(c & d & h & s){
                                        printf("four-of-a-kind\n");
                                }

                                printf("three-of-a-kind\n");
                        }

                        printf("pair\n");
                }
        }

	return 0;
}
