#ifndef MATHEURISTIC_H
#define MATHEURISTIC_H

#include "tsp.h"
#include "utilities_solution.h"
#include "utilities.h"
#include "tsp_cplex.h"

#define HF "HF"

/**
 * Hard fixing algorithm
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param timelimit Time limit for the algorithm
 */
void hard_fixing(instance *inst, solution *sol, const double timelimit);

/**
 * Fix p*(inst->nnodes) edges of the solution in the model lp
 * At the end 
 * - fixed_count will contain the number of actual fixed edges,
 * - edge_indices will contain the indeces (obtained with xpos()) of fixed edges
 * - lu will contain 'L' at the first fixed_count cells
 * - bd will contain 1.0 at the first fixed_count cells
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * 
 * @param p Percentage to fix the edges
 * @param fixed_count Number of fixed edges
 * @param edge_indeces Indeces of fixed edges
 * @param lu Array of which bound is fixed
 * @param bd Array of bounds
 */
void set_lowerbounds(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp, 
                     const double p, int *fixed_count, int *edge_indices, char *lu, double *bd);

/**
 * Reset the lower boundsof the edges, whose index is stored in the first fixed_count 
 * cells of edge_indeces in the model lp
 * At the end 
 * - lu will contain 'L' at the first fixed_count cells
 * - bd will contain 0.0 at the first fixed_count cells
 * 
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * 
 * @param fixed_count Number of fixed edges
 * @param edge_indeces Indeces of fixed edges
 * @param lu Array of which bound is fixed
 * @param bd Array of bounds
 */
void reset_lowerbounds(CPXENVptr env, CPXLPptr lp, const int fixed_count, const int *edge_indices, char *lu, double *bd);

#endif //MATHEURISTIC_H