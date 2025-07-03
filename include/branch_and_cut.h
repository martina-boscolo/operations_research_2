#ifndef BRANCH_AND_CUT_H
#define BRANCH_AND_CUT_H

#include "tsp.h"
#include "tsp_cplex.h"
#include "utilities.h"
#include "heuristics.h"
#include "../concorde/mincut.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ilcplex/cplex.h>

#define BRANCH_AND_CUT "BC"

// Struct to hold callback data
typedef struct {

    CPXCALLBACKCONTEXTptr context; // The context of node of branching tree
    const instance *inst;          // The instance that contains the problem data

} cut_callback_data;

/**
 * Branch and Cut algorithm.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution that will contain the best solution between the input one and the best found one (input/output)
 * @param timelimit The time limit for the algorithm (input)
 */
void branch_and_cut(instance *inst, solution *sol, const double timelimit);

/**
 * Install the callback for the branching tree.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 */
void install_callback(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Callback for the branching tree.
 * 
 * @param context The context of node of branching tree (input/output)
 * @param contextid The context in which the callback is invoked (input)
 * @param userhandle The instance that contains the problem data (input)
 */
static int CPXPUBLIC callback_branch_and_cut(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * Lazy callback: when a candidate is found check if it is feasible;
 * if not, reject it and add the correspondent SECs in the pool.
 * 
 * @param context The context of node of branching tree (input/output)
 * @param contextid The context in which the callback is invoked (input)
 * @param userhandle The instance that contains the problem data (input)
 * 
 * @return 0 if no error appeared
 */
static int CPXPUBLIC lazy_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * Relaxation callback: add fractional SEC's.
 * 
 * @param context The context of node of branching tree (input/output)
 * @param contextid The context in which the callback is invoked (input)
 * @param userhandle The instance that contains the problem data (input)
 * 
 * @return 0 if the SECs are added correctly
 */
static int CPXPUBLIC relaxation_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * Add the SEC's in the pool.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param context The context of node of branching tree (input)
 * @param comp The array with component associated for each nodes (input)
 * @param ncomp The number of components in the solution (input)
 * @param tree_node The index of node for which the function is called (input)
 */
void add_SECs_to_pool(const instance *inst, CPXCALLBACKCONTEXTptr context, const int *comp, const int ncomp, const int tree_node);

/**
 * If an integer non-feasible solution is found use patch heuristic and post it.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param context The context of node of branching tree (input/output)
 * @param succ The solution as successors (input/output)
 * @param comp The array with component associated for each nodes (input/output)
 * @param ncomp The number of components in the solution (input)
 * @param timelimit The time limit for the algorithm (input)
 */
void post_heuristic(const instance *inst, CPXCALLBACKCONTEXTptr context, int *succ, int *comp, int ncomp, const double timelimit);

/**
 * Add the violated SEC from a fractional solution to the user's cuts.
 * 
 * @param cutcount The number of nodes in the subtour that violates the constraint (input)
 * @param cutlist Array containing the indices of nodes in the violating subtour (input)
 * @param pass_param The instance data and the context of node of branching tree (input)
 * 
 * @return 0 if the SEC is added correctly
 */
static int add_violated_sec(int cutcount, int *cutlist, void *pass_param);

/**
 * Extract the SEC's violated by a fractional solution and add them to the user's cuts.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param context The context of node of branching tree (input)
 * @param xstar The fractional solution (inout)
 */
void add_violated_cuts_to_model(const instance *inst, CPXCALLBACKCONTEXTptr context, double *xstar);

#endif // BRANCH_AND_CUT_H