/*
 * Copyright (C) 2014 William Breathitt Gray
 *
 * This file is part of Cassandra.
 *
 * Cassandra is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cassandra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cassandra.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <math.h>

#include "counter.h"
#include "lookup.h"

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

static void betting_round(unsigned *bankroll, unsigned *pot, const struct win_counter counter, const unsigned long COMB);
static struct hand determine_hand(unsigned long long hand);
static void determine_win_counter(struct win_counter *const counter, const unsigned long long DECK, const unsigned long long COMMUNITY, const unsigned long long HAND, const unsigned NUM_CARDS);
static unsigned evaluate_opponents(unsigned numPlayers);
static unsigned find_extrema(unsigned lump, unsigned num);
static unsigned long long get_card(unsigned long long *deck);
static unsigned is_flush(unsigned chits, unsigned dhits, unsigned hhits, unsigned shits);
static unsigned is_foak(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_full_house(unsigned *const triplets, unsigned *const pairs);
static unsigned is_pair(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_straight(unsigned lump, unsigned smask);
static unsigned is_straight_flush(unsigned club, unsigned diamond, unsigned heart, unsigned spade, unsigned smask);
static unsigned is_toak(unsigned c, unsigned d, unsigned h, unsigned s);
static unsigned is_two_pair(unsigned *const pairs);
static double kelly(double b, double c, double p);
static unsigned long long parse_card(const char *card_str);
static void showdown(struct win_counter *const counter, const struct hand best_hand, const unsigned long long COMMUNITY, const unsigned long long DECK);

int main(void){
        unsigned long long deck = 0xFFFFFFFFFFFFF;
        
        unsigned bankroll;
        do{
                char buffer[8];
                printf("Bankroll: ");
                if(!fgets(buffer, sizeof(buffer), stdin)){
                        fprintf(stderr, "ERROR: Problem reading input.\n");
                        continue;
                }

                int retval = sscanf(buffer, "%u", &bankroll);
                if(retval < 1){
                        fprintf(stderr, "ERROR: Unable to parse input.\n");
                        continue;
                }

                break;
        }while(1);

        unsigned pot;
        do{
                char buffer[8];
                printf("Pot: ");
                if(!fgets(buffer, sizeof(buffer), stdin)){
                        fprintf(stderr, "ERROR: Problem reading input.\n");
                        continue;
                }

                int retval = sscanf(buffer, "%u", &pot);
                if(retval < 1){
                        fprintf(stderr, "ERROR: Unable to parse input.\n");
                        continue;
                }

                break;
        }while(1);

        printf("==Hole Cards==\n");
        unsigned long long hand = get_card(&deck);
        hand |= get_card(&deck);

        const size_t NUM_PROB = sizeof(begin_prob)/sizeof(*begin_prob);
        size_t h = 0;
        for(h = 0; h < NUM_PROB; h++){
                if(begin_prob[h].hand == hand){
                        break;
                }
        }

        const unsigned long HAND_COMB = 2097572400UL;
        betting_round(&bankroll, &pot, begin_prob[h].counter, HAND_COMB);
        printf("Pot: %u\n", pot);

        printf("==Flop==\n");
        unsigned long long flop = get_card(&deck);
        flop |= get_card(&deck);
        flop |= get_card(&deck);

        struct win_counter counter = {0};
        determine_win_counter(&counter, deck, flop, hand, 2);

        const unsigned long FLOP_COMB = 1070190UL;
        betting_round(&bankroll, &pot, counter, FLOP_COMB);
        printf("Pot: %u\n", pot);

        printf("==Turn==\n");
        unsigned long long turn = get_card(&deck);

        counter.split = 0;
        counter.win = 0;
        determine_win_counter(&counter, deck, flop|turn, hand, 1);

        const unsigned TURN_COMB = 45540U;
        betting_round(&bankroll, &pot, counter, TURN_COMB);
        printf("Pot: %u\n", pot);

        printf("==River==\n");
        unsigned long long river = get_card(&deck);

        counter.split = 0;
        counter.win = 0;
        struct hand best_hand = determine_hand(hand|flop|turn|river);
        showdown(&counter, best_hand, flop|turn|river, deck);

        const unsigned RIVER_COMB = 990;
        betting_round(&bankroll, &pot, counter, RIVER_COMB);
        printf("Pot: %u\n", pot);

        return 0;
}

static void betting_round(unsigned *bankroll, unsigned *pot, const struct win_counter counter, const unsigned long COMB){
        do{
                unsigned numOpponents;
                do{
                        char buffer[8];
                        printf("Number of Opponents: ");
                        if(!fgets(buffer, sizeof(buffer), stdin)){
                                fprintf(stderr, "ERROR: Problem reading input.\n");
                                continue;
                        }

                        int retval = sscanf(buffer, "%u", &numOpponents);
                        if(retval < 1){
                                fprintf(stderr, "ERROR: Unable to parse input.\n");
                                continue;
                        }

                        break;
                }while(1);

                if(!numOpponents){
                        break;
                }

                double c = (double)(*pot)/(*bankroll);
                double p = 1;
                for(unsigned i = 0; i < numOpponents; i++){
                        p *= (double)(counter.win - i)/(COMB - i);
                }
                printf("Win probability: %lf\n", p);
                double k_bet = floor(*bankroll * kelly(numOpponents, c, p));
                printf("You should bet: %.0lf\n", k_bet);

                unsigned bet;
                do{
                        char buffer[8];
                        printf("Bet Ammount: ");
                        if(!fgets(buffer, sizeof(buffer), stdin)){
                                fprintf(stderr, "ERROR: Problem reading input.\n");
                                continue;
                        }

                        int retval = sscanf(buffer, "%u", &bet);
                        if(retval < 1){
                                fprintf(stderr, "ERROR: Unable to parse input.\n");
                                continue;
                        }

                        if(bet > *bankroll){
                                fprintf(stderr, "ERROR: Bet size is greater than bankroll.\n");
                                continue;
                        }

                        break;
                }while(1);

                if(!bet){
                        char buffer[8];
                        printf("Check? [Y/n]: ");
                        if(!fgets(buffer, sizeof(buffer), stdin)){
                                fprintf(stderr, "ERROR: Problem reading input.\n");
                                continue;
                        }

                        char answer;
                        int retval = sscanf(buffer, "%c", &answer);
                        if(retval < 1){
                                fprintf(stderr, "ERROR: Unable to parse input.\n");
                                continue;
                        }

                        if(answer == 'N' || answer == 'n'){
                                break;
                        }
                }

                *bankroll -= bet;
                *pot += bet;

                *pot += evaluate_opponents(numOpponents);
        }while(1);
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
        if(type < FULL_HOUSE && is_full_house(&triplets, &pairs)){
                type = FULL_HOUSE;
        }

        // check for two pair
        if(type < TWO_PAIR && is_two_pair(&pairs)){
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

static void determine_win_counter(struct win_counter *const counter, const unsigned long long DECK, const unsigned long long COMMUNITY, const unsigned long long HAND, const unsigned NUM_CARDS){
        const unsigned TOT_CARDS = 52;
        const unsigned long long BOUNDARY = 1ULL << TOT_CARDS;

        unsigned long long cards = (1ULL << NUM_CARDS) - 1;
        do{
                if((cards & DECK) == cards){
                        struct hand best_hand = determine_hand(HAND|COMMUNITY|cards);
                        showdown(counter, best_hand, COMMUNITY|cards, DECK & (~cards));
                }

                unsigned long long pos_mask = 1;
                while(!(cards & pos_mask)){
                        pos_mask <<= 1;
                }

                unsigned card_num = 0;
                while(cards & pos_mask){
                        card_num++;
                        pos_mask <<= 1;
                }

                cards |= pos_mask;
                cards &= ~(pos_mask - 1);
                cards |= (1ULL<<card_num-1) - 1;
        }while(cards < BOUNDARY);
}

static unsigned evaluate_opponents(unsigned numPlayers){
        unsigned pot = 0;

        while(numPlayers){
                char buffer[8];
                printf("Player #%d: ", numPlayers);
                if(!fgets(buffer, sizeof(buffer), stdin)){
                        fprintf(stderr, "ERROR: Problem reading input.\n");
                        continue;
                }

                unsigned bet;
                int retval = sscanf(buffer, "%u", &bet);
                if(retval < 1){
                        fprintf(stderr, "ERROR: Unable to parse input.\n");
                        continue;
                }

                pot += bet;
                numPlayers--;
        }

        return pot;
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
                        printf("ATTENTION: This card has already been selected, please choose another.\n");
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

static unsigned is_full_house(unsigned *const triplets, unsigned *const pairs){
        if(*triplets){
                for(int i = 12; i >= 0; i--){
                        const unsigned TRIPLET = 1 << i;
                        if(*triplets & TRIPLET){
                                const unsigned OTHER_PAIRS = *pairs & ~(TRIPLET);
                                if(!OTHER_PAIRS){
                                        return 0;
                                }

                                *triplets = TRIPLET;

                                unsigned j = i;
                                while(!(OTHER_PAIRS >> j)){
                                        j--;
                                }
                                *pairs = 1 << j;

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

static unsigned is_two_pair(unsigned *const pairs){
        if(!*pairs){
                return 0;
        }

        unsigned i = 1;
        while(*pairs >> i){
                i++;
        }
        const unsigned HIGH_PAIR = 1 << (i-1);

        const unsigned OTHER_PAIRS = *pairs & ~(HIGH_PAIR);
        if(!OTHER_PAIRS){
                return 0;
        }

        *pairs = HIGH_PAIR;

        i = 1;
        while(OTHER_PAIRS >> i){
                i++;
        }
        *pairs |= 1 << (i-1);

        return 1;
}

/* Kelly equation for poker:
 * where        b = net odds received on the wager
 *              c = retained pot / current bankroll
 *              p = probability of winning
 *
 * f = ((b + c/f)*p - (1 - p))/(b + c/f)
 * which simplifies to:
 * f = (sqrt((1 + c - p - b*p)**2 + 4*b*c*p) - (1 + c - p - b*p))/(2*b)
 */
static double kelly(double b, double c, double p){
        double temp = 1 + c - p - b*p;
        return (sqrt(temp*temp + 4*b*c*p) - temp)/(2*b);
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

static void showdown(struct win_counter *const counter, const struct hand best_hand, const unsigned long long COMMUNITY, const unsigned long long DECK){
        const unsigned TOT_CARDS = 52;
        const unsigned long long BOUNDARY = 1ULL << TOT_CARDS;

        unsigned long long cards = 0x3;
        do{
                if((cards & DECK) == cards){
                        const struct hand test_hand = determine_hand(cards|COMMUNITY);
                        if(test_hand.category < best_hand.category){
                                counter->win++;
                        }else if(test_hand.category == best_hand.category){
                                switch(test_hand.category){
                                        case ONE_PAIR:
                                        case TWO_PAIR:
                                                if(test_hand.pairs < best_hand.pairs){
                                                        counter->win++;
                                                }else if(test_hand.pairs == best_hand.pairs){
                                                        if(test_hand.rank < best_hand.rank){
                                                                counter->win++;
                                                        }else if(test_hand.rank == best_hand.rank){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        case THREE_OF_A_KIND:
                                                if(test_hand.triplets < best_hand.triplets){
                                                        counter->win++;
                                                }else if(test_hand.triplets == best_hand.triplets){
                                                        if(test_hand.rank < best_hand.rank){
                                                                counter->win++;
                                                        }else if(test_hand.rank == best_hand.rank){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        case FULL_HOUSE:
                                                if(test_hand.triplets < best_hand.triplets){
                                                        counter->win++;
                                                }else if(test_hand.triplets == best_hand.triplets){
                                                        if(test_hand.pairs < best_hand.pairs){
                                                                counter->win++;
                                                        }else if(test_hand.pairs == best_hand.pairs){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        case FOUR_OF_A_KIND:
                                                if(test_hand.quadruplet < best_hand.quadruplet){
                                                        counter->win++;
                                                }else if(test_hand.quadruplet == best_hand.quadruplet){
                                                        if(test_hand.rank < best_hand.rank){
                                                                counter->win++;
                                                        }else if(test_hand.rank == best_hand.rank){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        default:
                                                if(test_hand.rank < best_hand.rank){
                                                        counter->win++;
                                                }else if(test_hand.rank == best_hand.rank){
                                                        counter->split++;
                                                }
                                                break;
                                }
                        }
                }

                unsigned long long pos_mask = 1;
                while(!(cards & pos_mask)){
                        pos_mask <<= 1;
                }

                unsigned card_num = 0;
                while(cards & pos_mask){
                        card_num++;
                        pos_mask <<= 1;
                }

                cards |= pos_mask;
                cards &= ~(pos_mask - 1);
                cards |= (1ULL<<card_num-1) - 1;
        }while(cards < BOUNDARY);
}
