#ifndef TABU_SEARCH_H
#define TABU_SEARCH_H

#include "tsp.h"
#include "heuristics.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>

#define TABU_SEARCH "TS"

// Enum for tenure types
typedef enum {

    FIXED_MIN,              // Fixed tenure value to min
    FIXED_MAX,              // Fixed tenure value to max
    RANDOM,                 // Random tenure value with occasional spikes
    LINEAR,                 // Sawtooth pattern: linear increase followed by reset
    SINUSOIDAL              // Sinusoidal tenure value

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
 * Tabu search algorithm.
 *  
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution that will contain the best solution between the input one and the best found one (input/output)
 * @param timelimit The time limit for the algorithm (input)
 */
void tabu_search(const instance *inst, solution *sol, const double timelimit);

/**
 * Compute tenure based on the chosen type.
 *  
 * @param params The tabu search parameters (input)
 * 
 * @return The computed tenure
 */
int compute_tenure(const tabu_params *params);

/**
 * Initialize tabu search parameters.
 *  
 * @param params The tabu search parameters (output)
 * @param nnodes The number of nodes in the instance (input)
 * @param min_tenure The minimum tabu tenure (input)
 * @param max_tenure The maximum tabu tenure (input)
 * @param tenure_type The type of tenure (input)
 */
void init_tabu_params(tabu_params *params, const int nnodes, const int min_tenure, const int max_tenure, const TenureType tenure_type);

/**
 * Free tabu search parameters.
 *  
 * @param params The tabu search parameters (input/output)
 */
void free_tabu_params(tabu_params *params);


/**
 * Check if a given node is tabu.
 *  
 * @param params The tabu search parameters (input)
 * @param node The node to check (input)
 * 
 * @return true if the node is tabu, false otherwise
 */
bool is_tabu(const tabu_params *params, const int node);

/**
 * Update tabu status for a given node.
 *  
 * @param params The tabu search parameters (input/output)
 * @param node The node to check (input)
 */
void update_tabu_status(tabu_params *params, const int node);

/**
 * Check if the tabu list is almost full.
 *  
 * @param params The tabu search parameters (input)
 * @param nnodes The number of nodes (input)
 * 
 * @return true if the tabu list is full, false otherwise
 */
bool is_tabu_list_full(const tabu_params *params, const int nnodes);

/**
 * Reset the tabu list if full.
 *  
 * @param params The tabu search parameters (input/output)
 * @param inst The instance that contains the problem data (input)
 */
void reset_tabu_list_if_full(tabu_params *params, const instance *inst);

/**
 * Find best neighbor using considering tabu status and move to it
 *  
 * @param inst The instance that contains the problem data (input)
 * @param sol TThe solution to modify (input/output)
 * @param params The tabu search parameters (input/output)
 */
void move_to_best_neighbor(const instance *inst, solution *current, tabu_params *params);

#endif //TABU_SEARCH_H