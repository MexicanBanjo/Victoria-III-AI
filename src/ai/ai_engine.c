#include "decision.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const double W_PROFIT           = 1.0;
static const double W_JOBS             = 0.01;
static const double W_STRATEGIC        = 5.0;
static const double W_SHORTAGE         = 0.5;
static const double W_CONSTRUCTION_EFF = 10.0;

static const double MIN_SCORE_THRESHOLD = 10.0;
static const float  CONSTRUCTION_COST_STEEL   = 5000.0f;
static const float  CONSTRUCTION_COST_TEXTILE = 3000.0f;

static double good_strategic_importance(GoodType t) {
    switch (t) {
        case GOOD_COAL:     return 1.0;
        case GOOD_IRON:     return 1.5;
        case GOOD_GRAIN:    return 1.0;
        case GOOD_TOOLS:    return 2.0;
        case GOOD_CLOTHING: return 1.0;
        case GOOD_STEEL:    return 2.5;
        default:            return 1.0;
    }
}

static double compute_projected_profit_for_build(const WorldState* world, int building_type);
static double compute_shortage_reduction_for_build(const WorldState* world, int building_type);
static double compute_construction_efficiency(double projected_profit, float cost);

static void consider_build_decisions(const WorldState* world, Decision* best);
static void consider_expand_decisions(const WorldState* world, Decision* best);

void ai_tick(WorldState* world) {
    Decision best;
    best.type = DECISION_NONE;
    best.score = -1e30;
    best.target_building_index = -1;
    best.building_type_to_build = -1;

    consider_build_decisions(world, &best);
    consider_expand_decisions(world, &best);

    if (best.type == DECISION_NONE || best.score < MIN_SCORE_THRESHOLD) {
        printf("[AI] No action taken this tick (best score = %.2f)\n", best.score);
        return;
    }

    if (best.type == DECISION_BUILD) {
        if (world->treasury < (best.building_type_to_build == 0
                               ? CONSTRUCTION_COST_STEEL
                               : CONSTRUCTION_COST_TEXTILE)) {
            printf("[AI] Wanted to build but not enough treasury (score=%.2f)\n", best.score);
            return;
        }

        int new_count = world->building_count + 1;
        Building* new_array = (Building*)realloc(world->buildings,
                                                 new_count * sizeof(Building));
        if (!new_array) {
            printf("[AI] Failed to allocate new building\n");
            return;
        }
        world->buildings = new_array;
        Building* b = &world->buildings[world->building_count];
        memset(b, 0, sizeof(*b));
        b->id = world->building_count;

        if (best.building_type_to_build == 0) {
            strcpy(b->name, "Steel Mill");
            b->level = 1;
            b->productivity = 1.0f;
            b->input_count = 2;
            b->inputs[0].type = GOOD_COAL;
            b->inputs[0].amount = 2.0f;
            b->inputs[1].type = GOOD_IRON;
            b->inputs[1].amount = 3.0f;
            b->output_count = 1;
            b->outputs[0].type = GOOD_STEEL;
            b->outputs[0].amount = 4.0f;
            b->maintenance_cost = 500.0f;
            world->treasury -= CONSTRUCTION_COST_STEEL;
        } else {
            strcpy(b->name, "Textile Mill");
            b->level = 1;
            b->productivity = 1.0f;
            b->input_count = 1;
            b->inputs[0].type = GOOD_GRAIN;
            b->inputs[0].amount = 2.0f;
            b->output_count = 1;
            b->outputs[0].type = GOOD_CLOTHING;
            b->outputs[0].amount = 3.0f;
            b->maintenance_cost = 200.0f;
            world->treasury -= CONSTRUCTION_COST_TEXTILE;
        }

        world->building_count = new_count;

        printf("[AI] BUILD: %s (score=%.2f)\n",
               (best.building_type_to_build == 0 ? "Steel Mill" : "Textile Mill"),
               best.score);
    } else if (best.type == DECISION_EXPAND) {
        if (best.target_building_index < 0 || best.target_building_index >= world->building_count) {
            return;
        }
        Building* b = &world->buildings[best.target_building_index];
        float cost = 0.0f;
        if (strcmp(b->name, "Steel Mill") == 0) {
            cost = CONSTRUCTION_COST_STEEL * 0.5f;
        } else {
            cost = CONSTRUCTION_COST_TEXTILE * 0.5f;
        }
        if (world->treasury < cost) {
            printf("[AI] Wanted to expand %s but insufficient treasury (score=%.2f)\n",
                   b->name, best.score);
            return;
        }
        b->level += 1;
        world->treasury -= cost;
        printf("[AI] EXPAND: %s to level %d (score=%.2f)\n", b->name, b->level, best.score);
    }
}

static void consider_build_decisions(const WorldState* world, Decision* best) {
    int types = 2; // 0 = Steel, 1 = Textile
    for (int t = 0; t < types; t++) {
        double profit = compute_projected_profit_for_build(world, t);
        double jobs = 200.0;
        double strategic = 0.0;
        double shortage = compute_shortage_reduction_for_build(world, t);
        float cost = (t == 0) ? CONSTRUCTION_COST_STEEL : CONSTRUCTION_COST_TEXTILE;
        double eff = compute_construction_efficiency(profit, cost);

        if (t == 0) {
            strategic = good_strategic_importance(GOOD_STEEL);
        } else {
            strategic = good_strategic_importance(GOOD_CLOTHING);
        }

        double score = W_PROFIT * profit
                     + W_JOBS * jobs
                     + W_STRATEGIC * strategic
                     + W_SHORTAGE * shortage
                     + W_CONSTRUCTION_EFF * eff;

        if (score > best->score) {
            best->type = DECISION_BUILD;
            best->building_type_to_build = t;
            best->target_building_index = -1;
            best->score = score;
        }
    }
}

static void consider_expand_decisions(const WorldState* world, Decision* best) {
    for (int i = 0; i < world->building_count; i++) {
        const Building* b = &world->buildings[i];
        double profit = 500.0;
        double jobs = 150.0;
        double strategic = 0.0;
        double shortage = 0.0;
        float cost = 0.0f;

        if (strcmp(b->name, "Steel Mill") == 0) {
            strategic = good_strategic_importance(GOOD_STEEL);
            cost = CONSTRUCTION_COST_STEEL * 0.5f;
            shortage = compute_shortage_reduction_for_build(world, 0);
        } else {
            strategic = good_strategic_importance(GOOD_CLOTHING);
            cost = CONSTRUCTION_COST_TEXTILE * 0.5f;
            shortage = compute_shortage_reduction_for_build(world, 1);
        }

        double eff = compute_construction_efficiency(profit, cost);

        double score = W_PROFIT * profit
                     + W_JOBS * jobs
                     + W_STRATEGIC * strategic
                     + W_SHORTAGE * shortage
                     + W_CONSTRUCTION_EFF * eff;

        if (score > best->score) {
            best->type = DECISION_EXPAND;
            best->target_building_index = i;
            best->building_type_to_build = -1;
            best->score = score;
        }
    }
}

static double compute_projected_profit_for_build(const WorldState* world, int building_type) {
    const MarketState* m = &world->market;
    double revenue = 0.0;
    double costs = 0.0;

    if (building_type == 0) {
        float out_amount = 4.0f;
        float out_price = m->goods[GOOD_STEEL].price;
        revenue = out_amount * out_price;

        float in_coal = 2.0f * m->goods[GOOD_COAL].price;
        float in_iron = 3.0f * m->goods[GOOD_IRON].price;
        costs = in_coal + in_iron + 500.0;
    } else {
        float out_amount = 3.0f;
        float out_price = m->goods[GOOD_CLOTHING].price;
        revenue = out_amount * out_price;

        float in_grain = 2.0f * m->goods[GOOD_GRAIN].price;
        costs = in_grain + 200.0;
    }

    return revenue - costs;
}

static double compute_shortage_reduction_for_build(const WorldState* world, int building_type) {
    const MarketState* m = &world->market;
    double before = 0.0;
    double after = 0.0;

    if (building_type == 0) {
        const GoodState* g = &m->goods[GOOD_STEEL];
        before = (g->demand > g->supply) ? (g->demand - g->supply) : 0.0;
        double extra_supply = 4.0;
        after = (g->demand > g->supply + extra_supply)
                    ? (g->demand - (g->supply + extra_supply))
                    : 0.0;
    } else {
        const GoodState* g = &m->goods[GOOD_CLOTHING];
        before = (g->demand > g->supply) ? (g->demand - g->supply) : 0.0;
        double extra_supply = 3.0;
        after = (g->demand > g->supply + extra_supply)
                    ? (g->demand - (g->supply + extra_supply))
                    : 0.0;
    }

    return before - after;
}

static double compute_construction_efficiency(double projected_profit, float cost) {
    if (cost <= 0.0f) return 0.0;
    return projected_profit / (double)cost;
}
