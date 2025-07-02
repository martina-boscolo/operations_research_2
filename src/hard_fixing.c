#include "hard_fixing.h"

// Hard fixing algorithm
void hard_fixing(instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, HARD_FIXING) == 0);
    
    // Set parameters for 
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

    const double percentages[] = {0.4, 0.5, 0.6, 0.8};
    const int num_options = 4;

    double percentage = (inst->param1 > 1) ? ((double)inst->param1)/100.0: 0.8;

    // Parameters for tree depth control
    int starting_depth = 0;  // Default starting tree depth
    
    int depth_increment = 5;  // Increase depth by this amount each iteration
    int current_depth = starting_depth;  // Track current depth
    
    warm_up(inst, sol, env, lp);

    double residual_time;
    
    // Count for actual fixed edges
    int fixed_count;

    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0) {

        if (inst->verbose >= LOW) {

            printf("Hard fixing percentage = %10.6f%%\n", percentage * 100);
            printf("Current tree depth limit: %5d\n", current_depth);

        }
        
        // Fix edges in the model
        fixed_count = set_lowerbounds(inst, sol, env, lp, percentage);

        if (inst->verbose >= GOOD) {

            printf("Iteration %5d: Fixed %5d out of %5d edges (%10.6f%%)\n", iter, fixed_count, inst->nnodes, 100.0 * fixed_count / inst->nnodes);

        }

        // Set time limit (still needed as a safety measure)
        CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);
        
        // Turn off node limit - we're controlling by depth instead
        CPXsetintparam(env, CPX_PARAM_NODELIM, 2100000000);
        
        // Set tree depth limit for this iteration
        CPXsetintparam(env, CPX_PARAM_TRELIM, current_depth);
        
        // Solve with CPLEX
        get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);
        
        if (inst->verbose >= LOW) {

            printf("Iteration %5d, tree depth %5d, ", iter, current_depth);

        }

        // Reset edges bounds
        reset_lowerbounds(inst, env, lp);

        build_solution_from_CPLEX(inst, &temp_sol, succ);

        if (inst->verbose >= LOW) {

            printf("solution cost %10.6lf, incumbment %10.6lf\n", temp_sol.cost, sol->cost);

        }

        bool var = update_sol(inst, sol, &temp_sol, true);
        updated = updated || var;
        
        iter++;

        if (inst->param1 == 1) {

            // Randomly change the fixing percentage
            percentage = percentages[rand() % num_options];

        }

        // Increase the depth for the next iteration
        if (!var) {

            current_depth += depth_increment;

        }

    }

    if (updated) {

        strncpy_s(sol->method, METH_NAME_LEN, HARD_FIXING, _TRUNCATE);

    }
    
    if (inst->verbose >= GOOD && is_asked_method) {

        plot_solution(inst, sol);

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