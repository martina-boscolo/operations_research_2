#include "vns.h"

void vns(instance *inst, solution *sol) {

    // obtain first solution
    nearest_neighbor(inst, sol, rand() % inst->nnodes);

    FILE* f = fopen("results/VNSResults.csv", "w+");
    int iteration = 0;
    while (get_elapsed_time(inst->t_start) < inst->timelimit) {

        // go to local optima
        two_opt(inst, sol);

        // update current best solution
        update_best_sol(inst, sol);
        fprintf(f, "%d,%f\n", iteration, sol->cost);
        if (inst->verbose >=50){
            printf("Time left: %lf \n", inst->timelimit -get_elapsed_time(inst->t_start) );
        }
        // escape local minima
        kick(inst, sol);
        
        
        iteration++;
    }

    strcpy(inst->best_solution->method, "VNS");

    //to be moved in a function? 
    FILE *plot = open_plot();
    char filename[50];
    sprintf(filename, "VNSIterationsPlot");
    plot_in_file(plot, filename);
    plot_stats(plot, "results/VNSResults.csv");
    free_plot(plot);

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

// Perform a 3-opt move by rearranging tour segments
// A -> C -> B (reorder from A-B-C to A-C-B)
void perform_3opt_move(instance *inst, solution *sol, int idx1, int idx2, int idx3) {
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


void kick(instance *inst, solution *sol) {
    const int reps = 3;
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
        perform_3opt_move(inst, sol, idx1, idx2, idx3);
        
        // Update the solution cost
        sol->cost = original_cost + (new_cost - old_cost);
        
        check_sol(inst, sol);
    }
}