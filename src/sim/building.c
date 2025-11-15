#include "building.h"
#include <stdio.h>

static float get_good_supply(MarketState* market, GoodType t) {
    return market->goods[t].supply;
}

static void add_good_supply(MarketState* market, GoodType t, float amount) {
    market->goods[t].supply += amount;
}

static void consume_good_supply(MarketState* market, GoodType t, float amount) {
    if (amount <= 0.0f) return;
    float* supply = &market->goods[t].supply;
    if (*supply < amount) {
        amount = *supply;
    }
    *supply -= amount;
}

void production_phase(WorldState* world) {
    MarketState* m = &world->market;

    for (int i = 0; i < world->building_count; i++) {
        Building* b = &world->buildings[i];

        float max_factor = (float)b->level * b->productivity;

        for (int in = 0; in < b->input_count; in++) {
            GoodAmount* ga = &b->inputs[in];
            float needed = ga->amount * b->level;
            float available = get_good_supply(m, ga->type);
            if (available < needed) {
                float factor = available / (needed > 0.0f ? needed : 1.0f);
                if (factor < max_factor) {
                    max_factor = factor;
                }
            }
        }

        if (max_factor <= 0.0f) {
            continue;
        }

        for (int in = 0; in < b->input_count; in++) {
            GoodAmount* ga = &b->inputs[in];
            float to_consume = ga->amount * b->level * max_factor;
            consume_good_supply(m, ga->type, to_consume);
        }

        for (int out = 0; out < b->output_count; out++) {
            GoodAmount* ga = &b->outputs[out];
            float produced = ga->amount * b->level * max_factor;
            add_good_supply(m, ga->type, produced);
        }
    }
}

float total_building_maintenance(const WorldState* world) {
    float total = 0.0f;
    for (int i = 0; i < world->building_count; i++) {
        const Building* b = &world->buildings[i];
        total += b->maintenance_cost * (float)b->level;
    }
    return total;
}
