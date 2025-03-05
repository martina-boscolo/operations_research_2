#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "tsp.h"

#include <math.h>
#include <time.h>
#include <float.h>

#define INF DBL_MAX

/**
 * Function to calculate Euclidean distance 
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
double euclidean_distance(coordinate a, coordinate b);

/**
 * Nearest Neighbor algorithm 
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void nearest_neighbor(instance *inst,  solution *sol, int start);

/**
 * Multi-start approach with time limit
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void multi_start_nn(instance *inst,  solution *sol);

int nn_main(instance *inst,  solution *sol); 

#endif //HEURISTICS_H