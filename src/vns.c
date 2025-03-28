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

void kick(const instance *inst, solution *sol, const int k, const int reps) {

    switch (k) {
        default: // default case takes 3opt move as kick
        case 3:
            for (int i = 0; i < reps; i++) {

                int idx1, idx2, idx3;
                select_three_indices(inst->nnodes, &idx1, &idx2, &idx3);
        
                // Update the solution cost
                sol->cost += delta3(inst, sol, idx1, idx2, idx3);
                
                // Perform the move
                shift_segment(sol, inst->nnodes, idx1, idx2, idx3);
                
                if (inst->verbose >= GOOD) {
                    check_sol(inst, sol);
                }
            }
            break;

        case 5:
            for (int i = 0; i < reps; i++) {

                int idx1, idx2, idx3, idx4, idx5;
                select_five_indices(inst->nnodes, &idx1, &idx2, &idx3, &idx4, &idx5);
        
                // Update the solution cost
                sol->cost += delta5(inst, sol, idx1, idx2, idx3, idx4, idx5);
                
                // Perform the move
                fixed_five_opt_move(sol, inst->nnodes, idx1, idx2, idx3, idx4, idx5);
                
                if (inst->verbose >= GOOD) {
                    check_sol(inst, sol);
                }
            }
            break;
    }

}

void fixed_five_opt_move(solution *sol, const int nnodes, int idx1, int idx2, int idx3, int idx4, int idx5) {

    int segmentB_size = idx2 - idx1;
    int segmentC_size = idx3 - idx2;
    int segmentD_size = idx4 - idx3;
    int segmentE_size = idx5 - idx4;

    // A-B-D-C-E-F
    shift_segment(sol, nnodes, idx2, idx3, idx4);
    idx3 = idx2 + segmentD_size;
    idx4 = idx3 + segmentC_size;

    // A-D-B-C-E-F
    shift_segment(sol, nnodes, idx1, idx2, idx3);
    idx2 = idx1 + segmentD_size;
    idx3 = idx2 + segmentB_size;

    // A-D-B-E-C-F
    shift_segment(sol, nnodes, idx3, idx4, idx5);
    idx4 = idx3 + segmentE_size;
    idx5 = idx4 + segmentC_size;

    // A-D-!B-E-C-F
    reverse_segment(sol, idx2+1, idx3);

    // A-D-!B-E-!C-F
    reverse_segment(sol, idx4+1, idx5);

}

double delta5(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3, const int idx4, const int idx5) {
    
    int n = inst->nnodes;

    // compute the cost of the three edges that would be removed
    // (idx1, idx1+1), (idx2, idx2+1), (idx3, idx3+1), (idx4, idx4+1), (idx4, idx4+1)
    double old_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[idx1+1], inst) +
                      cost(sol->visited_nodes[idx2], sol->visited_nodes[idx2+1], inst) +
                      cost(sol->visited_nodes[idx3], sol->visited_nodes[idx3+1], inst) +
                      cost(sol->visited_nodes[idx4], sol->visited_nodes[idx4+1], inst) +
                      cost(sol->visited_nodes[idx5], sol->visited_nodes[idx5+1], inst);
        
    // compute the cost of the three new edges that would be added
    // add edges (idx1, idx3+1), (idx4, idx2), (idx1+1, idx4+1), (idx5, idx3), (idx2+1, idx5+1)
    double new_cost = cost(sol->visited_nodes[idx1], sol->visited_nodes[idx3+1], inst) +
                      cost(sol->visited_nodes[idx4], sol->visited_nodes[idx2], inst) +
                      cost(sol->visited_nodes[idx1+1], sol->visited_nodes[idx4+1], inst) +
                      cost(sol->visited_nodes[idx5], sol->visited_nodes[idx3], inst) +
                      cost(sol->visited_nodes[idx2+1], sol->visited_nodes[idx5+1], inst);
        
    return new_cost - old_cost;

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

void select_five_indices(const int n, int *idx1, int *idx2, int *idx3, int *idx4, int *idx5) {
    *idx1 = rand() % n;
 
    do {
        *idx2 = rand() % n;
    } while (*idx2 == *idx1);
        
    do {
        *idx3 = rand() % n;
    } while (*idx3 == *idx1 || *idx3 == *idx2);

    do {
        *idx4 = rand() % n;
    } while (*idx4 == *idx1 || *idx4 == *idx2 || *idx4 == *idx3);

    do {
        *idx5 = rand() % n;
    } while (*idx5 == *idx1 || *idx5 == *idx2 || *idx5 == *idx3 || *idx5 == *idx4);
        
    // Sort the indices
    if (*idx1 > *idx2) { int temp = *idx1; *idx1 = *idx2; *idx2 = temp; }
    if (*idx1 > *idx3) { int temp = *idx1; *idx1 = *idx3; *idx3 = temp; }
    if (*idx1 > *idx4) { int temp = *idx1; *idx1 = *idx4; *idx4 = temp; }
    if (*idx1 > *idx5) { int temp = *idx1; *idx1 = *idx5; *idx5 = temp; }

    if (*idx2 > *idx3) { int temp = *idx2; *idx2 = *idx3; *idx3 = temp; }
    if (*idx2 > *idx4) { int temp = *idx2; *idx2 = *idx4; *idx4 = temp; }
    if (*idx2 > *idx5) { int temp = *idx2; *idx2 = *idx5; *idx5 = temp; }
    
    if (*idx3 > *idx4) { int temp = *idx3; *idx3 = *idx4; *idx4 = temp; }
    if (*idx3 > *idx5) { int temp = *idx3; *idx3 = *idx5; *idx5 = temp; }

    if (*idx4 > *idx5) { int temp = *idx4; *idx4 = *idx5; *idx5 = temp; }
}
