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
#include "hard_fixing.h"
#include "local_branching.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Initialize the solution with default values.
 * NOTE: This function doesn't allocate the memory.
 * 
 * @param sol The solution to initialize (output)
 */
void initialize_solution(solution *sol);

/**
 * Initialize array of nodes (ordered array).
 *  
 * @param visited_nodes The array of visited nodes to initialize (output)
 * @param nnodes The number of nodes (input)
 */
void initialize_tour(int *visited_nodes, const int nnodes);

/**
 * Select the method to solve the TSP and compute the solution.
 * NOTE: This function initializes the solution, allocates memory for it, and then calls the appropriate solving
 * 
 * @param inst The instance that contains the problem to solve (input)
 * @param sol The solution in which the result will be stored (output)
 */
void solve_with_method(instance *inst, solution *sol); 

/**
 * Check if all nodes are visited exactly once.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to check (input)
 * 
 * @return true if valid, false otherwise
 */
bool validate_node_visits(const instance *inst, const solution *sol);

/**
 * Compute the total cost of the given solution path.
 * NOTE: This function assumes that the solution is a valid tour
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution of which to compute the cost (input)
 * 
 * @return The cost of the solution
 */
double compute_solution_cost(const instance *inst, const solution *sol);

/**
 * Check if the cost in the solution is the expected one.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution whose cost to check (input)
 * 
 * @return true if valid, false otherwise
 */
bool validate_cost(const instance *inst, const solution *sol);

/**
 * Check the feasibility of the solution.
 * NOTE: If the solution is not feasible exit
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to check (input)
 */
void check_sol(const instance *inst, const solution *sol); 

/**
 * Check if the second solution is better than the first one, if so update the first solution
 * NOTE: Assumes that both solutions are valid tours, with correct costs.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol1 The first solution to update (input/output)
 * @param sol2 The solution to compare with the first one (input)
 * @param print If it is false it forces to NOT print independently from verbose parameter (input)
 * 
 * @return true if the first solution is updated, false otherwise
 */
bool update_sol(const instance *inst, solution *sol1, const solution *sol2, const bool print);

/**
 * Copy the second solution into the first solution.
 * NOTE: This function assumes that the first solution not initialized nor allocated.
 * 
 * @param sol1 The first solution to copy into (output)
 * @param sol2 The second solution to copy from (input)
 * @param nnodes The number of nodes of the problem (input)
 */
void copy_sol(solution *sol1, const solution *sol2, const int nnodes);

/**
 * Plot the solution using gnuplot.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to plot (input)
 */
void plot_solution(const instance *inst, const solution *sol);

/**
 * Print the solution
 * 
 * @param sol The solution to print (input)
 * @param nnodes The number of nodes (input)
 */
void print_solution(const solution *sol, const int nnodes);

/**
 * Allocate the memory for the most space-consuming attributes of solution.
 *  
 * @param sol The solution to allocate (output)
 * @param nnodes The number of nodes in the problem (input)
 */
void allocate_solution(solution *sol, const int nnodes);

/**
 * Free the most space-consuming attributes of solution from memory.
 * 
 * @param inst The solution to free (output)
 */
void free_solution(solution *sol);

#endif //UTILITIES_SOLUTION_H