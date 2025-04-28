#ifndef BRANCH_AND_CUT_H
#define BRANCH_AND_CUT_H

#include "tsp.h"
#include "tsp_cplex.h"
#include "utilities.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ilcplex/cplex.h>

#define BRANCH_AND_CUT "BC"

void branch_and_cut(instance *inst, solution *sol, const double timelimit);

/**
 * Install the callback for the branching tree
 * 
 * @param inst The instance pointer of the problem
 * @param env CPLEX environment
 * @param lp CPLEX LP
 */
void install_callback(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Lazy callback: when a candidate is found check if it is feasible;
 * if not, reject it and add the correspondent SECs in the pool
 * 
 * @param context Context of node of branching tree
 * @param contextid ID of when the function is called
 * @param userhandle The instance pointer of the problem
 */
int CPXPUBLIC lazy_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * Add the SECs in the pool
 * 
 * @param inst The instance pointer of the problem
 * @param context Context of node of branching tree
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 * @param tree_node Index of node for which the function is called
 * 
 * @return 0 if the SECs are added correctly, 1 otherwise
 */
int add_SECs_to_pool(const instance *inst, CPXCALLBACKCONTEXTptr context, const int *comp, const int ncomp, const int tree_node);

/**
 * If an integer non-feasible solution is found use patch heuristic and post it
 * 
 * @param inst The instance pointer of the problem
 * @param context Context of node of branching tree
 * @param succ Solution as successors
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 * @param timelimit Time limit for the algorithm
 * 
 * @return 0 if the solution is posted correctly, 1 otherwise
 */
int post_heuristic(const instance *inst, CPXCALLBACKCONTEXTptr context, int *succ, int *comp, int ncomp, const double timelimit);

#endif // BRANCH_AND_CUT_H