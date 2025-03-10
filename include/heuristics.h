#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "tsp.h"
#include "utilities.h"

#include <math.h>
#include <time.h>
#include <float.h>

#define INF DBL_MAX

// available methods to solve the tsp
/*enum Heuristic { // add all?
    BASE,               // Ordered nodes
    NEAREST_NEIGHBOR,   // Nearest Neighbor
    TWO_OPT             // 2-opt refinement
};

// Strings for command line
const char *command[] = {[BASE] = "BASE", 
                         [NEAREST_NEIGHBOR] = "NN",
                         [TWO_OPT] = "2OPT"};

// Strings with methods' name
const char *method[] = {[BASE] = "Base", 
                        [NEAREST_NEIGHBOR] = "Nearest Neighbor",
                        [TWO_OPT] = "2-opt refinement"};*/

#define BASE "BASE"
#define NEAREST_NEIGHBOR "NN"
#define TWO_OPT "2OPT"

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



//---------------------------------------- heuristics ----------------------------------------

//--- BASE ---

/**
 * The solution is the ordered sequence of nodes
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void make_base_solution(instance *inst, solution *sol);


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
int ms_2opt_nn_main(instance *inst, solution *sol); 

#endif //HEURISTICS_H