#ifndef BENDERS_H
#define BENDERS_H

#include "tsp.h"
#include "tsp_cplex.h"
#include "utilities.h"

#include <ilcplex/cplex.h>

#define BENDERS "B"

/**
 * Find the optimal solution of the instance
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param timelimit Time limit for the algorithm
 */
void benders_loop(instance *inst, solution *sol, const double timelimit);


#endif //BENDERS_H