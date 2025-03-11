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
    while (improved) {
        improved = 0;
        for (int i = 1; i < inst->nnodes; i++) {
            for (int j = i + 1; j < inst->nnodes; j++) {
                // Calculate the cost of the two edges that would be removed
                double old_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[i], inst) +
                                 cost(sol->visited_nodes[j], sol->visited_nodes[j+1], inst);
               
                // Calculate the cost of the two new edges that would be added
                double new_cost = cost(sol->visited_nodes[i-1], sol->visited_nodes[j], inst) +
                                 cost(sol->visited_nodes[i], sol->visited_nodes[j+1], inst);
                
                if (new_cost < old_cost - EPSILON){
                    // Reverse the segment between i and j
                    for (int k = 0; k < (j - i + 1) / 2; k++) {
                        int temp = sol->visited_nodes[i + k];
                        sol->visited_nodes[i + k] = sol->visited_nodes[j - k];
                        sol->visited_nodes[j - k] = temp;
                    }
                    
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
