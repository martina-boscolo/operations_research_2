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
 */
void vns(instance *inst, solution *sol, const int reps);

/**
 * Algorithm to modify the solution to escape the current local optima
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void kick(instance *inst, solution *sol, const int reps);

/**
 * Select three random different indices and return them sorted
 *  
 * @param n number of nodes of the instance
 * @param idx1 First index
 * @param idx2 Second index
 * @param idx3 Third index
 * 
 */
void select_three_indices(int n, int *idx1, int *idx2, int *idx3);

#endif //VNS_H