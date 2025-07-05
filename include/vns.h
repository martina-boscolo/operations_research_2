#ifndef VNS_H
#define VNS_H

#include "tsp.h"
#include "heuristics.h"

#define VNS "VNS"

// Default values: best performer
#define DEAULT_K 3      // Default kick type
#define DEFAULT_REPS 1  // Default repetition number

/**
 * VNS algorithm.
 * NOTE: The available kicks are 3-opt and 5-opt move.
 *  
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution that will contain the best solution between the input one and the best found one (input/output)
 * @param timelimit The time limit for the algorithm (input)
 */
void vns(const instance *inst, solution *sol, const double timelimit);

/**
 * Algorithm to modify the solution to escape the current local optima.
 *  
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to modify (input/output)
 * @param k The type of kick (input)
 * @param reps The number of k-opt moves that are performed as kick (input)
 */
void kick(const instance *inst, solution *sol, const int k, const int reps);

/**
 * Rearrange tour segments in the solution following this pattern:
 * A -> D -> !B -> E -> !C -> F (reorder from A-B-C-D-E-F to A-D-!B-E-!C-F)
 * NOTE: !X denotes that the segment X is revesed.
 * NOTE: This function assumes 0 <= idx1 < idx2 < idx3 < idx4 < idx5 < nnodes.
 * 
 * @param sol The solution to modify (input/output)
 * @param nnodes The number of node of the instance (input)
 * @param idx1 The first index (input)
 * @param idx2 The second index (input)
 * @param idx3 The third index (input)
 * @param idx4 The fourth index (input)
 * @param idx5 The fifth index (input)
 */
void fixed_five_opt_move(solution *sol, const int nnodes, int idx1, int idx2, int idx3, int idx4, int idx5);

/**
 * Compute the delta cost after the 5-opt move: 
 * - remove edges (idx1, idx1+1), (idx2, idx2+1), (idx3, idx3+1), (idx4, idx4+1), (idx4, idx4+1)
 * - add edges (idx1, idx3+1), (idx4, idx2), (idx1+1, idx4+1), (idx5, idx3), (idx2+1, idx5+1)
 * NOTE: idx1, idx2, idx3, idx4 and idx5 are the index of nodes on solution.visited_nodes not the actual nodes.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to modify (input/output)
 * @param idx1 The first index (input)
 * @param idx2 The second index (input)
 * @param idx3 The third index (input)
 * @param idx4 The fourth index (input)
 * @param idx5 The fifth index (input)
 * 
 * @return Cost of old edges minus cost of new edges
 */
double delta5(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3, const int idx4, const int idx5);

/**
 * Select three random different indices in [0, n) and return them sorted.
 *  
 * @param n The number of nodes in the instance (input)
 * @param idx1 The first index (output)
 * @param idx2 The second index (output)
 * @param idx3 The third index (output)
 */
void select_three_indices(const int n, int *idx1, int *idx2, int *idx3);

/**
 * Select five random different indices in [0, n) and return them sorted.
 *  
 * @param n The number of nodes in the instance (input)
 * @param idx1 The first index (output)
 * @param idx2 The second index (output)
 * @param idx3 The third index (output)
 * @param idx4 The fourth index (output)
 * @param idx5 The fifth index (output)
 */
void select_five_indices(const int n, int *idx1, int *idx2, int *idx3, int *idx4, int *idx5);

#endif //VNS_H