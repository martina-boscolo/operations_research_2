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

int is_tabu(tabu_params *params, int node) {
    return params->tabu_list[node] != -1 && 
           params->current_iter - params->tabu_list[node] <= get_random_tenure(params->min_tenure, params->max_tenure);
}

void update_tabu_status(tabu_params *params, int node) {
    params->tabu_list[node] = params->current_iter;
}

int is_tabu_list_full(tabu_params *params, int nnodes) {
    int count = 0;
    for (int i = 0; i < nnodes; i++) {
        if (params->tabu_list[i] != -1) {
            count++;
        }
    }
    // Consider the list "full" if more than 75% of nodes are tabu
    return ( count > (nnodes * 0.75));
}

void reset_tabu_list_if_full(tabu_params *params, instance *inst) {
    if (is_tabu_list_full(params, inst->nnodes)) {
        // Reset the entire tabu list
        for (int i = 0; i < inst->nnodes; i++) {
            params->tabu_list[i] = -1;
        }
        if (inst->verbose >= DEBUG)
            printf("Tabu list was reset at iteration %d\n", params->current_iter);
    }
}

void find_best_neighbor(instance *inst, solution *current, tabu_params *params) {
    int nnodes = inst->nnodes;
    double best_cost = DBL_MAX;
    int best_i = -1, best_j = -1;

    // Create a temporary tour for evaluating moves
    solution temp_sol;
    temp_sol.visited_nodes = (int *)malloc((nnodes + 1) * sizeof(int));

    // Try all possible 2-opt swaps
    for (int i = 1; i < nnodes - 1; i++) {
        for (int j = i + 1; j < nnodes; j++) {
            // Skip if any of the nodes are tabu, unless aspiration criterion applies
            if (is_tabu(params, current->visited_nodes[i]) ||
                is_tabu(params, current->visited_nodes[j])) {
                continue;
            }

            // Copy the current tour to evaluate this move
            memcpy(temp_sol.visited_nodes, current->visited_nodes, (nnodes + 1) * sizeof(int));

            // Perform 2-opt swap
            reverse_segment(&temp_sol, i, j);

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
        reverse_segment(current, best_i, best_j);
        current->cost = best_cost;

        // Randomly select one of the four nodes involved in the swap to mark as tabu
        int nodes_involved[4] = {best_i, best_i + 1, best_j, (best_j + 1) % nnodes};
        int random_node = nodes_involved[rand() % 4];
        update_tabu_status(params, current->visited_nodes[random_node]);
    }

    free_solution(&temp_sol);
}


void tabu_search(instance *inst, solution *sol) {
    
    int nnodes = inst->nnodes;
    
    // Initialize tabu parameters
    tabu_params params;
    int min_tenure = (int)(0.1 * nnodes);            // Min number of iterations node remains tabu
    int max_tenure = (int)(0.2 * nnodes);            // Max number of iterations node remains tabu
    init_tabu_params(&params, nnodes, min_tenure, max_tenure);
    FILE* f = fopen("results/TS.csv", "w+");
 
    // Main loop

    while (get_elapsed_time(inst->t_start) < inst->timelimit) {
        reset_tabu_list_if_full(&params, inst);

        // Find best neighbor
        find_best_neighbor(inst, sol, &params);
        
        // Update best solution if needed
        update_best_sol(inst, sol);
        fprintf(f, "%d,%f,%f\n", params.current_iter, sol->cost, inst->best_solution->cost);

        check_sol(inst, sol);
        params.current_iter++;
        
    }
    
    strcpy(inst->best_solution->method, "TS");
    plot_stats_in_file(inst->best_solution->method);   

    // Free memory
    free_tabu_params(&params);
}