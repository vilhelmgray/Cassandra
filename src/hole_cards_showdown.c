/*
 * Copyright (C) 2016 William Breathitt Gray
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
#include <stddef.h>

#include "lookup.h"

void print_hand(const unsigned long long HAND);

int main(void){
	printf("Win prob.\tSplit prob.\tHoles cards\n\n");

	const size_t NUM_PROB = sizeof(begin_prob)/sizeof(*begin_prob);
	for(size_t h = 0; h < NUM_PROB; h++){
		const unsigned long HAND_COMB = 2097572400;

		const double win_p = (double)(begin_prob[h].counter.win)/HAND_COMB;
		const double split_p = (double)(begin_prob[h].counter.split)/HAND_COMB;

		printf("%lf\t%lf\t", win_p, split_p);
		print_hand(begin_prob[h].hand);
		printf("\n");
	}

	return 0;
}

void print_hand(unsigned long long hand){
	for(unsigned suit = 0; suit < 4; suit++){
		for(unsigned rank = 0; rank < 13; rank++){
			const unsigned RANK_MASK = 1 << rank;
			if(hand & RANK_MASK){
				switch(rank){
					case 0:
						printf("Ace");
						break;
					case 1:
						printf("Two");
						break;
					case 2:
						printf("Three");
						break;
					case 3:
						printf("Four");
						break;
					case 4:
						printf("Five");
						break;
					case 5:
						printf("Six");
						break;
					case 6:
						printf("Seven");
						break;
					case 7:
						printf("Eight");
						break;
					case 8:
						printf("Nine");
						break;
					case 9:
						printf("Ten");
						break;
					case 10:
						printf("Jack");
						break;
					case 11:
						printf("Queen");
						break;
					case 12:
						printf("King");
						break;
				}

				printf(" of ");

				switch(suit){
					case 0:
						printf("Clubs");
						break;
					case 1:
						printf("Diamonds");
						break;
					case 2:
						printf("Hearts");
						break;
					case 3:
						printf("Spades");
						break;
				}

				printf(", ");
			}
		}

		hand >>= 13;
	}
}
