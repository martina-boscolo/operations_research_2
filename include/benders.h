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
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param timelimit Time limit for the algorithm
 */
void benders_loop(instance *inst, solution *sol, const double timelimit);

/**
 * 
 */
void patch_heuristic(instance *inst, int *succ, int *comp, int ncomp);

/**
 * 
 */
double delta_dir(const int i, const int j, const instance *inst, const int *succ);

/**
 * 
 */
double delta_rev(const int i, const int j, const instance *inst, const int *succ);

/**
 * 
 */
void reverse_succ(const int i, int *succ);

/**
 * 
 */
void update_comp(int i, const int new_comp, const int *succ, int *comp);

/**
 * Function to extract subtours from the successors array
 */
void extract_subtours_from_successors(const instance *inst, int *succ,
    int ***subtours_out, int **subtour_lengths_out, int *ncomp_out);

#endif //BENDERS_H