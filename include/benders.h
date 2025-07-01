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
 * Benders loop algorithm.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution that will contain the best solution between the input one and the best found one (input/output)
 * @param timelimit The time limit for the algorithm (input)
 */
void benders_loop(instance *inst, solution *sol, const double timelimit);

/**
 * For each connected component add the correspondent SEC to the lp model.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX LP (input/output)
 * @param comp The array that contains the component associated for each nodes (input)
 * @param ncomp The number of components in the solution (input)
 * @param iter The iteration number (input)
 */
void add_SECs_to_model(const instance *inst, CPXENVptr env, CPXLPptr lp, const int *comp, const int ncomp, const int iter);

/**
 * Build a feasible solution from different subtours connecting them.
 * NOTE: The solution computed is 2-opt refined.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to build (output)
 * @param succ The solution as successors (input/output)
 * @param comp The array with component associated for each nodes (input/output)
 * @param ncomp The number of components in the solution (input)
 * @param timelimit The time limit for the algorithm (input)
 */
void patch_heuristic(const instance *inst, solution *sol, int *succ, int *comp, int ncomp, const double timelimit);

/**
 * Compute the delta cost removing edges (i, succ_i), (j,succ_j) and adding the edges (i,succ_j), (j,succ_i)
 *
 * @param i The first index (input)
 * @param j The second index (input)
 * @param inst The instance that contains the problem data (input)
 * @param succ The solution as successors (input)
 */
double delta_dir(const int i, const int j, const instance *inst, const int *succ);

/**
 * Compute the delta cost removing edges (i, succ_i), (j,succ_j) and adding the edges (i,j), (succ_j,succ_i)
 *
 * @param i The first index (input)
 * @param j The second index (input)
 * @param inst The instance that contains the problem data (input)
 * @param succ The solution as successors (input)
 */
double delta_rev(const int i, const int j, const instance *inst, const int *succ);

/**
 * Reverse the order of the subtour that contains node i in succ 
 * 
 * @param i The node in the subtour (input)
 * @param succ The solution as successors (input/output)
 */
void reverse_succ(const int i, int *succ);

/**
 * Update the connected component such that the (sub)tour that contains i figures as connected component numbered new_comp.
 * NOTE: i must be the first node in the (sub)tour with different connected component number.
 * 
 * @param i The first node in the (sub)tour (input)
 * @param new_comp The number of connected component to assign (input)
 * @param succ The solution as successors (input)
 * @param comp The array with component associated for each nodes (input/output)
 */
void update_comp(int i, const int new_comp, const int *succ, int *comp);

/**
 * Function to extract subtours from the successors array.
 * NOTE: This function assumes that the arrays are correctly allocated.
 * NOTE: only the first ncomp arrays of subtours will be allocated.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param succ The solution as successors (input)
 * @param subtours The array in which each element will be an array with a subtour (output)
 * @param subtour_lengths The array that will contain the length of each subtour (output)
 * @param ncomp The number of connected components (output)
 */
void extract_subtours_from_successors(const instance *inst, const int *succ, int **subtours, int *subtour_lengths, int *ncomp);

#endif //BENDERS_H