#ifndef WORLD_H
#define WORLD_H

#include <stdio.h>

typedef enum {
    GOOD_COAL = 0,
    GOOD_IRON,
    GOOD_GRAIN,
    GOOD_TOOLS,
    GOOD_CLOTHING,
    GOOD_STEEL,
    NUM_GOODS
} GoodType;

typedef struct {
    float price;
    float supply;
    float demand;
    float base_price;
} GoodState;

typedef struct {
    GoodState goods[NUM_GOODS];
} MarketState;

typedef struct {
    GoodType type;
    float amount;
} GoodAmount;

typedef struct {
    int id;
    char name[32];
    int level;

    float productivity;

    GoodAmount inputs[4];
    int input_count;

    GoodAmount outputs[4];
    int output_count;

    float maintenance_cost;
} Building;

typedef struct {
    int id;
    int size;
    float wage;

    GoodAmount consumption[4];
    int consumption_count;
} Pop;

typedef struct {
    MarketState market;

    Building* buildings;
    int building_count;

    Pop* pops;
    int pop_count;

    float treasury;
    float weekly_tax_income;

    int current_tick;
} WorldState;

int world_init_default(WorldState* world);
void world_tick(WorldState* world);
void world_free(WorldState* world);
void world_log_state(const WorldState* world, FILE* out);

#endif
