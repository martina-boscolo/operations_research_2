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

// Enum for tenure types
typedef enum {
    FIXED_MIN,
    FIXED_MAX,
    RANDOM,
    LINEAR,
    SINUSOIDAL
} TenureType;

// Structure to hold tabu search parameters
typedef struct {
    int *tabu_list;         // Tabu list: -1 if not tabu, otherwise iteration when it became tabu
    int min_tenure;         // Minimum tabu tenure
    int max_tenure;         // Maximum tabu tenure
    int current_iter;       // Current iteration
    TenureType tenure_type; // Type of tenure 
} tabu_params;

/**
 * Calculate tenure based on the chosen type
 *  
 * @param params The tabu search parameters
 * @return The calculated tenure
 */
int calculate_tenure(tabu_params *params);

/**
 * Initialize tabu search parameters
 *  
 * @param params The tabu search parameters
 * @param nnodes The number of nodes
 * @param min_tenure Minimum tabu tenure
 * @param max_tenure Maximum tabu tenure
 * @param tenure_type Type of tenure 
 */
void init_tabu_params(tabu_params *params, int nnodes, int min_tenure, int max_tenure, TenureType tenure_type);

/**
 * Free tabu search parameters
 *  
 * @param params The tabu search parameters
 */
void free_tabu_params(tabu_params *params);


/**
 * Check if a node is tabu
 *  
 * @param params The tabu search parameters
 * @param node The node to check
 */
int is_tabu(tabu_params *params, int node);

/**
 * Update tabu status for a node
 *  
 * @param params 
 * @param node 
 */
void update_tabu_status(tabu_params *params, int node);

/**
 * Check if tabu list is almost full
 *  
 * @param params The tabu search parameters
 * @param nnodes Number of nodes
 */
int is_tabu_list_full(tabu_params *params, int nnodes);


/**
 * Reset tabu list if full
 *  
 * @param params The tabu search parameters
 * @param inst The instance pointer of the problem
 */
void reset_tabu_list_if_full(tabu_params *params, instance *inst);

/**
 * Find best neighbor using swap operator and considering tabu status
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param params The tabu search parameters
 */
void find_best_neighbor(instance *inst, solution *current, tabu_params *params);


/**
 * Main tabu search procedure
 *  
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void tabu_search(instance *inst, solution *sol, time_t t_start);


#endif //TABU_SEARCH_H