#ifndef VNS_H
#define VNS_H

#include "tsp.h"
#include "heuristics.h"

#define VNS "VNS"

#define DEAULT_K 3 
#define DEFAULT_REPS 1
#define POSSIBLE_THREE_OPT_MOVES 4

/**
 * VNS algorithm 
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 *            Note: at the end it will contain the best solution between the input one and the best found one
 * @param timelimit Time limit for the algorithm
 * @param k if 3 it will use a fixed 3-opt move as kick, if 5 it will use a fixed 5-opt move as kick
 * @param reps number of k-opt moves that are performed as kick
 */
void vns(const instance *inst, solution *sol, const double timelimit, const int k, const int reps);

/**
 * Algorithm to modify the solution to escape the current local optima
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param k if 3 it will use a fixed 3-opt move as kick, if 5 it will use a fixed 5-opt move as kick
 * @param reps number of k-opt moves that are performed as kick
 */
void kick(const instance *inst, solution *sol, const int k, const int reps);

void fixed_three_opt_move1(const instance *inst, solution *sol, int idx1, int idx2, int idx3);
void fixed_three_opt_move2(const instance *inst, solution *sol, int idx1, int idx2, int idx3);
void fixed_three_opt_move3(const instance *inst, solution *sol, int idx1, int idx2, int idx3);
void fixed_three_opt_move4(const instance *inst, solution *sol, int idx1, int idx2, int idx3);

/**
 * Rearrange tour segments in the solution following this pattern:
 * A -> D -> !B -> E -> !C -> F (reorder from A-B-C-D-E-F to A-D-!B-E-!C-F)
 * !X denotes that the segment X is revesed
 * Note: assuming 0 <= idx1 < idx2 < idx3 < idx4 > idx5 < nnodes
 * 
 * @param sol The solution pointer of the instance
 * @param nnodes The number of node of the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 * @param idx4 Fourth index
 * @param idx5 Fifth index
 */
void fixed_five_opt_move(solution *sol, const int nnodes, int idx1, int idx2, int idx3, int idx4, int idx5);

/**
 * Compute the delta cost after the 3-opt move: 
 * - remove edges (idx1, idx1+1), (idx2, idx2+1), (idx3, idx3+1), (idx4, idx4+1), (idx4, idx4+1)
 * - add edges (idx1, idx3+1), (idx4, idx2), (idx1+1, idx4+1), (idx5, idx3), (idx2+1, idx5+1)
 * Note: idx1, idx2, idx3, idx4 and idx5 are the index of nodes on solution.visited_nodes not the actual nodes
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 * @param idx4 Fourth index
 * @param idx5 Fifth index
 */
double delta5(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3, const int idx4, const int idx5);

/**
 * Select three random different indices in [0, n) and return them sorted
 *  
 * @param n number of nodes of the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 */
void select_three_indices(const int n, int *idx1, int *idx2, int *idx3);

/**
 * Select five random different indices in [0, n) and return them sorted
 *  
 * @param n number of nodes of the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 * @param idx4 Fourth index
 * @param idx5 Fifth index
 */
void select_five_indices(const int n, int *idx1, int *idx2, int *idx3, int *idx4, int *idx5);

#endif //VNS_H