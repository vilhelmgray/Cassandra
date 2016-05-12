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

int main(void){
	printf("Win probability\tSplit probability\tHand\n\n");

	const size_t NUM_PROB = sizeof(begin_prob)/sizeof(*begin_prob);
	size_t h = 0;
	for(h = 0; h < NUM_PROB; h++){
		const unsigned long HAND_COMB = 2097572400UL;

		const double win_p = (double)(begin_prob[h].counter.win)/HAND_COMB;
		const double split_p = (double)(begin_prob[h].counter.split)/HAND_COMB;

		printf("%lf\t%lf\t0x%013llX\n", win_p, split_p, begin_prob[h].hand);
	}

	return 0;
}
