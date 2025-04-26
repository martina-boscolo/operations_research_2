#ifndef BENDERS_H
#define BENDERS_H

#include "tsp.h"
#include "tsp_cplex.h"
#include "utilities.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ilcplex/cplex.h>

#define BENDERS "B"

/**
 * Find the optimal solution of the instance
 * Note: If the time limit is reached the solution might not be optimal
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param timelimit Time limit for the algorithm
 */
void benders_loop(const instance *inst, solution *sol, const double timelimit);

/**
 * For each connected component add the correspondent SEC to the lp model
 * 
 * @param inst The instance pointer of the problem
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 * @param iter Iteration
 */
void add_SECs_to_model(const instance *inst, CPXENVptr env, CPXLPptr lp, const int *comp, const int ncomp, const int iter);

/**
 * Build a feasible solution from different subtours connecting them and then apply 2-opt refinement
 * 
 * @param inst The instance pointer of the problem
 * @param succ Solution as successors
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 * @param timelimit Time limit for the algorithm
 */
void patch_heuristic(const instance *inst, solution *sol, int *succ, int *comp, int ncomp, const double timelimit);

/**
 * Compute the delta cost removing edges (i, succ_i), (j,succ_j) and adding the edges (i,succ_j), (j,succ_i)
 *
 * @param i First index
 * @param j Second index
 * @param inst The instance pointer of the problem
 * @param succ Solution as successors
 */
double delta_dir(const int i, const int j, const instance *inst, const int *succ);

/**
 * Compute the delta cost removing edges (i, succ_i), (j,succ_j) and adding the edges (i,j), (succ_j,succ_i)
 *
 * @param i First node
 * @param j Second node
 * @param inst The instance pointer of the problem
 * @param succ Solution as successors
 */
double delta_rev(const int i, const int j, const instance *inst, const int *succ);

/**
 * Reverse the order of the subtour that contains node i in succ 
 * 
 * @param i Node in the subtour
 * @param succ Solution as successors
 */
void reverse_succ(const int i, int *succ);

/**
 * Update the connected components such that the (sub)tour that contains i figures as connected component numbered new_comp
 * Note: Use this function once you have connect two subtours
 * Note: i must be the first node in the (sub)tour with different connected component number
 * 
 * @param i Node in the (sub)tour
 * @param new_comp Number of connected component to assign
 * @param succ Solution as successors
 * @param comp Component associated for each nodes
 */
void update_comp(int i, const int new_comp, const int *succ, int *comp);

/**
 * Function to extract subtours from the successors array
 */
void extract_subtours_from_successors(const instance *inst, int *succ,
    int ***subtours_out, int **subtour_lengths_out, int *ncomp_out);

#endif //BENDERS_H