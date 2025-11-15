#include "world.h"
#include "building.h"
#include "pop.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const float ALPHA_PRICE_ADJUST = 0.1f;
static const float EPSILON_SUPPLY = 1e-3f;

static void reset_supply_and_demand(WorldState* world);
static void pricing_phase(WorldState* world);
static void budget_phase(WorldState* world);

int world_init_default(WorldState* world) {
    memset(world, 0, sizeof(*world));

    // Initialize market base prices and starting values
    for (int i = 0; i < NUM_GOODS; i++) {
        world->market.goods[i].price = 1.0f;
        world->market.goods[i].base_price = 1.0f;
        world->market.goods[i].supply = 0.0f;
        world->market.goods[i].demand = 0.0f;
    }

    world->market.goods[GOOD_COAL].base_price = 5.0f;
    world->market.goods[GOOD_IRON].base_price = 8.0f;
    world->market.goods[GOOD_GRAIN].base_price = 1.0f;
    world->market.goods[GOOD_TOOLS].base_price = 20.0f;
    world->market.goods[GOOD_CLOTHING].base_price = 10.0f;
    world->market.goods[GOOD_STEEL].base_price = 25.0f;

    for (int i = 0; i < NUM_GOODS; i++) {
        world->market.goods[i].price = world->market.goods[i].base_price;
        world->market.goods[i].supply = 1000.0f;
        world->market.goods[i].demand = 1000.0f;
    }

    world->treasury = 100000.0f;
    world->weekly_tax_income = 5000.0f;
    world->current_tick = 0;

    // Allocate a couple of buildings
    world->building_count = 2;
    world->buildings = (Building*)calloc(world->building_count, sizeof(Building));
    if (!world->buildings) return 0;

    // Building 0: Steel Mill (uses coal+iron, produces steel)
    Building* b0 = &world->buildings[0];
    b0->id = 0;
    strcpy(b0->name, "Steel Mill");
    b0->level = 1;
    b0->productivity = 1.0f;
    b0->input_count = 2;
    b0->inputs[0].type = GOOD_COAL;
    b0->inputs[0].amount = 2.0f;
    b0->inputs[1].type = GOOD_IRON;
    b0->inputs[1].amount = 3.0f;
    b0->output_count = 1;
    b0->outputs[0].type = GOOD_STEEL;
    b0->outputs[0].amount = 4.0f;
    b0->maintenance_cost = 500.0f;

    // Building 1: Textile (uses grain, produces clothing)
    Building* b1 = &world->buildings[1];
    b1->id = 1;
    strcpy(b1->name, "Textile Mill");
    b1->level = 1;
    b1->productivity = 1.0f;
    b1->input_count = 1;
    b1->inputs[0].type = GOOD_GRAIN;
    b1->inputs[0].amount = 2.0f;
    b1->output_count = 1;
    b1->outputs[0].type = GOOD_CLOTHING;
    b1->outputs[0].amount = 3.0f;
    b1->maintenance_cost = 200.0f;

    // Pops
    world->pop_count = 1;
    world->pops = (Pop*)calloc(world->pop_count, sizeof(Pop));
    if (!world->pops) return 0;

    Pop* p0 = &world->pops[0];
    p0->id = 0;
    p0->size = 10000;
    p0->wage = 2.0f;
    p0->consumption_count = 2;
    p0->consumption[0].type = GOOD_GRAIN;
    p0->consumption[0].amount = 0.2f; // per capita
    p0->consumption[1].type = GOOD_CLOTHING;
    p0->consumption[1].amount = 0.05f;

    return 1;
}

void world_tick(WorldState* world) {
    world->current_tick++;

    reset_supply_and_demand(world);

    production_phase(world);
    consumption_phase(world);
    pricing_phase(world);
    budget_phase(world);
}

void world_free(WorldState* world) {
    free(world->buildings);
    free(world->pops);
    world->buildings = NULL;
    world->pops = NULL;
    world->building_count = 0;
    world->pop_count = 0;
}

static void reset_supply_and_demand(WorldState* world) {
    for (int i = 0; i < NUM_GOODS; i++) {
        world->market.goods[i].supply = 0.0f;
        world->market.goods[i].demand = 0.0f;
    }
}

static void pricing_phase(WorldState* world) {
    for (int i = 0; i < NUM_GOODS; i++) {
        GoodState* g = &world->market.goods[i];
        float supply = g->supply;
        float demand = g->demand;

        if (supply < EPSILON_SUPPLY) {
            supply = EPSILON_SUPPLY;
        }
        float ratio = demand / supply;
        float target_price = g->base_price * ratio;
        float new_price = g->price + ALPHA_PRICE_ADJUST * (target_price - g->price);
        if (new_price < 0.01f) new_price = 0.01f;
        g->price = new_price;
    }
}

static void budget_phase(WorldState* world) {
    float maintenance = total_building_maintenance(world);
    world->treasury += world->weekly_tax_income;
    world->treasury -= maintenance;
}

void world_log_state(const WorldState* world, FILE* out) {
    fprintf(out, "Tick %d\n", world->current_tick);
    fprintf(out, "Treasury: %.2f\n", world->treasury);
    fprintf(out, "Prices: ");
    for (int i = 0; i < NUM_GOODS; i++) {
        fprintf(out, "%d=%.2f ", i, world->market.goods[i].price);
    }
    fprintf(out, "\n");
}
