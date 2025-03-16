#include "tabu_search.h"

void init_tabu_params(tabu_params *params, int nnodes, int min_tenure, int max_tenure) {
    params->tabu_list = (int *)malloc(nnodes * sizeof(int));
    for (int i = 0; i < nnodes; i++) {
        params->tabu_list[i] = -1;  // Initialize all nodes as non-tabu
    }
    params->min_tenure = min_tenure;
    params->max_tenure = max_tenure;
    params->current_iter = 0;
}

void free_tabu_params(tabu_params *params) {
    free(params->tabu_list);
}

int get_random_tenure(int min, int max) {
    return min + rand() % (max - min + 1);
}

int is_tabu(tabu_params *params, int node, int current_iter) {
    return params->tabu_list[node] != -1 && 
           current_iter - params->tabu_list[node] <= get_random_tenure(params->min_tenure, params->max_tenure);
}

void update_tabu_status(tabu_params *params, int node) {
    params->tabu_list[node] = params->current_iter;
}

void find_best_neighbor(instance *inst, solution *current, tabu_params *params) {
    int nnodes = inst->nnodes;
    double best_cost = DBL_MAX;
    int best_i = -1, best_j = -1;
    
    // Create a temporary tour for evaluating moves
    solution temp_sol;
    temp_sol.visited_nodes = (int *)malloc((nnodes + 1) * sizeof(int));
    
    // Try all possible swaps (excluding first and last nodes since they must be the same)
    for (int i = 1; i < nnodes - 1; i++) {
        for (int j = i + 1; j < nnodes; j++) {
            // Skip if either node is tabu, unless it would result in an aspiration criterion
            if (is_tabu(params, current->visited_nodes[i], params->current_iter) || 
                is_tabu(params, current->visited_nodes[j], params->current_iter)) {
                continue;
            }
            
            // Copy the current tour to evaluate this move
            memcpy(temp_sol.visited_nodes, current->visited_nodes, (nnodes + 1) * sizeof(int));
            
            // Perform swap
            swap_nodes(temp_sol.visited_nodes, i, j);
            
            // Calculate new cost
            double new_cost = compute_solution_cost(inst, &temp_sol);
            
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
    
    free_solution(&temp_sol);
}


void tabu_search(instance *inst, solution *sol) {

    // obtain first solution
    nearest_neighbor(inst, sol, rand() % inst->nnodes);

    int nnodes = inst->nnodes;
    
    // Initialize tabu parameters
    tabu_params params;
    int min_tenure = 10;                              // Min number of iterations node remains tabu
    int max_tenure = (int)(0.3 * nnodes);            // Max number of iterations node remains tabu
    init_tabu_params(&params, nnodes, min_tenure, max_tenure);
    
 
    // Main loop

    while (get_elapsed_time(inst->t_start) < inst->timelimit) {

        // Find best neighbor
        find_best_neighbor(inst, sol, &params);
        
        
        // Update best solution if needed
        update_best_sol(inst, sol);
        check_sol(inst, sol);
        params.current_iter++;
        

    }
    printf("Time out!");
    
    strcpy(inst->best_solution->method, "Tabu_Search");
       
    // Free memory
    free_tabu_params(&params);
}