#include "hard_fixing.h"

void hard_fixing(instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol;
    copy_sol(&temp_best_sol, sol, inst->nnodes);

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
    
    // Instantiate memory for bounds
    int *edge_indices = (int *)malloc(inst->nnodes * sizeof(int));
    char *lu = (char *)malloc(inst->nnodes * sizeof(char));
    double *bd = (double *)malloc(inst->nnodes * sizeof(double));
    int fixed_count;

    if (succ == NULL || comp == NULL || xstar == NULL || edge_indices == NULL || lu == NULL || bd == NULL) {
        if (edge_indices) free(edge_indices);
        if (lu) free(lu);
        if (bd) free(bd);
        if (succ) free(succ);
        if (comp) free(comp);
        if (xstar) free(xstar);
        free_CPLEX(&env, &lp);
        free_solution(&temp_best_sol);
        free_solution(&temp_sol);
        print_error("Impossible to allocate memory.");
    }
   
    int iter = 0;

    const double percentages[] = {0.4, 0.5, 0.6, 0.8};
    const int num_options = 4;

    double percentage = (inst->param1 > 1) ? ((double)inst->param1)/100.0: 1.0; //0.8

    // Parameters for tree depth control
    int starting_depth = 0;  // Default starting tree depth
    int max_depth = 20;//(inst->param2 > 0) ? inst->param2 : 20;  // 
    int depth_increment = 5;  // Increase depth by this amount each iteration
    int current_depth = starting_depth;  // Track current depth
    
    warm_up(inst, &temp_best_sol, env, lp);

    double residual_time;

    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0 && current_depth <= max_depth) {

        if (inst->verbose >= LOW)
        {
            printf("\n\nHard fixing percentage = %.2f%%\n", percentage * 100);
            printf("Current tree depth limit: %d\n", current_depth);
        }
        
        // Fix edges in the model
        set_lowerbounds(inst, &temp_best_sol, env, lp, percentage, &fixed_count, edge_indices, lu, bd);

        if (inst->verbose >= GOOD)
        {
            printf("Iteration %d: Fixed %d out of %d edges (%.2f%%)\n",
                   iter, fixed_count, inst->nnodes, 100.0 * fixed_count / inst->nnodes);
        }

        // Set time limit (still needed as a safety measure)
        CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);
        
        // Turn off node limit - we're controlling by depth instead
        CPXsetintparam(env, CPX_PARAM_NODELIM, 2100000000);
        
        // Set tree depth limit for this iteration
        CPXsetintparam(env, CPX_PARAM_TRELIM, current_depth);
        
        // Solve with CPLEX
        int status = get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);
        
        if (inst->verbose >= LOW)
        {
            printf("Hard fixing iteration %d, tree depth: %d/%d --->", 
                   iter, current_depth, max_depth);
            if (status)
                printf("Unable to find solution\n\n\n");
            else
                printf("Found solution\n\n\n");
        }

        // Reset edges bounds
        reset_lowerbounds(env, lp, fixed_count, edge_indices, lu, bd);

        if (status == 0)
        {
            build_solution_from_CPLEX(inst, &temp_sol, succ);
            update_sol(inst, &temp_best_sol, &temp_sol, true);
        }
        iter++;

        if (inst->param1 == 1)
        {
            // Randomly change the fixing percentage
            percentage = percentages[rand() % num_options];
        }
        
        // Increase the depth for the next iteration
        current_depth += depth_increment;
    }

    strncpy_s(temp_best_sol.method, METH_NAME_LEN, "HardFixing", _TRUNCATE);
    update_sol(inst, sol, &temp_best_sol, false);
    
    if (inst->verbose >= GOOD)
    {
        plot_solution(inst, &temp_best_sol);
    }

    // Free allocated memory
    free(edge_indices);
    free(lu);
    free(bd);
    free(xstar);
    free(comp);
    free(succ);
    free_CPLEX(&env, &lp);
    free_solution(&temp_best_sol);
    free_solution(&temp_sol);

}

void set_lowerbounds(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp, 
                     const double p, int *fixed_count, int *edge_indices, char *lu, double *bd) 
{

    *fixed_count = 0;
        
    for (int i = 0; i < inst->nnodes; i++)
    {
        int next_idx = (i + 1) % inst->nnodes;
        int edge_idx = xpos(sol->visited_nodes[i], sol->visited_nodes[next_idx], inst);

        double rand_val = thread_safe_rand_01();
        if (rand_val < p)
        {
            edge_indices[*fixed_count] = edge_idx;
            lu[*fixed_count] = 'L';
            bd[*fixed_count] = 1.0;
            (*fixed_count)++;
        }
    }

    // Fix edges in one batch operation
    if (*fixed_count > 0)
    {
        if (CPXchgbds(env, lp, *fixed_count, edge_indices, lu, bd))
        {
            print_error("hard_fixing(): Error in setting bounds");
        }
    }

}

void reset_lowerbounds(CPXENVptr env, CPXLPptr lp, const int fixed_count, const int *edge_indices, char *lu, double *bd) {

    if (fixed_count > 0)
    {
        // to do : fix all the variables to 0 not only the one who changed 
        for (int i = 0; i < fixed_count; i++)
        {
            //lu[i] = 'L';
            bd[i] = 0.0;
        }

        if (CPXchgbds(env, lp, fixed_count, edge_indices, lu, bd))
        {
            print_error("hard_fixing(): Error in resetting bounds");
        }
    }
    // add a check to see if the bounds are reset correctly

}