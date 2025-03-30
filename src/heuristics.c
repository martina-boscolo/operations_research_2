#include "heuristics.h"

//---------------utilities------------------

int find_nearest_node(const instance *inst, const int len, const int *visited_nodes) {
    int nearest = -1;
    double min_cost = INFINITY;

    for (int i = len; i < inst->nnodes; i++) {
        int node = visited_nodes[i];
        double current_cost = cost(visited_nodes[len-1], node, inst);

        if (current_cost < min_cost) {
            min_cost = current_cost;
            nearest = i;
        }
    }
    return nearest;
}

void swap_nodes(int *nodes, const int i, const int j) {
    int temp = nodes[i];
    nodes[i] = nodes[j];
    nodes[j] = temp;
}

void reverse_segment(solution *sol, const int i, const int j) {
    for (int k = 0; k < (j - i + 1) / 2; k++) {
        int temp = sol->visited_nodes[i + k];
        sol->visited_nodes[i + k] = sol->visited_nodes[j - k];
        sol->visited_nodes[j - k] = temp;
    }
}

void shift_segment(solution *sol, const int n, const int idx1, const int idx2, const int idx3) {
    
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

double delta2(const instance *inst, const solution *sol, const int i, const int j) {
    
    // compute the cost of the two edges that would be removed
    double old_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[i], inst) +
                      cost(sol->visited_nodes[j], sol->visited_nodes[j+1], inst);

    // compute the cost of the two new edges that would be added
    double new_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[j], inst) +
                      cost(sol->visited_nodes[i], sol->visited_nodes[j+1], inst);

    return new_cost - old_cost;
}

double delta3(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3) {
    
    // compute the cost of the three edges that would be removed
    double old_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[idx1+1], inst) +
                      cost(sol->visited_nodes[idx2], sol->visited_nodes[idx2+1], inst) +
                      cost(sol->visited_nodes[idx3], sol->visited_nodes[idx3+1], inst);
        
    // compute the cost of the three new edges that would be added
    double new_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[idx2+1], inst) +
                      cost(sol->visited_nodes[idx2], sol->visited_nodes[idx3+1], inst) +
                      cost(sol->visited_nodes[idx3], sol->visited_nodes[idx1+1], inst);
        
    return new_cost - old_cost;
}

//---------------heuristics------------------

void nearest_neighbor(const instance *inst, solution *sol, const int start)
{
    solution temp_sol; 
    allocate_solution(&temp_sol, inst->nnodes);

    initialize_tour(temp_sol.visited_nodes, inst->nnodes);
    swap_nodes(temp_sol.visited_nodes, start, 0);

    int len = 1;
    double total_cost = 0;
    for (int i = 1; i < inst->nnodes; i++)
    {
        int nearest_index = find_nearest_node(inst, len, temp_sol.visited_nodes);
        if (nearest_index == -1)
        {
            printf("No valid nearest neighbor found\n");
            exit(EXIT_FAILURE);
        }

        int nearest_node = temp_sol.visited_nodes[nearest_index];
        swap_nodes(temp_sol.visited_nodes, nearest_index, len);
        total_cost += cost(temp_sol.visited_nodes[len - 1], nearest_node, inst);
        len++;
    }
    // Complete the cycle
    temp_sol.visited_nodes[inst->nnodes] = start;
    total_cost += cost(temp_sol.visited_nodes[len - 1], start, inst);

    temp_sol.cost = total_cost;

    if (inst->verbose >= GOOD) {
        check_sol(inst, &temp_sol);
    }

    strcpy(temp_sol.method, NEAREST_NEIGHBOR);
    copy_sol(sol, &temp_sol, inst->nnodes);
    free_solution(&temp_sol);
}

void multi_start_nn(const instance *inst, solution *sol, const int timelimit) {

    time_t t_start = seconds();

    solution temp_sol; 
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol; 
    copy_sol(&temp_best_sol, sol, inst->nnodes);

    double elapsed_time;
    for (int start = 0; start < inst->nnodes; start++) {

        elapsed_time = get_elapsed_time(t_start);
        
        if (elapsed_time >= timelimit) { // Stop if time limit is reached
            if (inst->verbose >= GOOD) {
                printf("Time limit reached.\n");
            }
            break;
        }

        nearest_neighbor(inst, &temp_sol, start);
        two_opt(inst, &temp_sol, (timelimit-elapsed_time), false);

        // Print intermediate results and check the solution
        if (inst->verbose >= GOOD) {
            printf("Start Node: %d, Cost: %.2lf\n", start, temp_sol.cost);
            check_sol(inst, &temp_sol);
        }
        update_sol(inst, &temp_best_sol, &temp_sol, true);
    }

    strcpy(temp_best_sol.method, NN_TWOOPT);
    update_sol(inst, sol, &temp_best_sol, false);
    free_solution(&temp_sol);
    free_solution(&temp_best_sol);

}

void two_opt(const instance *inst, solution *sol, const int timelimit, bool print) {

    time_t t_start = seconds();

    solution temp_sol; 
    copy_sol(&temp_sol, sol, inst->nnodes);

    int improved = 1; // Flag to track improvements
    while (improved && (get_elapsed_time(t_start) < timelimit)) {
        improved = 0;
        for (int i = 1; i < inst->nnodes; i++) {
            for (int j = i + 1; j < inst->nnodes; j++) {
                if (get_elapsed_time(t_start) >= timelimit) {
                    break; 
                }
                
                double delta = delta2(inst, &temp_sol, i, j);

                if (delta < - EPSILON) {
                    // Reverse the segment between i and j
                    reverse_segment(&temp_sol, i, j);
                    
                    // Update the solution cost correctly
                    temp_sol.cost += delta;
                    improved = 1; // Indicate improvement found
                }
            }
        }
    }

    if (inst->verbose >= GOOD) {
        check_sol(inst, &temp_sol);
    }

    strcpy(temp_sol.method, TWO_OPT);
    update_sol(inst, sol, &temp_sol, print);
    free_solution(&temp_sol);
}
