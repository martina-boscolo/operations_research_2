#include "tabu_search.h"


// Function to calculate tenure based on chosen type
int calculate_tenure(tabu_params *params) {
    switch (params->tenure_type) {
        case FIXED_MIN:
            return params->min_tenure; // Fixed tenure value to min
            
        case FIXED_MAX:
            return params->max_tenure; // Fixed tenure value to max
            
        case RANDOM:
            // More diverse random - occasional spikes
            if (rand() % 10 == 0) // 10% chance of extreme value
                return params->max_tenure + (int)(params->max_tenure * 0.5);
            else
                return params->min_tenure + rand() % (params->max_tenure - params->min_tenure + 1);
            
        case LINEAR:
            // Sawtooth pattern - linear increase followed by reset
            {
                int cycle_length = 2 * (params->max_tenure - params->min_tenure);
                int position = params->current_iter % cycle_length;
                if (position < cycle_length / 2)
                    return params->min_tenure + position; // Increasing
                else
                    return params->max_tenure - (position - cycle_length / 2); // Decreasing
            }
        default: //best performer   
        case SINUSOIDAL:
            // More pronounced sinusoidal with wider amplitude
            {
                double frequency = 0.05; // Slower oscillation
                double amplitude = params->max_tenure - params->min_tenure;
                return params->min_tenure + (int)(amplitude * (0.5 * (1 + sin(frequency * params->current_iter))));
            }
    }
}

void init_tabu_params(tabu_params *params, int nnodes, int min_tenure, int max_tenure, TenureType tenure_type) {
    params->tabu_list = (int *)malloc(nnodes * sizeof(int));
    for (int i = 0; i < nnodes; i++) {
        params->tabu_list[i] = -1;  // Initialize all nodes as non-tabu
    }
    params->min_tenure = min_tenure;
    params->max_tenure = max_tenure;
    params->current_iter = 0;
    params->tenure_type = tenure_type;
}

void free_tabu_params(tabu_params *params) {
    free(params->tabu_list);
}

int is_tabu(tabu_params *params, int node) {
    return params->tabu_list[node] != -1 && 
           params->current_iter - params->tabu_list[node] <= calculate_tenure(params);
}

void update_tabu_status(tabu_params *params, int node) {
    params->tabu_list[node] = params->current_iter;
}

int is_tabu_list_full(tabu_params *params, int nnodes) {
    int count = 0;
    for (int i = 0; i < nnodes; i++) {
        if (is_tabu(params, i)) {
            count++;
        }
    }
    // Consider the list "full" if more than 95% of nodes are tabu
    return ( count > (nnodes * 0.95));
}

void reset_tabu_list_if_full(tabu_params *params, const instance *inst) {
    if (is_tabu_list_full(params, inst->nnodes)) {
        // Reset the entire tabu list
        for (int i = 0; i < inst->nnodes; i++) {
            params->tabu_list[i] = -1;
        }
        if (inst->verbose >= DEBUG)
            printf("Tabu list was reset at iteration %d\n", params->current_iter);
    }
}

void find_best_neighbor(const instance *inst, solution *current, tabu_params *params) {
    
    int nnodes = inst->nnodes;
    double best_delta = DBL_MAX;
    int best_i = -1, best_j = -1;

    // Try all possible 2-opt swaps
    for (int i = 1; i < nnodes - 1; i++) {
        for (int j = i + 1; j < nnodes; j++) {
            // Skip if any of the nodes are tabu, unless aspiration criterion applies
            if (is_tabu(params, current->visited_nodes[i]) &&
                is_tabu(params, current->visited_nodes[j])) {
                continue;
            }

            // Compute the delta cost
            double delta = delta2(inst, current, i, j);

            // If current delta is better update it
            if (delta < best_delta - EPSILON) {
                best_delta = delta;
                best_i = i;
                best_j = j;
            }
            
        }
    }

    // If we found a valid move, apply it directly to the current solution
    if (best_i != -1 && best_j != -1) {

        current->cost += delta2(inst, current, best_i, best_j);
        reverse_segment(current, best_i, best_j);
        
        // Randomly select one of the four nodes involved in the swap to mark as tabu
        //int nodes_involved[4] = {best_i-1, best_i, best_j, best_j+1};
        //int random_node = nodes_involved[rand() % 4];
        //update_tabu_status(params, current->visited_nodes[random_node]);

        // Mark as tabu the edges
        update_tabu_status(params, current->visited_nodes[best_i-1]);
        update_tabu_status(params, current->visited_nodes[best_i]);
        update_tabu_status(params, current->visited_nodes[best_j]);
        update_tabu_status(params, current->visited_nodes[best_j+1]);

    }

}


void tabu_search(const instance *inst, solution *sol, const int timelimit) {

    time_t t_start = seconds();

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    // Go to local minima, for first iteration the tabu list is useless
    two_opt(inst, &temp_sol, timelimit, false);
        
    // Initialize tabu parameters
    tabu_params params;
    int min_tenure = (int)(1 + 0.2 * inst->nnodes); // Min number of iterations node remains tabu
    int max_tenure = (int)(1 + 0.6 * inst->nnodes); // Max number of iterations node remains tabu
    init_tabu_params(&params, inst->nnodes, min_tenure, max_tenure, inst->param1);

    char filename[50];
    sprintf(filename, "TS_p%d", inst->param1);
    FILE *f = NULL;
    if (inst->verbose >= ONLY_INCUMBMENT)
    {
        char filename_results[50];
        sprintf(filename_results, "results/%s.csv", filename);
        f = fopen(filename_results, "w+");
    }

    // Main loop
    while (get_elapsed_time(t_start) < timelimit)
    {
        reset_tabu_list_if_full(&params, inst);

        // Find best neighbor
        find_best_neighbor(inst, &temp_sol, &params);

        if (inst->verbose >= GOOD) {
            check_sol(inst, &temp_sol);
        }
        
        // Update best solution if needed
        update_sol(inst, sol, &temp_sol, true);

        if (inst->verbose >= ONLY_INCUMBMENT){
            fprintf(f, "%d,%f,%f\n", params.current_iter, temp_sol.cost, sol->cost);
        }
        check_sol(inst, sol);
        params.current_iter++;
    }

    sprintf(sol->method, filename);

    if (inst->verbose >= ONLY_INCUMBMENT){
        plot_stats_in_file(filename);
    }
    // Close the file if it was opened
    if (f != NULL){
        fclose(f);
    }

    // Free memory
    free_tabu_params(&params);
}