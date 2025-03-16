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

// Initialize tabu search parameters
void init_tabu_params(tabu_params *params, int nnodes, int min_tenure, int max_tenure);
// Free tabu search parameters
void free_tabu_params(tabu_params *params);

// Get a random number between min and max
int get_random_tenure(int min, int max);
// Check if a node is tabu
int is_tabu(tabu_params *params, int node, int current_iter);
// Update tabu status for a node
void update_tabu_status(tabu_params *params, int node);
// Copy solution
void copy_solution(solution *dest, solution *src, int nnodes);

// Calculate solution cost
double calculate_solution_cost(instance *inst, int *tour);
// Generate an initial solution using nearest neighbor heuristic
void generate_initial_solution(instance *inst, solution *sol);

// Perform a swap move between positions i and j in the tour
void swap_nodes(int *tour, int i, int j);
void find_best_neighbor(instance *inst, solution *current, tabu_params *params);

// Main tabu search procedure
void tabu_search(instance *inst, solution *sol);


#endif //TABU_SEARCH_H