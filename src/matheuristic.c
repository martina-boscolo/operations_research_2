#include "matheuristic.h"

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

    // Instantiate memory
    int *succ = (int *)malloc(inst->nnodes * sizeof(int));
    int *comp = (int *)malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *)malloc(inst->ncols * sizeof(double));
    int ncomp;

    if (succ == NULL || comp == NULL || xstar == NULL){
        if (succ != NULL) free(succ);
        if (comp != NULL) free(comp);
        if (xstar != NULL) free(xstar);
        free_CPLEX(&env, &lp);
        print_error("allocate_CPLEXsol(): Impossible to allocate memory.");
    }
   
    int iter = 0;

  
    double percentage = ((double)inst->param1)/100.0; 
    
    double local_timelimit = timelimit/5;
    double residual_time;

    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0) {
        int fixed_count = 0;
        warm_up(inst, &temp_best_sol, env, lp);
        for (int i=0; i<inst->nnodes; i++) {
            
            double rand  = thread_safe_rand_01(); // not sure if it needs to be thread safe
            if (rand < percentage) {

                int indeces[1] = { xpos(temp_best_sol.visited_nodes[i], temp_best_sol.visited_nodes[i+1], inst) };
                char lu[1] = {'L'};
                double bd[1] = {1.0};

                if (CPXchgbds(env, lp, 1, indeces, lu, bd)) {
                    print_error("hard_fixing(): Error in setting bounds");
                }
                fixed_count++; 
            }
        }
        if (inst->verbose >= GOOD){
            printf("Iteration %d: Fixed %d out of %d edges (%.2f%%)\n",
               iter, fixed_count, inst->nnodes, 100.0 * fixed_count / inst->nnodes);
        }

        // Set local timelimit
        CPXsetdblparam(env, CPX_PARAM_TILIM, ((residual_time > local_timelimit) ? local_timelimit : residual_time));

        // Solve with CPLEX
        int status = get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);
        if (inst->verbose >= LOW)
        {
            printf("Hard fixing iteration %d, remaining time: %4.2f seconds. --->", iter, residual_time);
            if (status)
                printf("Unable to find solution\n");
            else
                printf("Found solution\n");
        }

        // Reset edges bounds
        for (int i=0; i<inst->nnodes; i++) {

            int indeces[1] = { xpos(temp_best_sol.visited_nodes[i], temp_best_sol.visited_nodes[i+1], inst) };
            char lu[1] = {'L'};
            double bd[1] = {0.0};

            if (CPXchgbds(env, lp, 1, indeces, lu, bd)) {
                print_error("hard_fixing(): Error in setting bounds");
            }

        }

        if (status == 0) {

            build_solution_from_CPLEX(inst, &temp_sol, succ);
            update_sol(inst, &temp_best_sol, &temp_sol, true);

        }

        iter++;

        // not really improving the solution
        // percentage = percentage * 0.95; // Gradually reduce the fixing percentage
        // if (percentage < 0.5) percentage = 0.5; // Don't go below 50%

   
    }

    strncpy_s(temp_best_sol.method, METH_NAME_LEN, "HardFixing", _TRUNCATE);
    update_sol(inst, sol, &temp_best_sol, false);
    
    if (inst->verbose >= GOOD)
    {
        plot_solution(inst, &temp_best_sol);
    }

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);
    free_CPLEX(&env, &lp);
    free_solution(&temp_best_sol);
    free_solution(&temp_sol);

}