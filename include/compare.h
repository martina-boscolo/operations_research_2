#ifndef COMPARE_H
#define COMPARE_H

#include "tsp.h"
#include "heuristics.h"
#include "vns.h"
#include "tabu_search.h"

#define COMPARE "COMPARE"
#define INSTANCE_COMPARE 5
#define TIMELIMIT_COMPARE 5


int run_nn_algorithm(instance *inst, solution *sol);
int run_vns_algorithm(instance *inst, solution *sol);
int run_tabu_search_algorithm(instance *inst, solution *sol);
int compare(instance *inst);

#endif //COMPARE_H