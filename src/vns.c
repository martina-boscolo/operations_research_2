#include "vns.h"

void vns(const instance *inst, solution *sol, const double timelimit, const int k, const int reps) {

    double t_start = get_time_in_milliseconds();

    solution temp_sol; 
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol; 
    copy_sol(&temp_best_sol, sol, inst->nnodes);

    char method_name[50];
    sprintf(method_name, "%s_k%d_r%d", VNS, k, reps);

    FILE* f;
    if (inst->verbose >= ONLY_INCUMBMENT) {
        char filename[65];
        sprintf(filename, "results/%s.csv", method_name);
        f = fopen(filename, "w+");
    }
    int iteration = 0;

    double elapsed_time;
    while ((elapsed_time = get_elapsed_time(t_start)) < timelimit) {

        if (inst->verbose >= DEBUG){
            printf("Time left: %d \n",timelimit - elapsed_time);
        }

        // go to local optima
        two_opt(inst, &temp_sol, (timelimit-elapsed_time), false);

        // update local best solution
        update_sol(inst, &temp_best_sol, &temp_sol, false);
        
        if (inst->verbose >= ONLY_INCUMBMENT) {
            fprintf(f, "%d,%f,%f\n", iteration, temp_sol.cost, temp_best_sol.cost);
        }

        // escape local minima
        kick(inst, &temp_sol, k, reps);

        if (inst->verbose >= ONLY_INCUMBMENT) {
            fprintf(f, "%d,%f,%f\n", iteration, temp_sol.cost, temp_best_sol.cost);
        }
        
        iteration++;
    }

    strcpy(temp_best_sol.method, method_name);
    update_sol(inst, sol, &temp_best_sol, true);
    if (inst->verbose >= ONLY_INCUMBMENT) {
        plot_stats_in_file(method_name);
    }
    fclose(f);

}

void kick(const instance *inst, solution *sol, const int k, const int reps) {

    switch (k) {
        default: // default case uses 3opt move as kick
        case 3:

            if (inst->verbose >= GOOD) {
                printf("Kick with 3-opt move, with %d repetitions\n", reps);
            }

            for (int i = 0; i < reps; i++) {

                int idx1, idx2, idx3;
                select_three_indices(inst->nnodes, &idx1, &idx2, &idx3);

                int move = rand() % POSSIBLE_THREE_OPT_MOVES + 1;

                if (inst->verbose >= GOOD) {
                    printf("Kick %d with %d, %d, %d\n", move, idx1, idx2, idx3);
                }
                
                switch (move)
                {
                    case 1:
                        fixed_three_opt_move1(inst, sol, idx1, idx2, idx3);
                        break;

                    case 2:
                        fixed_three_opt_move2(inst, sol, idx1, idx2, idx3);
                        break;
                    
                    case 3:
                        fixed_three_opt_move1(inst, sol, idx1, idx2, idx3);
                        break;
                    
                    case 4:
                        fixed_three_opt_move1(inst, sol, idx1, idx2, idx3);
                        break;
                    
                    default:
                        break;
                }
                
                if (inst->verbose >= GOOD) {
                    check_sol(inst, sol);
                }
            }

            break;

        case 5:
        
            if (inst->verbose >= GOOD) {
                printf("Kick with 5-opt move, with %d repetitions\n", reps);
            }

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

void fixed_three_opt_move1(const instance *inst, solution *sol, int idx1, int idx2, int idx3) {

    // Update the solution cost
    sol->cost += delta3(inst, sol, idx1, idx2, idx3);
                
    // Perform the move
    shift_segment(sol, inst->nnodes, idx1, idx2, idx3);

}

void fixed_three_opt_move2(const instance *inst, solution *sol, int idx1, int idx2, int idx3) {

    // Update the solution cost
    sol->cost += delta2(inst, sol, idx1+1, idx2);
                
    // Perform the move
    reverse_segment(sol, idx1+1, idx2);

    // Update the solution cost
    sol->cost += delta2(inst, sol, idx2+1, idx3);
                
    // Perform the move
    reverse_segment(sol, idx2+1, idx3);

}

void fixed_three_opt_move3(const instance *inst, solution *sol, int idx1, int idx2, int idx3) {

    int segmentC_size = idx3 - idx2;

    // Update the solution cost
    sol->cost += delta3(inst, sol, idx1, idx2, idx3);
                
    // Perform the move
    shift_segment(sol, inst->nnodes, idx1, idx2, idx3);

    idx2 = idx1 + segmentC_size;

    // Update the solution cost
    sol->cost += delta2(inst, sol, idx1+1, idx2);
                
    // Perform the move
    reverse_segment(sol, idx1+1, idx2);

}

void fixed_three_opt_move4(const instance *inst, solution *sol, int idx1, int idx2, int idx3) {

    int segmentB_size = idx2 - idx1;
    int segmentC_size = idx3 - idx2;

    // Update the solution cost
    sol->cost += delta3(inst, sol, idx1, idx2, idx3);
                
    // Perform the move
    shift_segment(sol, inst->nnodes, idx1, idx2, idx3);

    idx2 = idx1 + segmentC_size;
    idx3 = idx2 + segmentB_size;

    // Update the solution cost
    sol->cost += delta2(inst, sol, idx2+1, idx3);
                
    // Perform the move
    reverse_segment(sol, idx2+1, idx3);

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
