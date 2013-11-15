#include <stdio.h>

int main(void){
        /* Each bit represents a unique card in a standard 52-card
         * playing card deck. Every 13 bits represents a suit. */
        unsigned long long deck = 0xFFFFFFFFFFFFF;

        unsigned long long river = 0x0;
        unsigned long long hand = 0x0;

        /* check for combinations of pairs */
        /* (1<<i | 1<<(i+13) | 1<<(i+13*2) | etc.. */
        /* if true then check for three of a kind */
        /* if true then check for four of a kind */
        for(unsigned i = 0; u < 13; i++){
                unsigned spades = hand & 1ULL<<i;
                unsigned hearts = hand & 1ULL<<(i+13);
                unsigned clubs = hand & 1ULL<<(i+26);
                unsigned diamonds = hand & 1ULL<<(i+39);
        }

	return 0;
}
