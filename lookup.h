#ifndef LOOKUP__H

#define LOOKUP__H

#include "counter.h"

struct probability{
        unsigned long long hand;
        struct win_counter counter;
};

extern const struct probability begin_prob[1326];

#endif
