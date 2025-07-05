#include "hard_fixing.h"

// Hard fixing algorithm
void hard_fixing(instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, HARD_FIXING) == 0);
    
    // Set parameters for B&C
    inst->param2 = 1; 
    inst->param3 = 1; 

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    install_callback(inst, env, lp);

    // Instantiate memory for cplex
    int *succ = (int *)malloc(inst->nnodes * sizeof(int));
    int *comp = (int *)malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *)malloc(inst->ncols * sizeof(double));
    int ncomp;
    if (succ == NULL || comp == NULL || xstar == NULL) print_error("hard_fixing(): Cannot allocate memory");
   
    int iter = 0;

    // Set the percentages of nodes to fix
    const double percentages[] = {0.4, 0.5, 0.6, 0.8};
    const int num_options = 4;

    double percentage = (inst->param1 > 1) ? ((double)inst->param1)/100.0 : percentages[rand() % num_options];

    // Parameters for tree depth control
    int starting_depth = 0;  // Default starting tree depth
    
    int depth_increment = 5;  // Increasing depth step
    int current_depth = starting_depth;  // Track current depth
    
    double residual_time;
    
    // Count for actual fixed edges
    int fixed_count;

    // CSV file setup for plotting
    char filename[FILE_NAME_LEN];
    sprintf_s(filename, FILE_NAME_LEN, "HF_p%d", inst->param1);

    FILE *f = NULL;
    if (inst->verbose >= ONLY_INCUMBMENT && is_asked_method) {

        char filename_results[FILE_NAME_LEN];
        sprintf_s(filename_results, FILE_NAME_LEN, "results/%s.csv", filename);

        if (fopen_s(&f, filename_results, "w+")) print_error("hard_fixing(): Cannot open file");

    }

    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0) {
        
        // Warm up the model with best current solution
        warm_up(inst, sol, env, lp);
        
        // Fix edges in the model
        fixed_count = set_lowerbounds(inst, sol, env, lp, percentage);

        if (inst->verbose >= GOOD) {

            printf("\tFixed %5d out of %5d edges (%10.6f%%)\n", iter, fixed_count, inst->nnodes, 100.0 * fixed_count / inst->nnodes);

        }

        // Set time limit (still needed as a safety measure)
        CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);
        
        // Turn off node limit - we're controlling by depth instead
        CPXsetintparam(env, CPX_PARAM_NODELIM, 2100000000);
        
        // Set tree depth limit for this iteration
        CPXsetintparam(env, CPX_PARAM_TRELIM, current_depth);
        
        // Solve with CPLEX
        get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

        // Reset edges bounds
        reset_lowerbounds(inst, env, lp);

        build_solution_from_CPLEX(inst, &temp_sol, succ);

        double old_cost = sol->cost;
        bool u = update_sol(inst, sol, &temp_sol, false);
        updated = updated || u;
        
        if (inst->verbose >= ONLY_INCUMBMENT && is_asked_method) {

            if (u) {

                printf(" * ");

            } else {

                printf("   ");

            }

            printf("Iteration %5d, Incumbment %10.6lf, Heuristic solution cost %10.6lf, Hard fixing percentage %10.6f%%, tree depth %5d, Residual time %10.6lf\n", 
                iter, old_cost, temp_sol.cost, percentage * 100, current_depth, residual_time);

            fprintf(f, "%d,%f,%f\n", iter, temp_sol.cost, sol->cost);

        }
        
        iter++;

        if (inst->param1 == 1) {

            // Randomly change the fixing percentage
            percentage = percentages[rand() % num_options];

        }

        // Increase depth for the next iteration if no improvements
        if (!u) {

            current_depth += depth_increment;

        }

    }

    if (updated) {

        sprintf_s(sol->method, METH_NAME_LEN, filename);

    }

    // Close the file if it was opened
    if (f != NULL) {

        fclose(f);

    }

    if (inst->verbose >= ONLY_INCUMBMENT && is_asked_method) {

        plot_stats_in_file(filename);

    }

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);

    free_CPLEX(&env, &lp);

    free_solution(&temp_sol);

}

// Fix some edges of the solution in the model
int set_lowerbounds(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp, const double p) {

    // Allocate memory for the constraints
    int *edge_indices = (int *)malloc(inst->nnodes * sizeof(int));
    char *lu = (char *)malloc(inst->nnodes * sizeof(char));
    double *bd = (double *)malloc(inst->nnodes * sizeof(double));
    if(edge_indices == NULL || lu == NULL || bd == NULL) print_error("set_lowerbounds(): Cannot allocate memory");

    int fixed_count = 0;
        
    for (int i = 0; i < inst->nnodes; i++) {

        int edge_idx = xpos(sol->visited_nodes[i], sol->visited_nodes[i+1], inst);

        double rand_val = random01();

        if (rand_val < p) {

            // To fix an edge you need to fix the lower bound equal to 1.0 (the value of the upper bound)
            edge_indices[fixed_count] = edge_idx;
            lu[fixed_count] = 'L';
            bd[fixed_count] = 1.0;
            fixed_count++;

        }

    }

    // Fix edges in one batch operation
    if (fixed_count > 0) {

        if (CPXchgbds(env, lp, fixed_count, edge_indices, lu, bd)) print_error("hard_fixing(): Error in setting bounds");
        
    }

    // Free allocated memory
    free(edge_indices);
    free(lu);
    free(bd);

    return fixed_count;

}

void reset_lowerbounds(const instance *inst, CPXENVptr env, CPXLPptr lp) {

    int *edge_indices = (int *)malloc(inst->ncols * sizeof(int));
    char *lu = (char *) malloc(inst->ncols * sizeof(char));
    double *bd = (double *) malloc(inst->ncols * sizeof(double));
    
    for (int i=0; i<inst->ncols; i++) {

        // To reset an edge you need to fix the lower bound equal to 0.0
        edge_indices[i] = i;
        lu[i] = 'L';
        bd[i] = 0.0;

    }

    if (CPXchgbds(env, lp, inst->ncols, edge_indices, lu, bd)) print_error("hard_fixing(): Error in resetting bounds");
        
    // Check if the bounds are reset correctly
    if (inst->verbose >= GOOD) {

        if(CPXgetlb(env, lp, bd, 0, inst->ncols-1)) print_error("CPXgetlb(): Cannot get the lowerbounds");

        for (int i=0; i<inst->ncols; i++) {

            if (bd[i] != 0.0) print_error("reset_lowerbounds(): Error in reset lowerbounds");

        }

    }

}