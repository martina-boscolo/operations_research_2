#ifndef UTILITIES_SOLUTION_H
#define UTILITIES_SOLUTION_H

#include "tsp.h"
#include "plot.h"
#include "heuristics.h"
#include "vns.h"
#include "tabu_search.h"
#include "compare.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define VNS_REPS 5

/**
 * Initialize the solution with default values.
 * Note: this function doesn't allocate the memory.
 * 
 * @param sol The solution pointer
 */
void initialize_solution(solution *sol);

/**
 * Initialize array of nodes (ordered array) 
 *  
 * @param visited_nodes array of nodes
 * @param nnodes number of nodes
 */
void initialize_tour(int *visited_nodes, int nnodes);

/**
 * Select the method to solve the TSP
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void solve_with_method(instance *inst, solution *sol); 

/**
 * Check if all nodes are visited exactly once.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return true if valid, false otherwise.
 */
bool validate_node_visits(instance *inst, solution *sol);

/**
 * Compute the total cost of the given solution path.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return the cost of the solution
 */
double compute_solution_cost(instance *inst, solution *sol);

/**
 * Main function to check the feasibility of the solution.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return 1 if valid, 0 otherwise.
 */
void check_sol(instance *inst, solution *sol); 

/**
 * Plot the solution using gnuplot
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void plot_solution(instance *inst, solution *sol);

/**
 * Print the hyperparameter of the solution
 * 
 * @param sol The solution pointer of the problem
 * @param nnodes The number of nodes
 */
void print_solution(solution *sol, int nnodes);

/**
 * Initialize sol, setting cost to INF and allocating visited_nodes 
 *  
 * @param sol The solution pointer of the instance
 * @param nnodes The number of nodes
 */
void allocate_solution(solution *sol, int nnodes);

/**
 * Deallocates the most space-consuming attributes of solution from memory
 * 
 * @param inst The solution pointer of the problem
 */
void free_solution(solution *sol);

#endif //UTILITIES_SOLUTION_H