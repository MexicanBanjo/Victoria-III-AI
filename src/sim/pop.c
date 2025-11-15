#include "pop.h"
#include <stdio.h>

void consumption_phase(WorldState* world) {
    MarketState* m = &world->market;

    for (int i = 0; i < world->pop_count; i++) {
        Pop* p = &world->pops[i];
        float income = (float)p->size * p->wage;
        float budget = income;

        for (int c = 0; c < p->consumption_count; c++) {
            GoodAmount* ga = &p->consumption[c];
            GoodType t = ga->type;
            float desired_per_capita = ga->amount;
            float desired_total = desired_per_capita * (float)p->size;

            float price = m->goods[t].price;
            float max_affordable = budget / (price > 0.0f ? price : 1.0f);

            float to_buy = desired_total;
            if (to_buy > max_affordable) {
                to_buy = max_affordable;
            }

            float* supply = &m->goods[t].supply;
            if (to_buy > *supply) {
                to_buy = *supply;
            }

            *supply -= to_buy;
            m->goods[t].demand += to_buy;

            budget -= to_buy * price;
            if (budget <= 0.0f) break;
        }
    }
}
