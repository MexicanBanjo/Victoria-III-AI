#ifndef DECISION_H
#define DECISION_H

#include "../sim/world.h"

typedef enum {
    DECISION_NONE = 0,
    DECISION_BUILD,
    DECISION_EXPAND
} DecisionType;

typedef struct {
    DecisionType type;
    int target_building_index; // for EXPAND; -1 for BUILD
    int building_type_to_build; // 0 = Steel Mill, 1 = Textile Mill, etc. (simple demo)
    double score;
} Decision;

#endif
