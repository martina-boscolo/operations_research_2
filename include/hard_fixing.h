#ifndef HARD_FIXING_H
#define HARD_FIXING_H

#include "tsp.h"
#include "utilities_solution.h"
#include "utilities.h"
#include "tsp_cplex.h"

#define HARD_FIXING "HF"

/**
 * Hard fixing algorithm.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution that will contain the best solution between the input one and the best found one (input/output)
 * @param timelimit The time limit for the algorithm (input)
 */
void hard_fixing(instance *inst, solution *sol, const double timelimit);

/**
 * Fix some edges of the solution in the model.
 * NOTE: Only the first fixed_count cells of output arrays will contain feasible values.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The reference solution (input)
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 * @param p The percentage of edges to be fixed (input)
 * 
 * @return The number of edges that will be actually fixed 
 */
int set_lowerbounds(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp, const double p);

/**
 * Reset the lower bounds of all the edges.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 */
void reset_lowerbounds(const instance *inst, CPXENVptr env, CPXLPptr lp);

#endif //HARD_FIXING_H