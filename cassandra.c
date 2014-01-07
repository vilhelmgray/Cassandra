#include <stdio.h>

enum hand_t{
        HIGH_CARD,
        ONE_PAIR,
        TWO_PAIR,
        THREE_OF_A_KIND,
        STRAIGHT,
        FLUSH,
        FULL_HOUSE,
        FOUR_OF_A_KIND,
        STRAIGHT_FLUSH
};

struct hand{
        enum hand_t category;
        unsigned quadruplet;
        unsigned triplets;
        unsigned pairs;
        unsigned rank;
};

static struct hand determine_hand(unsigned long long hand);
static unsigned long long get_card(unsigned long long *deck);
static unsigned is_flush(unsigned chits, unsigned dhits, unsigned hhits, unsigned shits);
static unsigned is_foak(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_full_house(unsigned triplets, unsigned pairs);
static unsigned is_pair(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_straight(unsigned lump, unsigned smask);
static unsigned is_straight_flush(unsigned club, unsigned diamond, unsigned heart, unsigned spade, unsigned smask);
static unsigned is_toak(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_two_pair(unsigned pairs);
static unsigned long long parse_card(const char *card_str);

unsigned long long hc = 0;
unsigned long long op = 0;
unsigned long long tp = 0;
unsigned long long tok = 0;
unsigned long long s = 0;
unsigned long long f = 0;
unsigned long long fh = 0;
unsigned long long fok = 0;
unsigned long long sf = 0;
static void combin(unsigned long long community, unsigned long long hand, unsigned long long totCards, unsigned long long numCards, unsigned long long x, struct hand curr_hand, unsigned long long deck){
        unsigned long long y = x<<1;
        for(unsigned long long i = 0; i < totCards; i++){
                if(numCards-1){
                        combin(community, x|hand, totCards-1-i, numCards-1, y<<i, curr_hand, deck);
                }else{
                        unsigned long long z = x|hand;
                        if((z & deck) == z){
                                struct hand best_hand = determine_hand(z|community);
                                if(best_hand.category >= curr_hand.category){
                                        switch(best_hand.category){
                                                case HIGH_CARD:
                                                        hc++;
                                                        break;
                                                case ONE_PAIR:
                                                        op++;
                                                        break;
                                                case TWO_PAIR:
                                                        tp++;
                                                        break;
                                                case THREE_OF_A_KIND:
                                                        tok++;
                                                        break;
                                                case STRAIGHT:
                                                        s++;
                                                        break;
                                                case FLUSH:
                                                        f++;
                                                        break;
                                                case FULL_HOUSE:
                                                        fh++;
                                                        break;
                                                case FOUR_OF_A_KIND:
                                                        fok++;
                                                        break;
                                                case STRAIGHT_FLUSH:
                                                        sf++;
                                                        break;
                                        }
                                }
                        }

                }
                x <<= 1;
        }
}

int main(void){
        /* Each bit represents a unique card in a standard 52-card
         * playing card deck. Every 13 bits represents a suit. */
        unsigned long long deck = 0xFFFFFFFFFFFFF;
        
        struct hand curr_hand = { .category = HIGH_CARD };
        //combin(0, 52, 7, 1, curr_hand, deck);

        //printf("SF:\t%llu\n"
        //       "FOK:\t%llu\n"
        //       "FH:\t%llu\n"
        //       "F:\t%llu\n"
        //       "S:\t%llu\n"
        //       "TOK:\t%llu\n"
        //       "TP:\t%llu\n"
        //       "OP:\t%llu\n"
        //       "HC:\t%llu\n",
        //sf, fok, fh, f, s, tok, tp, op, hc);

        printf("==Beginning Hand==\n");
        unsigned long long hand = get_card(&deck);
        hand |= get_card(&deck);

        printf("==Flop==\n");
        unsigned long long flop = get_card(&deck);
        flop |= get_card(&deck);
        flop |= get_card(&deck);

        printf("==Turn==\n");
        unsigned long long turn = get_card(&deck);

        printf("==River==\n");
        unsigned long long river = get_card(&deck);

        struct hand best_hand = determine_hand(hand|flop|turn|river);
        switch(best_hand.category){
                case HIGH_CARD:
                        printf("high card\n");
                        break;
                case ONE_PAIR:
                        printf("one pair\n");
                        break;
                case TWO_PAIR:
                        printf("two pair\n");
                        break;
                case THREE_OF_A_KIND:
                        printf("three of a kind\n");
                        break;
                case STRAIGHT:
                        printf("straight\n");
                        break;
                case FLUSH:
                        printf("flush\n");
                        break;
                case FULL_HOUSE:
                        printf("full house\n");
                        break;
                case FOUR_OF_A_KIND:
                        printf("four of a kind\n");
                        break;
                case STRAIGHT_FLUSH:
                        printf("straight flush\n");
                        break;
        }

        combin(flop|turn|river, 0, 52, 2, 1, best_hand, deck);

        printf("SF:\t%llu\n"
               "FOK:\t%llu\n"
               "FH:\t%llu\n"
               "F:\t%llu\n"
               "S:\t%llu\n"
               "TOK:\t%llu\n"
               "TP:\t%llu\n"
               "OP:\t%llu\n"
               "HC:\t%llu\n",
        sf, fok, fh, f, s, tok, tp, op, hc);

        return 0;
}

static struct hand determine_hand(unsigned long long hand){
        enum hand_t type = HIGH_CARD;

        unsigned club = hand & 0x1FFF;
        unsigned diamond = hand>>13 & 0x1FFF;
        unsigned heart = hand>>26 & 0x1FFF;
        unsigned spade = hand>>39 & 0x1FFF;

        unsigned lump = club | diamond | heart | spade;
        unsigned rank = ((lump&0x1)<<13) | lump;

        unsigned pairs = 0;
        unsigned triplets = 0;
        unsigned quadruplet = 0;

        unsigned chits = 0;
        unsigned dhits = 0;
        unsigned hhits = 0;
        unsigned shits = 0;

        for(unsigned i = 0; i < 13; i++){
                if(i < 10){
                        unsigned smask = 0xF<<i | 1<<((i+4)%13);

                        // check for straight
                        if(type <= STRAIGHT && is_straight(lump, smask)){
                                rank = i;
                                type = STRAIGHT;
                        }
                        // check for straight-flush
                        if(type >= STRAIGHT && is_straight_flush(club, diamond, heart, spade, smask)){
                                rank = i;
                                type = STRAIGHT_FLUSH;
                        }
                }

                unsigned c = club & 1<<i;
                unsigned d = diamond & 1<<i;
                unsigned h = heart & 1<<i;
                unsigned s = spade & 1<<i;

                // check for pair
                if(type <= FULL_HOUSE && is_pair(c, d, h, s)){
                        pairs |= 1<<i;
                        type = (type < ONE_PAIR) ? ONE_PAIR : type;

                        // check for three-of-a-kind
                        if(is_toak(c, d, h, s)){
                                triplets |= 1<<i;
                                type = (type < THREE_OF_A_KIND) ? THREE_OF_A_KIND : type;

                                // check for four-of-a-kind
                                if(is_foak(c, d, h, s)){
                                        quadruplet = 1<<i;
                                        type = FOUR_OF_A_KIND;
                                }
                        }
                }

                // check for flush
                chits += (c) ? 1 : 0;
                dhits += (d) ? 1 : 0;
                hhits += (h) ? 1 : 0;
                shits += (s) ? 1 : 0;
                unsigned suit = 0;
                if(type <= FLUSH && (suit = is_flush(chits, dhits, hhits, shits))){
                        switch(suit){
                                case 1:
                                        suit = club;
                                        break;
                                case 2:
                                        suit = diamond;
                                        break;
                                case 3:
                                        suit = heart;
                                        break;
                                case 4:
                                        suit = spade;
                                        break;
                        }
                        rank = ((suit&0x1)<<13) | suit;

                        type = FLUSH;
                }
        }

        // check for full-house
        if(type < FULL_HOUSE && is_full_house(triplets, pairs)){
                type = FULL_HOUSE;
        }

        // check for two pair
        if(type < TWO_PAIR && is_two_pair(pairs)){
                type = TWO_PAIR;
        }

        struct hand best_hand = { .category = type,
                                  .quadruplet = quadruplet,
                                  .triplets = triplets,
                                  .pairs = pairs,
                                  .rank = rank };

        return best_hand;
}

static unsigned long long get_card(unsigned long long *deck){
        unsigned long long card;
        do{
                char buffer[8];
                printf("Input card: ");
                if(!fgets(buffer, sizeof(buffer), stdin)){
                        fprintf(stderr, "ERROR: Problem reading input.\n");
                        continue;
                }

                card = parse_card(buffer);
                if(!card){
                        fprintf(stderr, "ERROR: Unable to parse card.\n");
                        continue;
                }else if(!(card & *deck)){
                        printf("This card has already been selected, be choose another.\n");
                        continue;
                }

                break;
        }while(1);

        *deck &= ~card;
        return card;
}

static unsigned is_flush(unsigned chits, unsigned dhits, unsigned hhits, unsigned shits){
        if(chits == 5){
                return 1;
        }
        if(dhits == 5){
                return 2;
        }
        if(hhits == 5){
                return 3;
        }
        if(shits == 5){
                return 4;
        }

        return 0;
}

static unsigned is_foak(unsigned c, unsigned d, unsigned h, unsigned s){
        if(c & d & h & s){
                return 1;
        }

        return 0;
}

static unsigned is_full_house(unsigned triplets, unsigned pairs){
        if(triplets){
                for(unsigned i = 0; i < 13; i++){
                        if(triplets & 1<<i && pairs & ~(1U<<i)){
                                return 1;
                        }
                }
        }

        return 0;
}

static unsigned is_pair(unsigned c, unsigned d, unsigned h, unsigned s){
        if(c & d || c & h || c & s || d & h || d & s || h & s){
                return 1;
        }

        return 0;
}

static unsigned is_straight(unsigned lump, unsigned smask){
        if((lump & smask) == smask){
                return 1;
        }
        
        return 0;
}

static unsigned is_straight_flush(unsigned club, unsigned diamond, unsigned heart, unsigned spade, unsigned smask){
        if((club & smask) == smask || (diamond & smask) == smask || (heart & smask) == smask || (spade & smask) == smask){
                return 1;
        }

        return 0;
}

static unsigned is_toak(unsigned c, unsigned d, unsigned h, unsigned s){
        if(c & d & h || c & d & s || c & h & s || d & h & s){
                return 1;
        }

        return 0;
}

static unsigned is_two_pair(unsigned pairs){
        if(pairs){
                for(unsigned i = 0; i < 12; i++){
                        if((pairs>>i) & 0x1 && pairs>>(i+1)){
                                return 1;
                        }
                }
        }

        return 0;
}

static unsigned long long parse_card(const char *card_str){
        unsigned long long card = 0x0;

        unsigned rank;
        char suit;
        int retval = sscanf(card_str, "%u %c", &rank, &suit);

        if(retval < 2 || !rank || rank > 13){
                return 0;
        }

        card = 1<<(rank-1);

        switch(suit){
                case 'c':
                        break;
                case 'd':
                        card <<= 13;
                        break;
                case 'h':
                        card <<= 26;
                        break;
                case 's':
                        card <<= 39;
                        break;
                default:
                        return 0;
        }

        return card;
}
