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
void vns(instance *inst, solution *sol);

/**
 * Algorithm to modify the solution to escape the current local optima
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void kick(instance *inst, solution *sol);

#endif //VNS_H