#include "heuristics.h"

//--------------- heuristic utilities ------------------

// Find the next nearest node
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

// Swap two nodes in the array
void swap_nodes(int *nodes, const int i, const int j) {

    int temp = nodes[i];
    nodes[i] = nodes[j];
    nodes[j] = temp;

}

// Reverse the segment between the two indeces in the solution
void reverse_segment(solution *sol, const int i, const int j) {

    for (int k = 0; k < (j - i + 1) / 2; k++) {

        int temp = sol->visited_nodes[i + k];
        sol->visited_nodes[i + k] = sol->visited_nodes[j - k];
        sol->visited_nodes[j - k] = temp;

    }

}

// Reorder tour segments from A -> B -> C to A -> C -> B
void shift_segment(solution *sol, const int n, const int idx1, const int idx2, const int idx3) {
    
    // Calculate segment sizes
    int segment1_size = (idx2+1) - (idx1+1);
    int segment2_size = (idx3+1) - (idx2+1);
    int segment3_size = n - (idx3+1) + (idx1+1);
    
    // Allocate memory for segments
    int *segment1 = (int *)malloc(segment1_size * sizeof(int));
    int *segment2 = (int *)malloc(segment2_size * sizeof(int));
    int *segment3 = (int *)malloc(segment3_size * sizeof(int));

    if (segment1 == NULL || segment2 == NULL || segment3 == NULL) print_error("shift_segment(): Cannot allocate memory");
    
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

// Compute the delta cost after the 2-opt move
double delta2(const instance *inst, const solution *sol, const int i, const int j) {
    
    // Compute the cost of the two edges that would be removed
    double old_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[i], inst) +
                      cost(sol->visited_nodes[j], sol->visited_nodes[j+1], inst);

    // Compute the cost of the two new edges that would be added
    double new_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[j], inst) +
                      cost(sol->visited_nodes[i], sol->visited_nodes[j+1], inst);

    return new_cost - old_cost;

}

// Compute the delta cost after the 3-opt move
double delta3(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3) {
    
    // Compute the cost of the three edges that would be removed
    double old_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[idx1+1], inst) +
                      cost(sol->visited_nodes[idx2], sol->visited_nodes[idx2+1], inst) +
                      cost(sol->visited_nodes[idx3], sol->visited_nodes[idx3+1], inst);
        
    // Compute the cost of the three new edges that would be added
    double new_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[idx2+1], inst) +
                      cost(sol->visited_nodes[idx2], sol->visited_nodes[idx3+1], inst) +
                      cost(sol->visited_nodes[idx3], sol->visited_nodes[idx1+1], inst);
        
    return new_cost - old_cost;

}

//---------------------------------------- heuristics ----------------------------------------

// Nearest Neighbor algorithm
void nearest_neighbor(const instance *inst, solution *sol, const int start) {

    initialize_tour(sol->visited_nodes, inst->nnodes);
    swap_nodes(sol->visited_nodes, start, 0);
    
    int len = 1;
    sol->cost = 0;

    for (int i = 1; i < inst->nnodes; i++) {

        int nearest_index = find_nearest_node(inst, len, sol->visited_nodes);

        if (nearest_index == -1) {

            print_error("nearest_neighbor(): No valid nearest neighbor found");

        }

        // Update tour with nearest node
        int nearest_node = sol->visited_nodes[nearest_index];
        swap_nodes(sol->visited_nodes, nearest_index, len);
        sol->cost += cost(sol->visited_nodes[len - 1], nearest_node, inst);

        len++;

    }

    // Complete the cycle
    sol->visited_nodes[inst->nnodes] = start;
    sol->cost += cost(sol->visited_nodes[len - 1], start, inst);

    if (inst->verbose >= GOOD) {

        check_sol(inst, sol);

    }

    strncpy_s(sol->method, METH_NAME_LEN, NEAREST_NEIGHBOR, _TRUNCATE);

}

// Multi-start approach for nearest neighbor
void multi_start_nn(const instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, NN_TWOOPT) == 0);

    solution temp_sol; 
    copy_sol(&temp_sol, sol, inst->nnodes);

    double elapsed_time;

    for (int start = 0; start < inst->nnodes; start++) {

        elapsed_time = get_elapsed_time(t_start);

        if (inst->verbose >= GOOD) {

            printf("Remaining time %10.5lf\n", timelimit-elapsed_time);

        }
        
        if (elapsed_time >= timelimit) { // Stop if time limit is reached

            break;

        }

        nearest_neighbor(inst, &temp_sol, start);
        two_opt(inst, &temp_sol, (timelimit-elapsed_time), false);

        // Print intermediate results and check the solution
        if (inst->verbose >= GOOD) {

            printf("Start Node: %5d, Cost: %10.6lf\n", start, temp_sol.cost);
            check_sol(inst, &temp_sol);

        }

        bool val = update_sol(inst, sol, &temp_sol, is_asked_method);
        updated = updated || val;

    }

    if (updated) {

        strncpy_s(sol->method, METH_NAME_LEN, NN_TWOOPT, _TRUNCATE);

    }

}

// Extra Mileage algorithm
void extra_mileage(const instance *inst, solution *sol) {

    initialize_tour(sol->visited_nodes, inst->nnodes);

    // Initialize all nodes as unvisited
    bool *visited = (bool *)calloc(inst->nnodes, sizeof(bool));
    if (visited == NULL) print_error("extra_mileage(): Cannot allocate memory");

    int node1 = 0;
    int node2 = 1;
    double max_cost = -1;

    for (int i=0; i<inst->nnodes; i++) {

        for (int j=i+1; j<inst->nnodes; j++) {

            if (fabs(cost(i,j, inst) - max_cost) > EPSILON) {

                node1 = i;
                node2 = j;
                max_cost = cost(i,j, inst);

            }

        }

    }

    // Start with a partial tour containing only the most distant nodes
    sol->visited_nodes[0] = node1;
    sol->visited_nodes[1] = node2; 
    sol->visited_nodes[2] = node1; // Close the tour initially
    visited[node1] = true;
    visited[node2] = true;
    
    int tour_size = 2; // Number of nodes currently in the tour

    // Iteratively add nodes to the tour
    while (tour_size < inst->nnodes) {

        double min_extra_cost = INFINITY;
        int best_node = -1;
        int best_position = -1;

        // Find the unvisited node and position that minimizes extra mileage
        for (int node = 0; node < inst->nnodes; node++) {

            if (visited[node]) continue; // Skip visited nodes

            // Try inserting this node at each possible position in the tour
            for (int pos = 1; pos <= tour_size; pos++) {

                int prev_node = sol->visited_nodes[pos - 1];
                int next_node = sol->visited_nodes[pos];

                // Calculate extra cost of inserting node between prev_node and next_node
                double old_edge_cost = cost(prev_node, next_node, inst);
                double new_edges_cost = cost(prev_node, node, inst) + cost(node, next_node, inst);
                double extra_cost = new_edges_cost - old_edge_cost;

                if (extra_cost < min_extra_cost) {
 
                    min_extra_cost = extra_cost;
                    best_node = node;
                    best_position = pos;

                }

            }

        }

        // Insert the best node at the best position
        if (best_node != -1) {

            // Shift nodes to make room for insertion
            for (int i = tour_size; i >= best_position; i--) {

                sol->visited_nodes[i + 1] = sol->visited_nodes[i];

            }
            
            // Insert the new node
            sol->visited_nodes[best_position] = best_node;
            visited[best_node] = true;
            tour_size++;

        } else {

            print_error("extra_mileage(): No valid node to insert found");

        }

    }

    // Ensure the tour is properly closed
    sol->visited_nodes[inst->nnodes] = sol->visited_nodes[0];

    // Calculate the total cost of the complete tour
    sol->cost = compute_solution_cost(inst, sol);

    if (inst->verbose >= GOOD) {

        check_sol(inst, sol);

    }

    strncpy_s(sol->method, METH_NAME_LEN, EXTRA_MILEAGE, _TRUNCATE);
    
    free(visited);

}

// Implementation of 2-opt for refinement of the solution
void two_opt(const instance *inst, solution *sol, const double timelimit, bool print) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, TWO_OPT) == 0);

    int improved = true; // Flag to track improvements
    while (improved && (get_elapsed_time(t_start) < timelimit)) {

        double best_delta = INFINITY;
        int best_i = -1, best_j = -1;

        improved = false;

        // Get the best delta
        for (int i = 1; i < inst->nnodes; i++) {

            for (int j = i + 1; j < inst->nnodes; j++) {

                if (get_elapsed_time(t_start) >= timelimit) break;
                
                double delta = delta2(inst, sol, i, j);

                if (delta < best_delta- EPSILON) {

                    best_delta = delta;
                    best_i = i;
                    best_j = j;

                }

            }

        }

        if (best_delta < - EPSILON) {

            if(print && inst->verbose >= ONLY_INCUMBMENT) {

                printf("Incumbment updated\nOld cost: %10.6lf,\tNew cost: %10.6lf\n", sol->cost, sol->cost + best_delta);

            }

            // Reverse the segment between i and j
            reverse_segment(sol, best_i, best_j);
            
            // Update the solution cost correctly
            sol->cost += best_delta;
            improved = true; // Indicate improvement found
            updated = true;

        }
        
    }

    if (inst->verbose >= GOOD) {

        check_sol(inst, sol);

    }

    if (updated && is_asked_method) {

        strncpy_s(sol->method, METH_NAME_LEN, TWO_OPT, _TRUNCATE);

    }

}
