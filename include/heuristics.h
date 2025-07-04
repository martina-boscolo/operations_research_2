#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "tsp.h"
#include "utilities.h"
#include "utilities_instance.h"
#include "utilities_solution.h"

#include <math.h>
#include <time.h>
#include <float.h>
#include <stdbool.h>

#define INF DBL_MAX

// Heuristics in the file
#define NEAREST_NEIGHBOR "NN"
#define TWO_OPT "TWO_OPT"
#define MULTI_START_NN "MS_NN"
#define EXTRA_MILEAGE "EM"

//----------------------------------- heuristic utilities ------------------------------------

/**
 * Finds the nearest node in the array of visited nodes, i.e. the one with the minimum cost from the last visited node.
 * NOTE: The last visited node is the one at index len-1.
 * NOTE: The only nodes considered are those that have not been visited yet (from index len to nnodes-1).
 *  
 * @param inst The instance that contains the problem data (input)
 * @param len The current length of the explored part of the array (input)
 * @param visited_nodes The array of nodes (input)
 * 
 * @return The index of nearest node, -1 if there is no nearest node
 */
int find_nearest_node(const instance *inst, const int len, const int *visited_nodes);

/**
 * Swaps two nodes in the array at the given indices.
 * NOTE: This finction assumes that the indices are within the bounds of the array. 
 *  
 * @param nodes The array of nodes (input/output)
 * @param i The first index (input)
 * @param j The second index (input)
 */
void swap_nodes(int *nodes, const int i, const int j);

/**
 * Reverse the segment between the two indeces in the solution, i.e. the nodes between i and j (inclusive) are reversed.
 * NOTE: This function assumes that the indices are within the bounds of the array, and i <= j.
 * 
 * @param sol The solution where the segment is reversed (input/output)
 * @param i The first index (input)
 * @param j The last index (input)
 */
void reverse_segment(solution *sol, const int i, const int j);

/**
 * Reorder tour segments in the solution following this pattern:
 * A -> C -> B (reorder from A -> B -> C to A -> C -> B).
 * NOTE: This function assumes 0 <= idx1 < idx2 < idx3 < nnodes.
 * 
 * @param sol The solution where the nodes are reordered (input/output)
 * @param n The number of nodes in the instance (input)
 * @param idx1 The first index (input)
 * @param idx2 The second index (input)
 * @param idx3 The third index (input)
 */
void shift_segment(solution *sol, const int n, const int idx1, const int idx2, const int idx3);

/**
 * Compute the delta cost after the 2-opt move: 
 * - remove edges (i-1, i), (j, j+1)
 * - add edges (i-1, j),(i, j+1)
 * NOTE: i and j are the index of nodes on solution.visited_nodes not the actual nodes.
 * 
 * @param inst The instance that contains the problem to solve (input)
 * @param sol The solution that contains the tour (input)
 * @param i The first index (input)
 * @param j The second index (input)
 * 
 * @return Cost of old edges minus cost of new edges
 */
double delta2(const instance *inst, const solution *sol, const int i, const int j);

/**
 * Compute the delta cost after the 3-opt move: 
 * - remove edges (idx1, idx1+1), (idx2, idx2+1), (idx3, idx3+1)
 * - add edges (idx1, idx2+1), (idx2, idx3+1), (idx3, idx1+1)
 * NOTE: idx1, idx2 and idx3 are the index of nodes on solution.visited_nodes not the actual nodes.
 * 
 * @param inst The instance that contains the problem to solve (input)
 * @param sol The solution that contains the tour (input)
 * @param idx1 The first index (input)
 * @param idx2 The second index (input)
 * @param idx3 The third index (input)
 */
double delta3(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3);

//---------------------------------------- heuristics ----------------------------------------

//--- NEAREST NEIGHBOR ---

/**
 * Nearest Neighbor algorithm.
 * NOTE: The algorithm will use start as starting point.
 *  
 * @param inst The instance that contains the problem to solve (input)
 * @param sol The solution that will contain the NN's solution (output)
 * @param start The starting node (input)
 */
void nearest_neighbor(const instance *inst,  solution *sol, const int start);

/**
 * Multi-start approach for nearest neighbor.
 * NOTE: The NN's solutions are refined using 2-opt.
 *  
 * @param inst The instance that contains the problem to solve (input)
 * @param sol The solution that will contain the best solution between the input one and the best found one (input/output)
 * @param timelimit The time limit for the algorithm (input)
 */
void multi_start_nn(const instance *inst, solution *sol, const double timelimit);

//--- EXTRA MILEAGE ---

/**
 * Extra Mileage algorithm.
 * 
 * @param inst The instance that contains the problem to solve (input)
 * @param sol The solution that will contain the EM's solution (output)
 */
void extra_mileage(const instance *inst, solution *sol);

//--- k-OPT REFINEMENT ---

/**
 * Implementation of 2-opt for refinement of the solution.
 *  
 * @param inst The instance that contains the problem to solve (input)
 * @param sol The solution to refine (input/output)
 * @param timelimit The time limit for the algorithm
 */
void two_opt(const instance *inst, solution *sol, const double timelimit, bool print); 

#endif //HEURISTICS_H