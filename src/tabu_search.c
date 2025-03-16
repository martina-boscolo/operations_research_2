#include "tabu_search.h"
#include <float.h>
#include <math.h>


// Initialize tabu search parameters
void init_tabu_params(tabu_params *params, int nnodes, int min_tenure, int max_tenure) {
    params->tabu_list = (int *)malloc(nnodes * sizeof(int));
    for (int i = 0; i < nnodes; i++) {
        params->tabu_list[i] = -1;  // Initialize all nodes as non-tabu
    }
    params->min_tenure = min_tenure;
    params->max_tenure = max_tenure;
    params->current_iter = 0;
}

// Free tabu search parameters
void free_tabu_params(tabu_params *params) {
    free(params->tabu_list);
}

// Get a random number between min and max
int get_random_tenure(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Check if a node is tabu
int is_tabu(tabu_params *params, int node, int current_iter) {
    return params->tabu_list[node] != -1 && 
           current_iter - params->tabu_list[node] <= get_random_tenure(params->min_tenure, params->max_tenure);
}

// Update tabu status for a node
void update_tabu_status(tabu_params *params, int node) {
    params->tabu_list[node] = params->current_iter;
}

// Copy solution
void copy_solution(solution *dest, solution *src, int nnodes) {
    dest->cost = src->cost;
    strcpy(dest->method, src->method);
    for (int i = 0; i <= nnodes; i++) {
        dest->visited_nodes[i] = src->visited_nodes[i];
    }
}

// Calculate solution cost
double calculate_solution_cost(instance *inst, int *tour) {
    double cost = 0.0;
    for (int i = 0; i < inst->nnodes; i++) {
        int from = tour[i];
        int to = tour[i + 1];
        cost += inst->costs[from * inst->nnodes + to];
    }
    return cost;
}

// Generate an initial solution using nearest neighbor heuristic
void generate_initial_solution(instance *inst, solution *sol) {
    int nnodes = inst->nnodes;
    int *visited = (int *)calloc(nnodes, sizeof(int));
    
    // Start from node 0
    sol->visited_nodes[0] = 0;
    visited[0] = 1;
    
    // Build tour
    for (int i = 1; i < nnodes; i++) {
        int prev = sol->visited_nodes[i - 1];
        int best_next = -1;
        double min_dist = DBL_MAX;
        
        for (int j = 0; j < nnodes; j++) {
            if (!visited[j]) {
                double dist = inst->costs[prev * nnodes + j];
                if (dist < min_dist) {
                    min_dist = dist;
                    best_next = j;
                }
            }
        }
        
        sol->visited_nodes[i] = best_next;
        visited[best_next] = 1;
    }
    
    // Complete the cycle by returning to starting node
    sol->visited_nodes[nnodes] = sol->visited_nodes[0];
    
    // Calculate cost
    sol->cost = calculate_solution_cost(inst, sol->visited_nodes);
    strcpy(sol->method, "Nearest Neighbor");
    
    free(visited);
}


// // Find best neighbor using swap operator and considering tabu status
// int find_best_neighbor(instance *inst, solution *current, solution *best_neighbor, tabu_params *params) {
//     int nnodes = inst->nnodes;
//     double best_cost = DBL_MAX;
//     int best_i = -1, best_j = -1;
//     int improvement = 0;
    
//     // Create a temporary tour for evaluating moves
//     int *temp_tour = (int *)malloc((nnodes + 1) * sizeof(int));
//     memcpy(temp_tour, current->visited_nodes, (nnodes + 1) * sizeof(int));
    
//     // Try all possible swaps (excluding first and last nodes since they must be the same)
//     for (int i = 1; i < nnodes - 1; i++) {
//         for (int j = i + 1; j < nnodes; j++) {
//             // Skip if either node is tabu, unless it would result in an aspiration criterion
//             if (is_tabu(params, temp_tour[i], params->current_iter) || 
//                 is_tabu(params, temp_tour[j], params->current_iter)) {
//                 continue;
//             }
            
//             // Perform swap
//             swap_nodes(temp_tour, i, j);
            
//             // Calculate new cost
//             double new_cost = calculate_solution_cost(inst, temp_tour);
            
//             // Check if this is the best move so far
//             if (new_cost < best_cost) {
//                 best_cost = new_cost;
//                 best_i = i;
//                 best_j = j;
                
//                 // Check if this is an improvement over the current solution
//                 if (new_cost < current->cost) {
//                     improvement = 1;
//                 }
//             }
            
//             // Undo swap for next iteration
//             swap_nodes(temp_tour, i, j);
//         }
//     }
    
//     // If we found a valid move
//     if (best_i != -1 && best_j != -1) {
//         // Apply the best move to the neighbor solution
//         memcpy(best_neighbor->visited_nodes, current->visited_nodes, (nnodes + 1) * sizeof(int));
//         swap_nodes(best_neighbor->visited_nodes, best_i, best_j);
//         best_neighbor->cost = best_cost;
//         strcpy(best_neighbor->method, "Tabu Search");
        
//         // Update tabu status for the moved nodes
//         update_tabu_status(params, current->visited_nodes[best_i]);
//         update_tabu_status(params, current->visited_nodes[best_j]);
//     }
    
//     free(temp_tour);
//     return improvement;
// }

void find_best_neighbor(instance *inst, solution *current, tabu_params *params) {
    int nnodes = inst->nnodes;
    double best_cost = DBL_MAX;
    int best_i = -1, best_j = -1;
    
    // Create a temporary tour for evaluating moves
    int *temp_tour = (int *)malloc((nnodes + 1) * sizeof(int));
    
    // Try all possible swaps (excluding first and last nodes since they must be the same)
    for (int i = 1; i < nnodes - 1; i++) {
        for (int j = i + 1; j < nnodes; j++) {
            // Skip if either node is tabu, unless it would result in an aspiration criterion
            if (is_tabu(params, current->visited_nodes[i], params->current_iter) || 
                is_tabu(params, current->visited_nodes[j], params->current_iter)) {
                continue;
            }
            
            // Copy the current tour to evaluate this move
            memcpy(temp_tour, current->visited_nodes, (nnodes + 1) * sizeof(int));
            
            // Perform swap
            swap_nodes(temp_tour, i, j);
            
            // Calculate new cost
            double new_cost = calculate_solution_cost(inst, temp_tour);
            
            // Check if this is the best move so far
            if (new_cost < best_cost) {
                best_cost = new_cost;
                best_i = i;
                best_j = j;
                
            }
        }
    }
    
    // If we found a valid move, apply it directly to the current solution
    if (best_i != -1 && best_j != -1) {
        swap_nodes(current->visited_nodes, best_i, best_j);
        current->cost = best_cost;
        
        // Update tabu status for the moved nodes
        update_tabu_status(params, current->visited_nodes[best_i]);
        update_tabu_status(params, current->visited_nodes[best_j]);
    }
    
    free(temp_tour);
}

// Main tabu search procedure
void tabu_search(instance *inst, solution *sol) {

    // obtain first solution
    nearest_neighbor(inst, sol, rand() % inst->nnodes);

    int nnodes = inst->nnodes;
    
      
    // Initialize tabu parameters
    tabu_params params;
    int min_tenure = 5;                              // Min number of iterations node remains tabu
    int max_tenure = (int)(0.3 * nnodes);            // Max number of iterations node remains tabu
    init_tabu_params(&params, nnodes, min_tenure, max_tenure);
    
 
    // Main loop

    while (get_elapsed_time(inst->t_start) < inst->timelimit) {

        // Find best neighbor
        find_best_neighbor(inst, sol, &params);
        
        
        // Update best solution if needed
        update_best_sol(inst, sol);
        
        params.current_iter++;
        

    }
    
    strcpy(inst->best_solution->method, "Tabu Search");
    
    if (inst->verbose >= 50) {
        printf("Tabu search completed after %d iterations\n", params.current_iter);
        printf("Best solution cost: %.2f\n", inst->best_solution->cost);
    }
    
    // Free memory
    free_tabu_params(&params);
}