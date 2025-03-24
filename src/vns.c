#include "vns.h"

void vns(instance *inst, solution *sol, const int reps) {

    FILE* f = fopen("results/VNS.csv", "w+");
    int iteration = 0;
    while (get_elapsed_time(inst->t_start) < inst->timelimit) {

        // go to local optima
        two_opt(inst, sol);

        // update current best solution
        update_best_sol(inst, sol);
        
        fprintf(f, "%d,%f,%f\n", iteration, sol->cost, inst->best_solution->cost);

        if (inst->verbose >= DEBUG){
            printf("Time left: %lf \n", inst->timelimit -get_elapsed_time(inst->t_start) );
        }
        // escape local minima
        kick(inst, sol, reps);

        fprintf(f, "%d,%f,%f\n", iteration, sol->cost, inst->best_solution->cost);
        
        iteration++;
    }

    strcpy(inst->best_solution->method, "VNS");
    plot_stats_in_file(inst->best_solution->method);

}

void select_three_indices(int n, int *idx1, int *idx2, int *idx3) {

        *idx1 = rand() % n;
 
        do {
            *idx2 = rand() % n;
        } while (*idx2 == *idx1);
        
        do {
            *idx3 = rand() % n;
        } while (*idx3 == *idx1 || *idx3 == *idx2);
        
        // Sort the indices
        if (*idx1 > *idx2) { int temp = *idx1; *idx1 = *idx2; *idx2 = temp; }
        if (*idx1 > *idx3) { int temp = *idx1; *idx1 = *idx3; *idx3 = temp; }
        if (*idx2 > *idx3) { int temp = *idx2; *idx2 = *idx3; *idx3 = temp; }
    }

void kick(instance *inst, solution *sol, const int reps) { //aggiungere int reps 
    int n = inst->nnodes;
    
    for (int i = 0; i < reps; i++) {

        int idx1, idx2, idx3;
        select_three_indices(n, &idx1, &idx2, &idx3);
        
        // Calculate the cost of the three edges that would be removed
        double old_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[(idx1+1) % n], inst) +
                          cost(sol->visited_nodes[idx2], sol->visited_nodes[(idx2+1) % n], inst) +
                          cost(sol->visited_nodes[idx3], sol->visited_nodes[(idx3+1) % n], inst);
        
        // Calculate the cost of the three new edges that would be added
        // For a standard 3-opt move, we would connect:
        // idx1 to (idx2+1), idx2 to (idx3+1), and idx3 to (idx1+1)
        double new_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[(idx2+1) % n], inst) +
                          cost(sol->visited_nodes[idx2], sol->visited_nodes[(idx3+1) % n], inst) +
                          cost(sol->visited_nodes[idx3], sol->visited_nodes[(idx1+1) % n], inst);
        
        // Save the original cost for validation
        double original_cost = sol->cost;
        
        // Perform the move
        shift_segment(inst, sol, idx1, idx2, idx3);
        
        // Update the solution cost
        sol->cost = original_cost + (new_cost - old_cost);
        
        check_sol(inst, sol);
    }
}