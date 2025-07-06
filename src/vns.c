#include "vns.h"

// VNS algorithm
void vns(const instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, VNS) == 0);

    solution temp_sol; 
    copy_sol(&temp_sol, sol, inst->nnodes);

    int k, reps;
    if (inst->param1 != 3 && inst->param1 != 5) { k = DEAULT_K; }
    if (inst->param2 < 1) { reps = DEFAULT_REPS; }

    char method_name[METH_NAME_LEN];
    sprintf_s(method_name, METH_NAME_LEN, "%s_k%d_r%d", VNS, k, reps);

    FILE* f = NULL;
    if (inst->verbose >= ONLY_INCUMBENT && is_asked_method) {

        char filename[FILE_NAME_LEN];
        sprintf_s(filename, FILE_NAME_LEN, "results/%s.csv", method_name);
        fopen_s(&f, filename, "w+");

    }

    int iteration = 0;

    double residual_time;
    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0) {

        // go to local optima
        two_opt(inst, &temp_sol, residual_time, false);

        // update local best solution
        double old_cost = sol->cost;
        bool u = update_sol(inst, sol, &temp_sol, false);
        updated = updated || u;
        
        if (inst->verbose >= ONLY_INCUMBENT && is_asked_method) {

            if (u) {

                printf(" * Iteration %5d, Incumbent %10.6lf, Heuristic solution cost %10.6lf, Kick %d, Repetitions %5d, Residual time %10.6lf\n", 
                    iteration, old_cost, temp_sol.cost, k, reps, residual_time);

            }

            fprintf(f, "%d,%f,%f\n", iteration, temp_sol.cost, sol->cost);

        }

        // escape local minima
        kick(inst, &temp_sol, k, reps);

        if (inst->verbose >= ONLY_INCUMBENT && is_asked_method) {

            fprintf(f, "%d,%f,%f\n", iteration, temp_sol.cost, sol->cost);

        }
        
        iteration++;

    }
    
    if (updated) {

        strncpy_s(sol->method, METH_NAME_LEN, method_name, _TRUNCATE);

    }
        
    // Close the file if it was opened
    if (f != NULL) {

        fclose(f);
    
    }
    
    if (inst->verbose >= ONLY_INCUMBENT && is_asked_method) {

        plot_stats_in_file(method_name);

    }

}

// Algorithm to modify the solution to escape the current local optima
void kick(const instance *inst, solution *sol, const int k, const int reps) {

    switch (k) {

        default: 
        case 3:

            for (int i = 0; i < reps; i++) {

                int idx1, idx2, idx3;
                select_three_indices(inst->nnodes, &idx1, &idx2, &idx3);

                if (inst->verbose >= DEBUG_V) {

                    printf("Kick with %5d, %5d, %5d\n", idx1, idx2, idx3);

                }
                
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

                if (inst->verbose >= DEBUG_V) {

                    printf("Kick with %5d, %5d, %5d, %5d, %5d\n", idx1, idx2, idx3,idx4, idx5);

                }
        
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

// Rearrange tour segments in the solution
void fixed_five_opt_move(solution *sol, const int nnodes, int idx1, int idx2, int idx3, int idx4, int idx5) {

    // Compute segmants's size
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

// Compute the delta cost after the 5-opt move
double delta5(const instance *inst, const solution *sol, const int idx1, const int idx2, const int idx3, const int idx4, const int idx5) {

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

// Select three random different indices in [0, n) and return them sorted
void select_three_indices(const int n, int *idx1, int *idx2, int *idx3) {

    *idx1 = rand() % n;
 
    do {

        *idx2 = rand() % n;

    } while (abs(*idx2 - *idx1) <= 1);
        
    do {

        *idx3 = rand() % n;

    } while (abs(*idx3 - *idx1) <= 1 || abs(*idx3 - *idx2) <= 1);
        
    // Sort the indices
    if (*idx1 > *idx2) { int temp = *idx1; *idx1 = *idx2; *idx2 = temp; }
    if (*idx1 > *idx3) { int temp = *idx1; *idx1 = *idx3; *idx3 = temp; }
    if (*idx2 > *idx3) { int temp = *idx2; *idx2 = *idx3; *idx3 = temp; }

}

// Select five random different indices in [0, n) and return them sorted
void select_five_indices(const int n, int *idx1, int *idx2, int *idx3, int *idx4, int *idx5) {

    *idx1 = rand() % n;
 
    do {

        *idx2 = rand() % n;

    } while (abs(*idx2 - *idx1) <= 1);
        
    do {

        *idx3 = rand() % n;

    } while (abs(*idx3 - *idx1) <= 1 || abs(*idx3 - *idx2) <= 1);

    do {

        *idx4 = rand() % n;

    } while (abs(*idx4 - *idx1) <= 1 || abs(*idx4 - *idx2) <= 1 || abs(*idx4 - *idx3) <= 1);

    do {

        *idx5 = rand() % n;

    } while (abs(*idx5 - *idx1) <= 1 || abs(*idx5 - *idx2) <= 1 || abs(*idx5 - *idx3) <= 1 || abs(*idx5 - *idx4) <= 1);
        
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
