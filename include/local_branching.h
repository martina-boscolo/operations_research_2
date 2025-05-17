#ifndef LOCAL_BRANCHING_H
#define LOCAL_BRANCHING_H
#include "tsp.h"
#include "utilities_solution.h"
#include "utilities.h"
#include "tsp_cplex.h"

#define LB "LB"
/**
 * Local branching algorithm
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param timelimit Time limit for the algorithm
 */
void local_branching(instance *inst, solution *sol, const double timelimit);

/**
 * Set the local branching constraint in the CPLEX model
 * 
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * @param constraint_index Index of the constraint to be removed
 */
void remove_local_branching_constraint(CPXENVptr env, CPXLPptr lp, const int constraint_index) ;

/**
 * Set the local branching constraint in the CPLEX model
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * @param k Neighborhood size
 * @param indices Array of indices for the constraint
 * @param values Array of values for the constraint
 */
int add_local_branching_constraint(const instance *inst, const solution *sol, 
                                  CPXENVptr env, CPXLPptr lp, const int k,
                                  int *indices, double *values) ;
                                  

#endif // LOCAL_BRANCHING_H