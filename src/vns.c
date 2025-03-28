#include "vns.h"

void vns(const instance *inst, solution *sol, const double timelimit, const int k, const int reps) {

    double t_start = seconds();

    solution temp_sol; 
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol; 
    copy_sol(&temp_best_sol, sol, inst->nnodes);

    FILE* f = fopen("results/VNS.csv", "w+");
    int iteration = 0;

    double elapsed_time;
    while ((elapsed_time = get_elapsed_time(t_start)) < timelimit) {

        printf("\n\nBEFORE TWO_OPT\n");/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // go to local optima
        two_opt(inst, &temp_sol, (timelimit-elapsed_time));
        printf("AFTER TWO_OPT\n");/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // update local best solution
        update_sol(inst, &temp_best_sol, &temp_sol);
        
        fprintf(f, "%d,%f,%f\n", iteration, temp_sol.cost, temp_best_sol.cost);

        if (inst->verbose >= DEBUG){
            printf("Time left: %lf \n", timelimit - get_elapsed_time(t_start) );
        }
        // escape local minima
        kick(inst, &temp_sol, k, reps);

        fprintf(f, "%d,%f,%f\n", iteration, temp_sol.cost, temp_best_sol.cost);
        
        iteration++;
    }

    strcpy(temp_best_sol.method, VNS);
    update_sol(inst, sol, &temp_best_sol);
    plot_stats_in_file(sol->method);

}

void select_three_indices(const int n, int *idx1, int *idx2, int *idx3) {

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

void kick(const instance *inst, solution *sol, const int k, const int reps) {
    
    for (int i = 0; i < reps; i++) {

        int idx1, idx2, idx3;
        select_three_indices(inst->nnodes, &idx1, &idx2, &idx3);

        // Update the solution cost
        sol->cost += delta3(inst, sol, idx1, idx2, idx3);
        
        // Perform the move
        shift_segment(inst, sol, idx1, idx2, idx3);
        
        if (inst->verbose >= GOOD) {
            check_sol(inst, sol);
        }
    }

}