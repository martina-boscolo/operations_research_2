#ifndef TABU_SEARCH_H
#define TABU_SEARCH_H

#include "tsp.h"
#include "heuristics.h"

#include <float.h>
#include <math.h>

#define TABU_SEARCH "TS"


#include "tabu_search.h"
#include <float.h>
#include <math.h>

// Structure to hold tabu search parameters
typedef struct {
    int *tabu_list;         // Tabu list: -1 if not tabu, otherwise iteration when it became tabu
    int min_tenure;         // Minimum tabu tenure
    int max_tenure;         // Maximum tabu tenure
    int current_iter;       // Current iteration
} tabu_params;


/**
 * Initialize tabu search parameters
 *  
 * @param params 
 * @param nnodes 
 * @param min_tenure 
 * @param max_tenure 
 */
void init_tabu_params(tabu_params *params, int nnodes, int min_tenure, int max_tenure);

/**
 * Free tabu search parameters
 *  
 * @param params 
 */
void free_tabu_params(tabu_params *params);

/**
 * Get a random number between min and max
 *  
 * @param params 
 * @param node 
 */
int get_random_tenure(int min, int max);

/**
 * Check if a node is tabu
 *  
 * @param params 
 * @param node 
 */
int is_tabu(tabu_params *params, int node, int current_iter);

/**
 * Update tabu status for a node
 *  
 * @param params 
 * @param node 
 */
void update_tabu_status(tabu_params *params, int node);

/**
 * Find best neighbor using swap operator and considering tabu status
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void find_best_neighbor(instance *inst, solution *current, tabu_params *params);


/**
 * Main tabu search procedure
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void tabu_search(instance *inst, solution *sol);


#endif //TABU_SEARCH_H