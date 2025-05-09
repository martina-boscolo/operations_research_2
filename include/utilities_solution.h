#ifndef UTILITIES_SOLUTION_H
#define UTILITIES_SOLUTION_H

#include "tsp.h"
#include "plot.h"
#include "heuristics.h"
#include "vns.h"
#include "tabu_search.h"
#include "tsp_cplex.h"
#include "benders.h"
#include "branch_and_cut.h"
#include "matheuristic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
void initialize_tour(int *visited_nodes, const int nnodes);

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
 * @return true if valid, false otherwise
 */
bool validate_node_visits(const instance *inst, const solution *sol);

/**
 * Compute the total cost of the given solution path.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return the cost of the solution
 */
double compute_solution_cost(const instance *inst, const solution *sol);

/**
 * Check if the cost in the solution is the expected one
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return true if valid, false otherwise
 */
bool validate_cost(const instance *inst, const solution *sol);

/**
 * Main function to check the feasibility of the solution
 * If the solution is not feasible exit
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void check_sol(const instance *inst, solution *sol); 

/**
 * Check if the second solution is better than the first one,
 * if so update the first solution
 * 
 * @param inst The instance pointer of the problem
 * @param sol1 The first solution pointer of the instance
 * @param sol2 The second solution pointer of the instance
 * @param print If it is false it forces to NOT print independently from verbose parameter
 */
void update_sol(const instance *inst, solution *sol1, const solution *sol2, bool print);

/**
 * Copy the second solution into the first solution
 * 
 * @param sol1 The first solution pointer
 * @param sol2 The second solution pointer
 * @param nnodes The number of nodes in the instance
 */
void copy_sol(solution *sol1, const solution *sol2, const int nnodes);

/**
 * Plot the solution using gnuplot
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void plot_solution(const instance *inst, const solution *sol);

/**
 * Print the hyperparameter of the solution
 * 
 * @param sol The solution pointer of the problem
 * @param nnodes The number of nodes
 */
void print_solution(const solution *sol, const int nnodes);

/**
 * Initialize sol, setting cost to INF and allocating visited_nodes 
 *  
 * @param sol The solution pointer of the instance
 * @param nnodes The number of nodes
 */
void allocate_solution(solution *sol, const int nnodes);

/**
 * Deallocates the most space-consuming attributes of solution from memory
 * 
 * @param inst The solution pointer of the problem
 */
void free_solution(solution *sol);

#endif //UTILITIES_SOLUTION_H