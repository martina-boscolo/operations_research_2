#ifndef MATHEURISTIC_H
#define MATHEURISTIC_H

#include "tsp.h"
#include "utilities_solution.h"
#include "utilities.h"
#include "tsp_cplex.h"

#define HF "HF"

void hard_fixing(instance *inst, solution *sol, const double timelimit);

#endif //MATHEURISTIC_H