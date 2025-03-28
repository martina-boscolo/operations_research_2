#ifndef VNS_H
#define VNS_H

#include "tsp.h"
#include "heuristics.h"

#define VNS "VNS"

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

/**
 * Select three random different indices and return them sorted
 *  
 * @param n number of nodes of the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 */
void select_three_indices(int n, int *idx1, int *idx2, int *idx3);

#endif //VNS_H