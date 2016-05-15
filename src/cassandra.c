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
        unsigned triplet;
        unsigned pairs;
        unsigned rank;
};

static void betting_round(unsigned *bankroll, unsigned *pot, const struct win_counter counter, const unsigned long COMB);
static void determine_hand(struct hand *const best_hand, const unsigned long long HAND);
static void determine_win_counter(struct win_counter *const counter, const unsigned long long DECK, const unsigned long long COMMUNITY, const unsigned long long HAND, const unsigned NUM_CARDS);
static unsigned evaluate_opponents(unsigned numPlayers);
static unsigned find_extrema(const unsigned LUMP, unsigned num);
static void generate_lookup(void);
static unsigned long long get_card(unsigned long long *deck);
static unsigned is_flush(const unsigned CHITS, const unsigned DHITS, const unsigned HHITS, const unsigned SHITS);
static unsigned is_full_house(const unsigned TRIPLET, unsigned *const pairs);
static unsigned is_straight(const unsigned LUMP, const unsigned SMASK);
static unsigned is_straight_flush(const unsigned CLUB, const unsigned DIAMOND, const unsigned HEART, const unsigned SPADE, const unsigned SMASK);
static unsigned is_two_pair(unsigned *const pairs);
static double kelly(const double B, const double C, const double P);
static unsigned long long parse_card(const char *card_str);
static void showdown(struct win_counter *const counter, const struct hand *const BEST_HAND, const unsigned long long COMMUNITY, const unsigned long long DECK);

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

        struct hand best_hand;
        determine_hand(&best_hand, hand|flop|turn|river);
        counter.split = 0;
        counter.win = 0;
        showdown(&counter, &best_hand, flop|turn|river, deck);

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

static void determine_hand(struct hand *const best_hand, const unsigned long long HAND){
        enum hand_t type = HIGH_CARD;

        const unsigned CLUB = HAND & 0x1FFF;
        const unsigned DIAMOND = HAND>>13 & 0x1FFF;
        const unsigned HEART = HAND>>26 & 0x1FFF;
        const unsigned SPADE = HAND>>39 & 0x1FFF;

        const unsigned CLUB_ACED = CLUB | (CLUB & 0x1) << 13;
        const unsigned DIAMOND_ACED = DIAMOND | (DIAMOND & 0x1) << 13;
        const unsigned HEART_ACED = HEART | (HEART & 0x1) << 13;
        const unsigned SPADE_ACED = SPADE | (SPADE & 0x1) << 13;

        const unsigned LUMP = CLUB_ACED | DIAMOND_ACED | HEART_ACED | SPADE_ACED;

        unsigned rank;
        for(unsigned i = 0; i < 10; i++){
                const unsigned SMASK = 0x3E00 >> i;

                // check for straight
                if(is_straight(LUMP, SMASK)){
                        // check for straight-flush
                        if(is_straight_flush(CLUB_ACED, DIAMOND_ACED, HEART_ACED, SPADE_ACED, SMASK)){
                                type = STRAIGHT_FLUSH;
                        }else{
                                type = STRAIGHT;
                        }

                        rank = 9 - i;

                        break;
                }
        }

        const unsigned CLUB_NORMALIZED = CLUB_ACED >> 1;
        const unsigned DIAMOND_NORMALIZED = DIAMOND_ACED >> 1;
        const unsigned HEART_NORMALIZED = HEART_ACED >> 1;
        const unsigned SPADE_NORMALIZED = SPADE_ACED >> 1;
        const unsigned LUMP_NORMALIZED = LUMP >> 1;

        unsigned pairs = 0;
        unsigned triplet = 0;
        unsigned quadruplet = 0;

        unsigned chits = 0;
        unsigned dhits = 0;
        unsigned hhits = 0;
        unsigned shits = 0;

        for(unsigned i = 0; type < FOUR_OF_A_KIND && i < 13; i++){
                const unsigned C = CLUB_NORMALIZED & 1<<i;
                const unsigned D = DIAMOND_NORMALIZED & 1<<i;
                const unsigned H = HEART_NORMALIZED & 1<<i;
                const unsigned S = SPADE_NORMALIZED & 1<<i;

                const unsigned C_HIT = (C > 0);
                const unsigned D_HIT = (D > 0);
                const unsigned H_HIT = (H > 0);
                const unsigned S_HIT = (S > 0);

                switch(C_HIT + D_HIT + H_HIT + S_HIT){
                        case 4:
                                quadruplet = 1<<i;
                                type = FOUR_OF_A_KIND;
                                break;
                        case 3:
                                triplet = 1<<i;
                                if(type < THREE_OF_A_KIND){
                                        type = THREE_OF_A_KIND;
                                }
                        case 2:
                                pairs |= 1<<i;
                                if(type < ONE_PAIR){
                                        type = ONE_PAIR;
                                }
                                break;
                }

                // check for flush
                if(type < FLUSH){
                        chits += C_HIT;
                        dhits += D_HIT;
                        hhits += H_HIT;
                        shits += S_HIT;
                        const unsigned SUIT = is_flush(chits, dhits, hhits, shits);
                        if(SUIT){
                                switch(SUIT){
                                        case 1:
                                                rank = CLUB_NORMALIZED;
                                                break;
                                        case 2:
                                                rank = DIAMOND_NORMALIZED;
                                                break;
                                        case 3:
                                                rank = HEART_NORMALIZED;
                                                break;
                                        case 4:
                                                rank = SPADE_NORMALIZED;
                                                break;
                                }

                                type = FLUSH;
                        }
                }
        }

        // check for full-house
        if(type < FULL_HOUSE && is_full_house(triplet, &pairs)){
                type = FULL_HOUSE;
        }

        // check for two pair
        if(type < TWO_PAIR && is_two_pair(&pairs)){
                type = TWO_PAIR;
        }

        switch(type){
                case HIGH_CARD:
                        rank = find_extrema(LUMP_NORMALIZED, 5);
                        break;
                case ONE_PAIR:
                        rank = find_extrema(LUMP_NORMALIZED & (~pairs), 3);
                        break;
                case TWO_PAIR:
                        rank = find_extrema(LUMP_NORMALIZED & (~pairs), 1);
                        break;
                case THREE_OF_A_KIND:
                        rank = find_extrema(LUMP_NORMALIZED & (~triplet), 2);
                case FOUR_OF_A_KIND:
                        rank = find_extrema(LUMP_NORMALIZED & (~quadruplet), 1);
        }

        best_hand->category = type;
        best_hand->quadruplet = quadruplet;
        best_hand->triplet = triplet;
        best_hand->pairs = pairs;
        best_hand->rank = rank;
}

static void determine_win_counter(struct win_counter *const counter, const unsigned long long DECK, const unsigned long long COMMUNITY, const unsigned long long HAND, const unsigned NUM_CARDS){
        const unsigned TOT_CARDS = 52;
        const unsigned long long BOUNDARY = 1ULL << TOT_CARDS;

        unsigned long long cards = (1ULL << NUM_CARDS) - 1;
        do{
                if((cards & DECK) == cards){
                        struct hand best_hand;
                        determine_hand(&best_hand, HAND|COMMUNITY|cards);

                        showdown(counter, &best_hand, COMMUNITY|cards, DECK & (~cards));
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

                cards &= ~(pos_mask - 1);
                cards |= pos_mask;
                cards |= (1ULL << card_num-1) - 1;
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

static unsigned find_extrema(const unsigned LUMP, unsigned num){
        unsigned extrema = 0;

        for(int i = 12; num && i >= 0; i--){
                const unsigned CARD = 1 << i;
                if(LUMP & CARD){
                        extrema |= CARD;
                        num--;
                }
        }

        return extrema;
}

static void generate_lookup(void){
        printf("const struct probability begin_prob[325] = {\n");

        const unsigned TOT_CARDS = 26;
        const unsigned long long BOUNDARY = 1ULL << TOT_CARDS;

        unsigned long long cards = 0x3;
        do{
                struct win_counter counter = {0};
                determine_win_counter(&counter, 0xFFFFFFFFFFFFF & (~cards), 0, cards, 5);
                printf("\t{ .hand = 0x%llX, .counter = { .split = %lu, .win = %lu } },\n", cards, counter.split, counter.win);

                unsigned long long pos_mask = 1;
                while(!(cards & pos_mask)){
                        pos_mask <<= 1;
                }

                unsigned card_num = 0;
                while(cards & pos_mask){
                        card_num++;
                        pos_mask <<= 1;
                }

                cards &= ~(pos_mask - 1);
                cards |= pos_mask;
                cards |= (1ULL << card_num-1) - 1;
        }while(cards < BOUNDARY);

        printf("};\n");
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

static unsigned is_flush(const unsigned CHITS, const unsigned DHITS, const unsigned HHITS, const unsigned SHITS){
        if(CHITS == 5){
                return 1;
        }
        if(DHITS == 5){
                return 2;
        }
        if(HHITS == 5){
                return 3;
        }
        if(SHITS == 5){
                return 4;
        }

        return 0;
}

static unsigned is_full_house(const unsigned TRIPLET, unsigned *const pairs){
        if(!TRIPLET){
                return 0;
        }

        const unsigned OTHER_PAIRS = *pairs & ~(TRIPLET);
        if(!OTHER_PAIRS){
                return 0;
        }

        unsigned i = 1;
        while(OTHER_PAIRS >> i){
                i++;
        }
        *pairs = 1 << (i-1);

        return 1;
}

static unsigned is_straight(const unsigned LUMP, const unsigned SMASK){
        return (LUMP & SMASK) == SMASK;
}

static unsigned is_straight_flush(const unsigned CLUB, const unsigned DIAMOND, const unsigned HEART, const unsigned SPADE, const unsigned SMASK){
        return is_straight(CLUB, SMASK) || is_straight(DIAMOND, SMASK) || is_straight(HEART, SMASK) || is_straight(SPADE, SMASK);
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
 * where        B = net odds received on the wager
 *              C = retained pot / current bankroll
 *              P = probability of winning
 *
 * F = ((B + C/F)*P - (1 - P))/(B + C/F)
 * which simplifies to:
 * F = (sqrt((1 + C - P - B*P)**2 + 4*B*C*P) - (1 + C - P - B*P))/(2*B)
 */
static double kelly(const double B, const double C, const double P){
        const double TEMP = 1 + C - P - B*P;
        return (sqrt(TEMP*TEMP + 4*B*C*P) - TEMP)/(2*B);
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

static void showdown(struct win_counter *const counter, const struct hand *const BEST_HAND, const unsigned long long COMMUNITY, const unsigned long long DECK){
        const unsigned TOT_CARDS = 52;
        const unsigned long long BOUNDARY = 1ULL << TOT_CARDS;

        unsigned long long cards = 0x3;
        do{
                if((cards & DECK) == cards){
                        struct hand test_hand;
                        determine_hand(&test_hand, cards|COMMUNITY);

                        if(test_hand.category < BEST_HAND->category){
                                counter->win++;
                        }else if(test_hand.category == BEST_HAND->category){
                                switch(test_hand.category){
                                        case ONE_PAIR:
                                        case TWO_PAIR:
                                                if(test_hand.pairs < BEST_HAND->pairs){
                                                        counter->win++;
                                                }else if(test_hand.pairs == BEST_HAND->pairs){
                                                        if(test_hand.rank < BEST_HAND->rank){
                                                                counter->win++;
                                                        }else if(test_hand.rank == BEST_HAND->rank){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        case THREE_OF_A_KIND:
                                                if(test_hand.triplet < BEST_HAND->triplet){
                                                        counter->win++;
                                                }else if(test_hand.triplet == BEST_HAND->triplet){
                                                        if(test_hand.rank < BEST_HAND->rank){
                                                                counter->win++;
                                                        }else if(test_hand.rank == BEST_HAND->rank){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        case FULL_HOUSE:
                                                if(test_hand.triplet < BEST_HAND->triplet){
                                                        counter->win++;
                                                }else if(test_hand.triplet == BEST_HAND->triplet){
                                                        if(test_hand.pairs < BEST_HAND->pairs){
                                                                counter->win++;
                                                        }else if(test_hand.pairs == BEST_HAND->pairs){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        case FOUR_OF_A_KIND:
                                                if(test_hand.quadruplet < BEST_HAND->quadruplet){
                                                        counter->win++;
                                                }else if(test_hand.quadruplet == BEST_HAND->quadruplet){
                                                        if(test_hand.rank < BEST_HAND->rank){
                                                                counter->win++;
                                                        }else if(test_hand.rank == BEST_HAND->rank){
                                                                counter->split++;
                                                        }
                                                }
                                                break;
                                        default:
                                                if(test_hand.rank < BEST_HAND->rank){
                                                        counter->win++;
                                                }else if(test_hand.rank == BEST_HAND->rank){
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

                cards &= ~(pos_mask - 1);
                cards |= pos_mask;
                cards |= (1ULL << card_num-1) - 1;
        }while(cards < BOUNDARY);
}
