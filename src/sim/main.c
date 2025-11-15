#include "world.h"
#include "../ai/decision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declared in ai_engine.c
void ai_tick(WorldState* world);

static int parse_ticks_arg(int argc, char** argv);

int main(int argc, char** argv) {
    int ticks = parse_ticks_arg(argc, argv);
    if (ticks <= 0) ticks = 20; // default 20 ticks

    WorldState world;
    if (!world_init_default(&world)) {
        fprintf(stderr, "Failed to initialize world\n");
        return 1;
    }

    printf("=== Victoria III Economic AI Simulator ===\n");
    printf("Running for %d ticks.\n", ticks);
    printf("-----------------------------------------\n");

    for (int i = 0; i < ticks; i++) {
        // One simulation tick (production, consumption, prices, budget)
        world_tick(&world);

        // Print world state
        world_log_state(&world, stdout);

        // Run economic AI
        ai_tick(&world);

        printf("-----------------------------------------\n");
    }

    world_free(&world);
    return 0;
}

static int parse_ticks_arg(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "--ticks") == 0 || strcmp(argv[i], "-t") == 0) && i + 1 < argc) {
            return atoi(argv[i + 1]);
        }
    }
    return -1;
}
