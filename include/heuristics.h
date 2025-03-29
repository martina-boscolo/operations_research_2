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

// heuristics in the file
#define NEAREST_NEIGHBOR "NN"
#define TWO_OPT "TWO_OPT"
#define NN_TWOOPT "NN_2OPT"

//----------------------------------- heuristic utilities ------------------------------------

/**
 * Find the next nearest node 
 *  
 * @param inst The instance pointer of the problem
 * @param len current length of the explored part of the array
 * @param visited_nodes array of node, where until length is explored
 * 
 * @return index of nearest node, -1 if there is no nearest node
 */
int find_nearest_node(const instance *inst, const int len, const int *visited_nodes);

/**
 * Swaps two nodes in the array 
 *  
 * @param nodes array of nodes
 * @param i first index 
 * @param j second index
 */
void swap_nodes(int *nodes, const int i, const int j);

/**
 * Reverse the segment between the two indeces in the solution
 * 
 * @param sol The solution pointer 
 * @param i The first index 
 * @param j The last index
 */
void reverse_segment(solution *sol, const int i, const int j);

/**
 * Rearrange tour segments in the solution following this pattern:
 * A -> C -> B (reorder from A-B-C to A-C-B)
 * Note: assuming 0 <= idx1 < idx2 < idx3 < nnodes
 * 
 * @param sol The solution pointer of the instance
 * @param n The number of nodes in the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 */
void shift_segment(solution *sol, const int n, const int idx1, const int idx2, const int idx3);

/**
 * Compute the delta cost after the 2-opt move: 
 * - remove edges (i-1, i), (j, j+1)
 * - add edges (i-1, j),(i, j+1)
 * Note: i and j are the index of nodes on solution.visited_nodes not the actual nodes
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param i First index
 * @param j Second index
 * 
 * @return cost of old edges minus cost of new edges
 */
double delta2(const instance *inst, const solution *sol, const int i, const int j);

/**
 * Compute the delta cost after the 3-opt move: 
 * - remove edges (idx1, idx1+1), (idx2, idx2+1), (idx3, idx3+1)
 * - add edges (idx1, idx2+1), (idx2, idx3+1), (idx3, idx1+1)
 * Note: idx1, idx2 and idx3 are the index of nodes on solution.visited_nodes not the actual nodes
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 */
double delta3(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3);

//---------------------------------------- heuristics ----------------------------------------

//--- NEAREST NEIGHBOR ---

/**
 * Nearest Neighbor algorithm 
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance 
 *            Note: at the end it will contain the NN's solution
 * @param start Starting node
 */
void nearest_neighbor(const instance *inst,  solution *sol, const int start);

/**
 * Multi-start approach for nearest neighbor
 * The NN's solutions are refined using 2-opt
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 *            Note: at the end it will contain the best solution between the input one and the best found one.
 * @param timelimit Time limit for the algorithm
 */
void multi_start_nn(const instance *inst, solution *sol, const double timelimit);

//--- k-OPT REFINEMENT ---

/**
 * Implementation of 2-opt for refinement of the solution
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param timelimit Time limit for the algorithm
 */
void two_opt(const instance *inst, solution *sol, const double timelimit, bool print); 

#endif //HEURISTICS_H