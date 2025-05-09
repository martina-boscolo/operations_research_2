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

    inst->param1 = 1;
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
    double p = 0.8;

    double local_timelimit = timelimit/10;
    double residual_time;

    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0) {

        warm_up(inst, &temp_best_sol, env, lp);

        for (int i=0; i<inst->nnodes; i++) {

            if (random01() < p) {

                int indeces[1] = { xpos(temp_best_sol.visited_nodes[i], temp_best_sol.visited_nodes[i+1], inst) };
                char lu[1] = {'L'};
                double bd[1] = {1.0};

                if (CPXchgbds(env, lp, 1, indeces, lu, bd)) {
                    print_error("hard_fixing(): Error in setting bounds");
                }

            }

        }

        // Set local timelimit
        CPXsetdblparam(env, CPX_PARAM_TILIM, ((residual_time > local_timelimit) ? local_timelimit : residual_time));

        // Solve with CPLEX
        int status = get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);
        if(status) {
            if (inst->verbose >= LOW) {
                printf("Unable to find optimal solution\n");
            }
        } else {
            if (inst->verbose >= LOW) {
                printf("Found optimal solution at iter %d\n", iter);
            }
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

    }

    strncpy_s(temp_best_sol.method, METH_NAME_LEN, "HardFixing", _TRUNCATE);
    update_sol(inst, sol, &temp_best_sol, false);

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);
    free_CPLEX(&env, &lp);
    free_solution(&temp_best_sol);
    free_solution(&temp_sol);

}