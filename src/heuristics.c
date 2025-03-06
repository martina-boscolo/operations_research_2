#include "heuristics.h"

/* Allocate memory for a solution */
void allocate_solution(solution *sol, int nnodes) {
    sol->visited_nodes = (int *)malloc((nnodes + 1) * sizeof(int));
    if (!sol->visited_nodes) {
        fprintf(stderr, "Memory allocation failed for solution\n");
        exit(EXIT_FAILURE);
    }
}

double euclidean_distance(coordinate a, coordinate b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}


//TODO use the functions already existing
void compute_cost_matrix(instance *inst) {
    inst->costs = (double *)malloc(inst->nnodes * inst->nnodes * sizeof(double));
    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = 0; j < inst->nnodes; j++) {
            if (i == j) {
                inst->costs[i * inst->nnodes + j] = INF;
            } else {
                inst->costs[i * inst->nnodes + j] = euclidean_distance(inst->coord[i], inst->coord[j]);
            }
        }
    }
}

void nearest_neighbor(instance *inst, solution *sol, int start) {
    // Free previous allocations if necessary
    if (sol->visited_nodes != NULL) {
        free(sol->visited_nodes);
    }


    /* Allocate memory for a solution */
    allocate_solution(sol, inst->nnodes);


    // Allocate memory for visited nodes
    sol->visited_nodes = (int *)malloc((inst->nnodes + 1) * sizeof(int));
    if (sol->visited_nodes == NULL) {
        fprintf(stderr, "Memory allocation failed for visited_nodes\n");
        exit(EXIT_FAILURE);
    }

    // Allocate and initialize visited array
    int *visited = (int *)calloc(inst->nnodes, sizeof(int));
    if (visited == NULL) {
        fprintf(stderr, "Memory allocation failed for visited array\n");
        exit(EXIT_FAILURE);
    }

    sol->visited_nodes[0] = start;
    visited[start] = 1;
    double total_cost = 0;
    int current = start;

    for (int i = 1; i < inst->nnodes; i++) {
        int nearest = -1;
        double min_cost = INF;

        for (int j = 0; j < inst->nnodes; j++) {
            if (!visited[j] && inst->costs[current * inst->nnodes + j] < min_cost) {
                min_cost = inst->costs[current * inst->nnodes + j];
                nearest = j;
            }
        }

        // Ensure a valid nearest node was found (should always be the case in a connected graph)
        if (nearest == -1) {
            fprintf(stderr, "Error: No valid nearest neighbor found\n");
            exit(EXIT_FAILURE);
        }

        sol->visited_nodes[i] = nearest;
        visited[nearest] = 1;
        total_cost += min_cost;
        current = nearest;
    }

    // Complete the cycle
    sol->visited_nodes[inst->nnodes] = start;  
    total_cost += inst->costs[current * inst->nnodes + start];  

    // Store solution cost and method name
    sol->cost = total_cost;
    strcpy(sol->method, "nearest_neighbor");

    // Free temporary memory
    free(visited);
    validate_solution(sol, inst, sol->cost, "Nearest Neighbor");
}

/* Multi-start approach with time limit */
void multi_start_nn(instance *inst, solution *sol) {
    clock_t start_time = clock();
    
    // Allocate memory for the best solution
    inst->best_solution = (solution *)malloc(sizeof(solution));
    inst->best_solution->cost = INF;
    inst->best_solution->visited_nodes = (int *)malloc((inst->nnodes + 1) * sizeof(int));

    for (int start = 0; start < inst->nnodes; start++) {
        double elapsed_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;

        if (inst->timelimit > 0 && elapsed_time >= inst->timelimit) {
            printf("Time limit reached! Stopping early.\n");
            break; // Stop if time limit is reached
        }

        nearest_neighbor(inst, sol, start);
        two_opt(inst, sol);  // Apply 2-opt refinement

        // TODO: remove, this is just for debug
        // Print the path and cost for each start node
        // printf("Start Node: %d, Cost: %.2lf, Path: ", start, sol->cost);
        // for (int i = 0; i <= inst->nnodes; i++) {
        //     printf("%d ", sol->visited_nodes[i]);
        // }
        // printf("\n");
        // Print intermediate results
        printf("Start Node: %d, Cost: %.2lf\n", start, sol->cost);

        // If the new solution is better, update best_solution
        if (sol->cost < inst->best_solution->cost) {
            inst->best_solution->cost = sol->cost;
            memcpy(inst->best_solution->visited_nodes, sol->visited_nodes, (inst->nnodes + 1) * sizeof(int));
            validate_solution(inst->best_solution, inst, inst->best_solution->cost, "Best Solution Update");
        }
    }
}

void two_opt(instance *inst, solution *sol) {
    int improved = 1; // Flag to track improvements
    while (improved) {
        improved = 0;

        for (int i = 1; i < inst->nnodes - 1; i++) {
            for (int j = i + 1; j < inst->nnodes; j++) {
                double old_cost = inst->costs[sol->visited_nodes[i-1] * inst->nnodes + sol->visited_nodes[i]] +
                                  inst->costs[sol->visited_nodes[j] * inst->nnodes + sol->visited_nodes[j+1]];
                
                double new_cost = inst->costs[sol->visited_nodes[i-1] * inst->nnodes + sol->visited_nodes[j]] +
                                  inst->costs[sol->visited_nodes[i] * inst->nnodes + sol->visited_nodes[j+1]];

                if (new_cost < old_cost) {
                    // Reverse the segment between i and j
                    for (int k = 0; k < (j - i + 1) / 2; k++) {
                        int temp = sol->visited_nodes[i + k];
                        sol->visited_nodes[i + k] = sol->visited_nodes[j - k];
                        sol->visited_nodes[j - k] = temp;
                    }

                    // Update cost
                    sol->cost -= (old_cost - new_cost);
                    improved = 1; // Indicate improvement found
                }
            }
        }
    }

    strcpy(sol->method, "2-opt_refinement");
    validate_solution(sol, inst, sol->cost, "2-opt Refinement");
}


int nn_main(instance *inst, solution *sol) {
    
    compute_cost_matrix(inst);
    multi_start_nn(inst, sol);

    printf("Best Solution Cost: %lf\n", inst->best_solution->cost);
    printf("Best Path: ");
    for (int i = 0; i <= inst->nnodes; i++) {
        printf("%d ", inst->best_solution->visited_nodes[i]);
    }
    printf("\n");
    validate_solution(inst->best_solution, inst, inst->best_solution->cost, "Final Check in nn_main");

    return 0;
}