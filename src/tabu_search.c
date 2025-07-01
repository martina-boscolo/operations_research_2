#include "tabu_search.h"

// Tabu search algorithm
void tabu_search(const instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, TABU_SEARCH) == 0);

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    // Go to local minima, for first iterations the tabu list is useless
    two_opt(inst, &temp_sol, timelimit, false);
    updated = updated || update_sol(inst, sol, &temp_sol, is_asked_method);
        
    // Initialize tabu parameters
    tabu_params params;
    int min_tenure = (int)(1 + 0.2 * inst->nnodes); // Min number of iterations node remains tabu
    int max_tenure = (int)(1 + 0.6 * inst->nnodes); // Max number of iterations node remains tabu
    init_tabu_params(&params, inst->nnodes, min_tenure, max_tenure, inst->param1);

    char filename[FILE_NAME_LEN];
    sprintf_s(filename, FILE_NAME_LEN, "TS_p%d", inst->param1);

    FILE *f;
    if (inst->verbose >= ONLY_INCUMBMENT && is_asked_method) {

        char filename_results[FILE_NAME_LEN];
        sprintf_s(filename_results, FILE_NAME_LEN, "results/%s.csv", filename);

        if (fopen_s(&f, filename_results, "w+")) print_error("tabu_search(): Cannot open file");

    }

    // Main loop
    while (get_elapsed_time(t_start) < timelimit) {

        reset_tabu_list_if_full(&params, inst);

        // Find best neighbor
        move_to_best_neighbor(inst, &temp_sol, &params);

        if (inst->verbose >= GOOD) {
            check_sol(inst, &temp_sol);
        }
        
        // Update best solution if needed
        updated = updated || update_sol(inst, sol, &temp_sol, true);

        if (inst->verbose >= ONLY_INCUMBMENT && is_asked_method) {
            fprintf(f, "%d,%f,%f\n", params.current_iter, temp_sol.cost, sol->cost);
        }
        
        params.current_iter++;
    }

    if (updated) {

        sprintf_s(sol->method, METH_NAME_LEN, filename);

    }

    if (inst->verbose >= ONLY_INCUMBMENT && is_asked_method) {

        plot_stats_in_file(filename);

    }

    // Close the file if it was opened
    if (f != NULL) {

        fclose(f);

    }

    // Free memory
    free_tabu_params(&params);

}

// Compute tenure based on the chosen type
int compute_tenure(const tabu_params *params) {

    switch (params->tenure_type) {

        case FIXED_MIN:

            return params->min_tenure; // Fixed tenure value to min
            
        case FIXED_MAX:

            return params->max_tenure; // Fixed tenure value to max
            
        case RANDOM:

            // More diverse random - occasional spikes
            if (rand() % 10 == 0) { // 10% chance of extreme value

                return params->max_tenure + (int)(params->max_tenure * 0.5);

            } else {

                return params->min_tenure + rand() % (params->max_tenure - params->min_tenure + 1);

            }
           
        default: //best performer 
        case LINEAR:

            // Sawtooth pattern - linear increase followed by reset
            int cycle_length = 2 * (params->max_tenure - params->min_tenure);
            int position = params->current_iter % cycle_length;

            if (position < cycle_length / 2) {

                return params->min_tenure + position; // Increasing

            } else{

                return params->max_tenure - (position - cycle_length / 2); // Decreasing

            }

        case SINUSOIDAL:

            // More pronounced sinusoidal with wider amplitude
            double frequency = 0.05; // Slower oscillation
            double amplitude = params->max_tenure - params->min_tenure;
            return params->min_tenure + (int)(amplitude * (0.5 * (1 + sin(frequency * params->current_iter))));

    }

}

// Initialize tabu search parameters
void init_tabu_params(tabu_params *params, const int nnodes, const int min_tenure, const int max_tenure, const TenureType tenure_type) {
    
    params->tabu_list = (int *)malloc(nnodes * sizeof(int));

    if (params->tabu_list == NULL) print_error("init_tabu_params(): Cannot allocate memory");

    for (int i = 0; i < nnodes; i++) {

        params->tabu_list[i] = -1;  // Initialize all nodes as non-tabu

    }

    params->min_tenure = min_tenure;
    params->max_tenure = max_tenure;
    params->current_iter = 0;
    params->tenure_type = tenure_type;

}

// Free tabu search parameters
void free_tabu_params(tabu_params *params) {

    free(params->tabu_list);

}

// Check if a given node is tabu
int is_tabu(const tabu_params *params, const int node) {

    return params->tabu_list[node] != -1 && 
           params->current_iter - params->tabu_list[node] <= compute_tenure(params);

}

// Update tabu status for a given node
void update_tabu_status(tabu_params *params, const int node) {

    params->tabu_list[node] = params->current_iter;

}

// Check if the tabu list is almost full
int is_tabu_list_full(const tabu_params *params, const int nnodes) {

    int count = 0;

    for (int i = 0; i < nnodes; i++) {

        if (is_tabu(params, i)) {

            count++;

        }

    }

    // Consider the list "full" if more than 95% of nodes are tabu
    return ( count > (nnodes * 0.95));

}

// Reset the tabu list if full
void reset_tabu_list_if_full(tabu_params *params, const instance *inst) {

    if (is_tabu_list_full(params, inst->nnodes)) {

        // Reset the entire tabu list
        for (int i = 0; i < inst->nnodes; i++) {

            params->tabu_list[i] = -1;

        }

        if (inst->verbose >= DEBUG_V) {

            printf("Tabu list was reset at iteration %5d\n", params->current_iter);
        
        }

    }

}

// Find best neighbor using considering tabu status and move to it
void move_to_best_neighbor(const instance *inst, solution *current, tabu_params *params) {
    
    int nnodes = inst->nnodes;
    double best_delta = DBL_MAX;
    int best_i = -1, best_j = -1;

    // Try all possible 2-opt swaps
    for (int i = 1; i < nnodes - 1; i++) {

        for (int j = i + 1; j < nnodes; j++) {

            // Skip if the edge is tabu
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

    // Apply the best move found
    if (best_i != -1 && best_j != -1) {

        current->cost += best_delta;
        reverse_segment(current, best_i, best_j);

        // Mark as tabu the edges
        update_tabu_status(params, current->visited_nodes[best_i-1]);
        update_tabu_status(params, current->visited_nodes[best_i]);
        update_tabu_status(params, current->visited_nodes[best_j]);
        update_tabu_status(params, current->visited_nodes[best_j+1]);

    }

}
