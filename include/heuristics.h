#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "tsp.h"
#include "utilities.h"
#include "utilities_instance.h"
#include "utilities_solution.h"

#include <math.h>
#include <time.h>
#include <float.h>

#define INF DBL_MAX

// heuristics in the file
#define NEAREST_NEIGHBOR "NN"
#define TWO_OPT "TWO_OPT"

//----------------------------------- heuristic utilities ------------------------------------

/**
 * Find the next nearest node 
 *  
 * @param inst The instance pointer of the problem
 * @param len current length of the explored part of the array
 * @param visited_nodes array of node, where until length is explored
 */
int find_nearest_node(instance *inst, int len, int *visited_nodes);

/**
 * Swaps two nodes in the array 
 *  
 * @param nodes array of nodes
 * @param i first index 
 * @param j second index
 */
void swap_nodes(int *nodes, int i, int j);

/**
 * Reverse the segment between the two indeces
 * 
 * @param sol The solution pointer 
 * @param i The first index 
 * @param j The second index
 */
void reverse_segment(solution *sol, int i, int j);

/**
 * Shift segment between index i and index j, after index k
 * Note: assumption k < i < j
 * 
 * @param sol The solution pointer 
 * @param i The first index
 * @param j The second index
 * @param k The third index
 */
void shift_segment(solution *sol, int i, int j, int k);


//---------------------------------------- heuristics ----------------------------------------

//--- NEAREST NEIGHBOR ---

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
void multi_start_nn(instance *inst, solution *sol);

/**
 * main for Nearest Neighbour
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
int ms_2opt_nn_main(instance *inst, solution *sol); 


//--- 2-opt refinement ---

/**
 * Implementation of 2-opt for refinement of the solution
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void two_opt(instance *inst, solution *sol); 

#endif //HEURISTICS_H