#ifndef LOCAL_BRANCHING_H
#define LOCAL_BRANCHING_H
#include "tsp.h"
#include "utilities_solution.h"
#include "utilities.h"
#include "tsp_cplex.h"

#define LOCAL_BRANCHING "LB"

/**
 * Local branching algorithm.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution that will contain the best solution between the input one and the best found one (input/output)
 * @param timelimit The time limit for the algorithm (input)
 */
void local_branching(instance *inst, solution *sol, const double timelimit);

/**
 * Set the local branching constraint in the CPLEX model.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 * @param k The neighborhood size (input)
 */
void add_local_branching_constraint(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp, const int k);

/**
 * Remove the local branching constraint in the CPLEX model.
 * 
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 */
void remove_local_branching_constraint(CPXENVptr env, CPXLPptr lp) ;

#endif // LOCAL_BRANCHING_H