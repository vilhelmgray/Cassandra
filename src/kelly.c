/*
 * Copyright (C) 2017 William Breathitt Gray
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

#include <gsl/gsl_errno.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_sf.h>

#include "counter.h"

#include "kelly.h"

struct kelly_params{
        const struct win_counter *counter;
        double retained_gain;
        unsigned long comb;
        unsigned num_opponents;
};

static double kellyRoot(double f, void *params);

double kelly(const unsigned NUM_OPPONENTS, const double RETAINED_GAIN, const struct win_counter *const COUNTER, const unsigned long COMB){
        /* if no chance of winning/splitting, then just fold;
         * if no chance of losing, then just go all-in */
        if(COUNTER->win + COUNTER->split == 0){
                return 0;
        }else if(COUNTER->win + COUNTER->split == COMB){
                return 1;
        }

        struct kelly_params kparams = {
                .counter = COUNTER,
                .num_opponents = NUM_OPPONENTS,
                .retained_gain = RETAINED_GAIN,
                .comb = COMB
        };

        double x_lo = 0.001;
        double x_hi = 0.999;
        const double X_LO_KELLY = kellyRoot(x_lo, &kparams);
        const double X_HI_KELLY = kellyRoot(x_hi, &kparams);
        /* actual criterion is where a bound returns 0,
         * otherwise if no bound straddles actual criterion,
         * bet high bound if both bounds are positive,
         * or fold if both bounds return negative */
        if(X_LO_KELLY == 0){
                return x_lo;
        }else if(X_HI_KELLY == 0 || (X_LO_KELLY > 0 && X_HI_KELLY > 0)){
                return x_hi;
        }else if(X_LO_KELLY < 0 && X_HI_KELLY < 0){
                return 0;
        }

        gsl_function kellyRootFunc = {
                .function = &kellyRoot,
                .params = &kparams
        };

	gsl_root_fsolver *const KELLY_ROOT_SOLV = gsl_root_fsolver_alloc(gsl_root_fsolver_brent);
        if(!KELLY_ROOT_SOLV){
                fprintf(stderr, "Unable to allocate memory for GSL one dimensional root-finding function\n");
                return 0;
        }
        gsl_root_fsolver_set(KELLY_ROOT_SOLV, &kellyRootFunc, x_lo, x_hi);

        double criterion = 0;
        for(unsigned i = 0; i < 256; i++){
                gsl_root_fsolver_iterate(KELLY_ROOT_SOLV);

                criterion = gsl_root_fsolver_root(KELLY_ROOT_SOLV);

                x_lo = gsl_root_fsolver_x_lower(KELLY_ROOT_SOLV);
                x_hi = gsl_root_fsolver_x_upper(KELLY_ROOT_SOLV);
                if(gsl_root_test_interval(x_lo, x_hi, 0, 0.001) == GSL_SUCCESS){
                        break;
                }
        }

        gsl_root_fsolver_free(KELLY_ROOT_SOLV);

        return criterion;
}

static double kellyRoot(double x, void *params){
        struct kelly_params *kparams = params;
        const unsigned NUM_OPPONENTS = kparams->num_opponents;
        const double RETAINED_GAIN = kparams->retained_gain;
        const unsigned long COMB = kparams->comb;
        const unsigned long WIN = kparams->counter->win;
        const unsigned long SPLIT = kparams->counter->split;

        double p_noloss = 0;
        double root = 0;
        /* TODO: prevent division by 0 when NUM_OPPONENTS is greater than or equal to COMB;
         *       winning/splitting may not be possible when NUM_OPPONENTS is high, so check */
        for(unsigned i = 0; i <= NUM_OPPONENTS; i++){
                double p_win = 1;
                for(unsigned j = 0; j < i; j++){
                        p_win *= (double)(WIN - j)/(COMB - j);
                }
                const unsigned NUM_SPLIT_OPPONENTS = NUM_OPPONENTS - i;
                double p_split = 1;
                for(unsigned j = 0; j < NUM_SPLIT_OPPONENTS; j++){
                        p_split *= (double)(SPLIT - j)/(COMB - j);
                }

                const double P = gsl_sf_choose(NUM_OPPONENTS, i) * p_win * p_split;
                p_noloss += P;

                /* NET_WIN_ODDS = NUM_OPPONENTS + RETAINED_GAIN/x
                 * PAYOUT = (1 + NET_WIN_ODDS)/(1 + NUM_SPLIT_OPPONENTS)
                 * NET_ODDS = PAYOUT - 1 */
                const double NET_ODDS = (1 + NUM_OPPONENTS + RETAINED_GAIN/x) / (1 + NUM_SPLIT_OPPONENTS) - 1;
                root += (P * NET_ODDS)/(1 + NET_ODDS * x);
        }

        const double P_LOSE = 1 - p_noloss;
        root -= P_LOSE/(1-x);

        return root;
}
