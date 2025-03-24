#include "heuristics.h"

//---------------utilities------------------

int find_nearest_node(instance *inst, int len, int *visited_nodes) {
    int nearest = -1;
    double min_cost = INFINITY;

    for (int i = len; i < inst->nnodes; i++) {
        int node = visited_nodes[i];
        double current_cost = cost(visited_nodes[len-1] , node, inst);

        if (current_cost < min_cost) {
            min_cost = current_cost;
            nearest = i;
        }
    }
    return nearest;
}

void swap_nodes(int *nodes, int i, int j) {
    int temp = nodes[i];
    nodes[i] = nodes[j];
    nodes[j] = temp;
}

void reverse_segment(solution *sol, int i, int j) {
    for (int k = 0; k < (j - i + 1) / 2; k++) {
        int temp = sol->visited_nodes[i + k];
        sol->visited_nodes[i + k] = sol->visited_nodes[j - k];
        sol->visited_nodes[j - k] = temp;
    }
}

void shift_segment(instance *inst, solution *sol, int idx1, int idx2, int idx3) {
    int n = inst->nnodes;
    
    // Calculate segment sizes
    int segment1_size = (idx2+1) - (idx1+1);
    int segment2_size = (idx3+1) - (idx2+1);
    int segment3_size = n - (idx3+1) + (idx1+1);
    
    // Allocate memory for segments
    int *segment1 = (int *)malloc(segment1_size * sizeof(int));
    int *segment2 = (int *)malloc(segment2_size * sizeof(int));
    int *segment3 = (int *)malloc(segment3_size * sizeof(int));
    
    // Copy segments
    for (int j = 0; j < segment1_size; j++) {
        segment1[j] = sol->visited_nodes[(idx1+1+j) % n];
    }
    
    for (int j = 0; j < segment2_size; j++) {
        segment2[j] = sol->visited_nodes[(idx2+1+j) % n];
    }
    
    for (int j = 0; j < segment3_size; j++) {
        segment3[j] = sol->visited_nodes[(idx3+1+j) % n];
    }
    
    // Rearrange segments
    int pos = (idx1+1) % n;
    
    // Place segment2
    for (int j = 0; j < segment2_size; j++) {
        sol->visited_nodes[pos] = segment2[j];
        pos = (pos + 1) % n;
    }
    
    // Place segment1
    for (int j = 0; j < segment1_size; j++) {
        sol->visited_nodes[pos] = segment1[j];
        pos = (pos + 1) % n;
    }
    
    // Place segment3
    for (int j = 0; j < segment3_size; j++) {
        sol->visited_nodes[pos] = segment3[j];
        pos = (pos + 1) % n;
    }
    
    // Free allocated memory
    free(segment1);
    free(segment2);
    free(segment3);    
}

//---------------heuristics------------------

void nearest_neighbor(instance *inst, solution *sol, int start)
{
    initialize_tour(sol->visited_nodes, inst->nnodes);
    swap_nodes(sol->visited_nodes, start, 0);

    int len = 1;
    double total_cost = 0;
    for (int i = 1; i < inst->nnodes; i++)
    {
        int nearest_index = find_nearest_node(inst, len, sol->visited_nodes);
        if (nearest_index == -1)
        {
            print_error("No valid nearest neighbor found\n");
        } // almost useless

        int nearest_node = sol->visited_nodes[nearest_index];
        swap_nodes(sol->visited_nodes, nearest_index, len);
        total_cost += cost(sol->visited_nodes[len - 1], nearest_node, inst);
        len++;
    }
    // Complete the cycle
    sol->visited_nodes[inst->nnodes] = start;
    total_cost += cost(sol->visited_nodes[len - 1], start, inst);

    sol->cost = total_cost;

    check_sol(inst, sol);
}

void multi_start_nn(instance *inst, solution *sol) {

    for (int start = 0; start < inst->nnodes; start++) {

        double elapsed_time = get_elapsed_time(inst->t_start);
        printf("Elapsed time: %.6f seconds\n", elapsed_time); //to debug

        if (inst->timelimit > 0 && elapsed_time >= inst->timelimit) {
            printf("Time limit reached! Stopping early.\n");
            break; // Stop if time limit is reached, otherwise finish the complete computation
        }

        //no check of time limit from now on
        nearest_neighbor(inst, sol, start);
        two_opt(inst, sol);

        // Debug: Print intermediate results
        printf("Start Node: %d, Cost: %.2lf\n", start, sol->cost);

        update_best_sol(inst, sol);
        check_sol(inst, sol);
    }
}

void two_opt(instance *inst, solution *sol) {
    int improved = 1; // Flag to track improvements
    while (improved && (get_elapsed_time(inst->t_start) < inst->timelimit)) {
        improved = 0;
        for (int i = 1; i < inst->nnodes; i++) {
            if (get_elapsed_time(inst->t_start) >= inst->timelimit) {
                break; 
            }
            for (int j = i + 1; j < inst->nnodes; j++) {
                if (get_elapsed_time(inst->t_start) >= inst->timelimit) {
                    break; 
                }
                // Calculate the cost of the two edges that would be removed
                double old_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[i], inst) +
                                  cost(sol->visited_nodes[j], sol->visited_nodes[j+1], inst);
               
                // Calculate the cost of the two new edges that would be added
                double new_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[j], inst) +
                                  cost(sol->visited_nodes[i], sol->visited_nodes[j+1], inst);
                
                if (new_cost < old_cost - EPSILON){
                    // Reverse the segment between i and j
                    reverse_segment(sol, i, j);
                    
                    // Update the solution cost correctly
                    sol->cost -= (old_cost - new_cost);
                    improved = 1; // Indicate improvement found
                }
            }
        }
    }
    strcpy(sol->method, "TWO_OPT");
    check_sol(inst, sol);
}

int ms_2opt_nn_main(instance *inst, solution *sol) {

    strcpy(sol->method, "NEAREST_NEIGHBOR");
    
    multi_start_nn(inst, sol);

    printf("Best Solution Cost: %lf\n", inst->best_solution->cost);
    printf("Best Path: ");
    for (int i = 0; i <= inst->nnodes; i++) {
        printf("%d ", inst->best_solution->visited_nodes[i]);
    }
    printf("\n");
    check_sol(inst, sol);

    return 0;
}
