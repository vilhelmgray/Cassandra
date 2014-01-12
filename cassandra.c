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

static void combine(unsigned totCards, unsigned numCards, unsigned long long hand, unsigned long long community, struct hand best_hand, unsigned long long deck);
static struct hand determine_hand(unsigned long long hand);
static unsigned find_extrema(unsigned lump, unsigned num);
static unsigned long long get_card(unsigned long long *deck);
static unsigned get_worst_rank(unsigned long long deck, unsigned long long hand, unsigned cardsLeft);
static unsigned is_flush(unsigned chits, unsigned dhits, unsigned hhits, unsigned shits);
static unsigned is_foak(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_full_house(unsigned triplets, unsigned pairs);
static unsigned is_pair(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_straight(unsigned lump, unsigned smask);
static unsigned is_straight_flush(unsigned club, unsigned diamond, unsigned heart, unsigned spade, unsigned smask);
static unsigned is_toak(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_two_pair(unsigned pairs);
static unsigned long long parse_card(const char *card_str);

static unsigned long lose = 0;
static unsigned long split = 0;
static const unsigned long TOT_COMB= 133784560UL;
int main(void){
        unsigned long long deck = 0xFFFFFFFFFFFFF;

        printf("==Beginning Hand==\n");
        unsigned long long hand = get_card(&deck);
        hand |= get_card(&deck);

        struct hand curr_hand = determine_hand(hand);
        curr_hand.rank |= get_worst_rank(deck, hand, 5);
        if(curr_hand.category == ONE_PAIR){
                curr_hand.rank = find_extrema(curr_hand.rank, 3);
        }else{
                curr_hand.rank = find_extrema(curr_hand.rank, 5);
        }

        combine(52, 7, 0, 0, curr_hand, deck);
        printf("Ratio: %lf\n", (double)(lose+split)/TOT_COMB);
        lose = 0;
        split = 0;

        printf("==Flop==\n");
        unsigned long long flop = get_card(&deck);
        flop |= get_card(&deck);
        flop |= get_card(&deck);

        curr_hand = determine_hand(hand|flop);
        combine(52, 4, 0, flop, curr_hand, deck);

        printf("Ratio: %lf\n", (double)(lose+split)/TOT_COMB);
        lose = 0;
        split = 0;

        printf("==Turn==\n");
        unsigned long long turn = get_card(&deck);

        curr_hand = determine_hand(hand|flop|turn);
        combine(52, 3, 0, flop|turn, curr_hand, deck);

        printf("Ratio: %lf\n", (double)(lose+split)/TOT_COMB);
        lose = 0;
        split = 0;

        printf("==River==\n");
        unsigned long long river = get_card(&deck);

        curr_hand = determine_hand(hand|flop|turn|river);
        combine(52, 2, 0, flop|turn|river, curr_hand, deck);

        printf("Ratio: %lf\n", (double)(lose+split)/TOT_COMB);
        lose = 0;
        split = 0;

        return 0;
}

static void combine(unsigned totCards, unsigned numCards, unsigned long long hand, unsigned long long community, struct hand best_hand, unsigned long long deck){
        numCards--;

        unsigned long long currCard = 1ULL << numCards;
        totCards -= numCards;

        for(unsigned i = 0; i < totCards; i++){
                if(numCards){
                        combine(numCards+i, numCards, currCard|hand, community, best_hand, deck);
                }else{
                        unsigned long long currHand = currCard | hand;
                        if((currHand & deck) == currHand){
                                struct hand test_hand = determine_hand(currHand|community);
                                if(test_hand.category > best_hand.category){
                                        lose++;
                                }else if(test_hand.category == best_hand.category){
                                        if(test_hand.rank > best_hand.rank){
                                                lose++;
                                        }else if(test_hand.rank == best_hand.rank){
                                                split++;
                                        }
                                }
                        }
                }

                currCard <<= 1;
        }
}

static struct hand determine_hand(unsigned long long hand){
        enum hand_t type = HIGH_CARD;

        unsigned club = hand & 0x1FFF;
        unsigned diamond = hand>>13 & 0x1FFF;
        unsigned heart = hand>>26 & 0x1FFF;
        unsigned spade = hand>>39 & 0x1FFF;

        unsigned lump = club | diamond | heart | spade;
        unsigned rank = (lump>>1) | ((lump&0x1)<<12);
        rank = find_extrema(rank, 5);

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
                        if(type < ONE_PAIR){
                                type = ONE_PAIR;

                                unsigned normalize = lump & (~pairs);
                                normalize = (normalize>>1) | ((normalize&0x1)<<12);
                                rank = find_extrema(normalize, 3);
                        }

                        // check for three-of-a-kind
                        if(is_toak(c, d, h, s)){
                                triplets |= 1<<i;
                                if(type < THREE_OF_A_KIND){
                                        type = THREE_OF_A_KIND;

                                        unsigned normalize = lump & (~triplets);
                                        normalize = (normalize>>1) | ((normalize&0x1)<<12);
                                        rank = find_extrema(normalize, 2);
                                }

                                // check for four-of-a-kind
                                if(is_foak(c, d, h, s)){
                                        quadruplet = 1<<i;
                                        type = FOUR_OF_A_KIND;

                                        unsigned normalize = lump & (~quadruplet);
                                        normalize = (normalize>>1) | ((normalize&0x1)<<12);
                                        rank = find_extrema(normalize, 1);
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

                unsigned normalize = lump & (~pairs);
                normalize = (normalize>>1) | ((normalize&0x1)<<12);
                rank = find_extrema(normalize, 1);
        }

        struct hand best_hand = { .category = type,
                                  .quadruplet = (quadruplet>>1) | ((quadruplet&0x1)<<12),
                                  .triplets = (triplets>>1) | ((triplets&0x1)<<12),
                                  .pairs = (pairs>>1) | ((pairs&0x1)<<12),
                                  .rank = rank };

        return best_hand;
}

static unsigned find_extrema(unsigned lump, unsigned num){
        unsigned extrema = 0;

        for(int i = 12; num && i >= 0; i--){
                unsigned card = 0x1U << i;
                if(lump & card){
                        extrema |= card;
                        num--;
                }
        }

        return extrema;
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

static unsigned get_worst_rank(unsigned long long deck, unsigned long long hand, unsigned cardsLeft){
        unsigned rank = 0;

        unsigned club = deck & 0x1FFF;
        unsigned diamond = deck>>13 & 0x1FFF;
        unsigned heart = deck>>26 & 0x1FFF;
        unsigned spade = deck>>39 & 0x1FFF;
        unsigned lump = (club & diamond & heart & spade)>>1;

        unsigned c = hand & 0x1FFF;
        unsigned d = hand>>13 & 0x1FFF;
        unsigned h = hand>>26 & 0x1FFF;
        unsigned s = hand>>39 & 0x1FFF;
        unsigned handLump = (c | d | h | s)>>1;

        unsigned card = 0x1;
        do{
                unsigned isCardFree = lump & card;
                unsigned curr_hand = handLump | rank | card;

                if(isCardFree && ((curr_hand&0x1F) != 0x1F) && ((curr_hand&0x3E) != 0x3E) && ((curr_hand&0x7C) != 0x7C)){
                        rank |= card;
                        cardsLeft--;
                }

                card <<= 1;
        }while(cardsLeft);

        return rank;
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
