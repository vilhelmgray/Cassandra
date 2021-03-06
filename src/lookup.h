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

#ifndef LOOKUP__H

#define LOOKUP__H

#include "counter.h"

struct probability{
        unsigned long long hand;
        struct win_counter counter;
};

extern const struct probability begin_prob[169];

#endif
