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

        unsigned lump = club | diamond | heart | spade;

        unsigned pairs = 0;
        unsigned triples = 0;

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
                                // check for four-of-a-kind
                                if(c & d & h & s){
                                        printf("four-of-a-kind\n");
                                }

                                triples |= 1<<i;
                                printf("three-of-a-kind\n");
                        }

                        pairs |= 1<<i;
                        printf("pair\n");
                }

                // check for straight
                if(i < 10){
                        unsigned smask = 0xF<<i | 1<<((i+4)%13);
                        if((lump & smask) == smask){
                                // check for straight-flush
                                if(club & smask == smask || diamond & smask == smask || heart & smask == smask || spade & smask == smask){
                                        if(i == 9){
                                                printf("royal-flush\n");
                                        }
                                        printf("straight-flush\n");
                                }

                                printf("straight\n");
                        }
                }

                static unsigned chits = 0;
                static unsigned dhits = 0;
                static unsigned hhits = 0;
                static unsigned shits = 0;
                chits += (c) ? 1 : 0;
                dhits += (d) ? 1 : 0;
                hhits += (h) ? 1 : 0;
                shits += (s) ? 1 : 0;
                if(chits == 5 || dhits == 5 || hhits == 5 || shits == 5){
                        printf("flush\n");
                }
        }

        // check for full-house
        if(triples){
                for(unsigned i = 0; i < 13; i++){
                        if(triples & 1<<i && pairs & ~(1U<<i)){
                                printf("full-house\n");
                        }
                }
        }

        // check for two pairs
        if(pairs){
                for(unsigned i = 0; i < 12; i++){
                        if((pairs>>i) & 0x1 && pairs>>(i+1)){
                               printf("two pairs\n"); 
                        }
                }
        }

	return 0;
}
