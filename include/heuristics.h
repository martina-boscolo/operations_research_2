#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "tsp.h"
#include "utilities.h"

#include <math.h>
#include <time.h>
#include <float.h>

#define INF DBL_MAX

/**
 * Function to calculate Euclidean distance 
 *  
 * @param a first node
 * @param b second node
 */
double euclidean_distance(coordinate a, coordinate b);

/**
 * Nearest Neighbor algorithm 
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param start Starting node
 */
void nearest_neighbor(instance *inst,  solution *sol, int start);

/**
 * Multi-start approach with time limit
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void multi_start_nn(instance *inst,  solution *sol);

/**
 * Implementation of 2-opt for refinement of the solution
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void two_opt(instance *inst, solution *sol); 

/**
 * main for Nearest Neighbour
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
int nn_main(instance *inst,  solution *sol); 

#endif //HEURISTICS_H