#ifndef TSP_CPLEX_H
#define TSP_CPLEX_H

#include "tsp.h"
#include "utilities_instance.h"

#include <ilcplex/cplex.h>
#include <stdbool.h>

#define CPLEX "C"

/**
 Optimizer.

 @param inst instance of the struct "instance" for TSP problem.
 @param timelimit 
 @return 0 if successful, otherwise 1.
 */
int TSPopt(instance *inst, const double timelimit);


/**
 Return the index of CPLEX solution array from two subsequent nodes..

 @param i first node.
 @param j second node.
 @param inst instance of the struct "instance" for TSP problem.
 @return array position.
 */
int xpos(int i, int j, instance *inst);                                       

    
/**
 Build the model.

 @param inst instance of the struct "instance" for TSP problem.
 @param env CPLEX environment.
 @param lp CPLEX LP.
 */
void build_model_CPLEX(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 Build succ() and comp() wrt xstar().

 @param xstar CPLEX solution.
 @param inst instance of the struct "instance" for TSP problem.
 @param succ TSP solution as successors.
 @param comp component associated for each nodes.
 @param ncomp number of components in the solution.
 */
void build_cplex_sol(const double *xstar, instance *inst, int *succ, int *comp, int *ncomp);

void build_solution(instance *inst, solution *sol, int *succ);

#endif //TSP_CPLEX_H