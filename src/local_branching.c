#include "local_branching.h"

void local_branching(instance *inst, solution *sol, const double timelimit) {
    double t_start = get_time_in_milliseconds();

    // Initialize temporary solutions
    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol;
    copy_sol(&temp_best_sol, sol, inst->nnodes);

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    install_callback(inst, env, lp);

    // Allocate memory for CPLEX
    int *succ = (int *)malloc(inst->nnodes * sizeof(int));
    int *comp = (int *)malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *)malloc(inst->ncols * sizeof(double));
    int ncomp;
    
    // Memory for constraints
    int *indices = (int *)malloc(inst->ncols * sizeof(int));
    double *values = (double *)malloc(inst->ncols * sizeof(double));
    
    if (succ == NULL || comp == NULL || xstar == NULL || indices == NULL || values == NULL) {
        if (indices) free(indices);
        if (values) free(values);
        if (succ) free(succ);
        if (comp) free(comp);
        if (xstar) free(xstar);
        free_CPLEX(&env, &lp);
        free_solution(&temp_best_sol);
        free_solution(&temp_sol);
        print_error("Impossible to allocate memory.");
    }
   
    int iter = 0;
    int constraint_index = -1;  // Track the local branching constraint
    
    // Neighborhood size (k parameter)
    // If param1 is set and greater than 1, use it as k; otherwise use 20% of nodes as default
    int k = (inst->param1 > 1) ? inst->param1 : (int)(0.2 * inst->nnodes);
    
    double local_timelimit = timelimit / 5;
    double residual_time;

    // Warm up the model with initial solution
    warm_up(inst, &temp_best_sol, env, lp);

    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0) {
        if (inst->verbose >= LOW) {
            printf("\n\nLocal branching iteration %d with k = %d\n", iter, k);
        }
        
        // Remove previous local branching constraint if it exists
        if (constraint_index >= 0) {
            remove_local_branching_constraint(env, lp, constraint_index);
            constraint_index = -1;
        }
        
        // Add new local branching constraint based on current best solution
        constraint_index = add_local_branching_constraint(inst, &temp_best_sol, env, lp, k, indices, values);
        
        if (inst->verbose >= GOOD) {
            printf("Iteration %d: Added local branching constraint with k = %d\n", iter, k);
        }

        // Set local timelimit
        CPXsetdblparam(env, CPX_PARAM_TILIM, ((residual_time > local_timelimit) ? local_timelimit : residual_time));

        // Solve with CPLEX
        int status = get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);
        
        if (inst->verbose >= LOW) {
            printf("Local branching iteration %d, remaining time: %4.2f seconds. --->", iter, residual_time);
            if (status)
                printf("Unable to find improved solution\n\n\n");
            else
                printf("Found new solution\n\n\n");
        }

        if (status == 0) {
            build_solution_from_CPLEX(inst, &temp_sol, succ);
            if (!update_sol(inst, &temp_best_sol, &temp_sol, true))
            {
                k = (int)(k * 1.1);
                if (k > inst->nnodes)
                {
                    k = (int)(0.5 * inst->nnodes); // Reset k if too large
                }
            }
        }
       
        iter++;
    }

    // Remove final constraint before returning
    if (constraint_index >= 0) {
        remove_local_branching_constraint(env, lp, constraint_index);
    }

    strncpy_s(temp_best_sol.method, METH_NAME_LEN, "LocalBranching", _TRUNCATE);
    update_sol(inst, sol, &temp_best_sol, false);
    
    if (inst->verbose >= GOOD) {
        plot_solution(inst, &temp_best_sol);
    }

    // Free allocated memory
    free(indices);
    free(values);
    free(xstar);
    free(comp);
    free(succ);
    free_CPLEX(&env, &lp);
    free_solution(&temp_best_sol);
    free_solution(&temp_sol);
}

int add_local_branching_constraint(const instance *inst, const solution *sol, 
                                  CPXENVptr env, CPXLPptr lp, const int k,
                                  int *indices, double *values) {
    int cnt = 0;
    char sense = 'L';
    int matbeg = 0;

    // Track which edges are used
    bool *in_solution = (bool *)calloc(inst->ncols, sizeof(bool));
    for (int i = 0; i < inst->nnodes; i++) {
        int next_idx = (i + 1) % inst->nnodes;
        int node1 = sol->visited_nodes[i];
        int node2 = sol->visited_nodes[next_idx];
        int edge_idx = xpos(node1, node2, inst);
        in_solution[edge_idx] = true;
    }

    for (int i = 0; i < inst->ncols; i++) {
        if (in_solution[i]) {
            indices[cnt] = i;
            values[cnt] = -1.0;
            cnt++;
        } else {
            indices[cnt] = i;
            values[cnt] = 1.0;
            cnt++;
        }
    }

    double rhs = k - inst->nnodes;  // Since you added -x_e for current, and +x_e for rest

    int status = CPXaddrows(env, lp, 0, 1, cnt, &rhs, &sense, &matbeg, indices, values, NULL, NULL);
    free(in_solution);

    if (status) {
        print_error("Failed to add local branching constraint");
        return -1;
    }

    return CPXgetnumrows(env, lp) - 1;
}

void remove_local_branching_constraint(CPXENVptr env, CPXLPptr lp, const int constraint_index) {
    int status = CPXdelrows(env, lp, constraint_index, constraint_index);
    
    if (status) {
        print_error("remove_local_branching_constraint(): Failed to remove constraint");
    }
}