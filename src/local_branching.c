#include "local_branching.h"

// Local branching algorithm
void local_branching(instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, LOCAL_BRANCHING) == 0);

    // Initialize temporary solutions
    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    install_callback(inst, env, lp);

    // Allocate memory for CPLEX solution
    int *succ = (int *)malloc(inst->nnodes * sizeof(int));
    int *comp = (int *)malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *)malloc(inst->ncols * sizeof(double));
    int ncomp;

    if (succ == NULL || comp == NULL || xstar == NULL) print_error("local_branching(): Cannot allocate memory");
   
    int iter = 0;
    
    // Neighborhood size (k parameter)
    // If param1 is set and greater than 1, use it as k; otherwise use 20% of nodes as default
    int default_k = (inst->param1 > 1) ? inst->param1 : (int)(0.2 * inst->nnodes);
    int k = default_k;
   
    double residual_time;

    // Warm up the model with initial solution
    warm_up(inst, sol, env, lp);

    while ((residual_time = timelimit - get_elapsed_time(t_start)) > 0) {

        // Add new local branching constraint based on current best solution
        add_local_branching_constraint(inst, sol, env, lp, k);

        // Set local timelimit
        CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);

        // Solve with CPLEX
        get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

        // Remove local branching constraint
        remove_local_branching_constraint(env, lp);

        build_solution_from_CPLEX(inst, &temp_sol, succ);

        double old_cost = sol->cost;
        bool u = update_sol(inst, sol, &temp_sol, false);
        updated = updated || u;
        
        if (inst->verbose >= LOW) {

            if (u) {

                printf(" * ");

            } else {

                printf("   ");

            }

            printf("Iteration %5d, Incumbment %10.6lf, Heuristic solution cost %10.6lf, k %5d, Residual time %10.6lf\n", 
                iter, old_cost, temp_sol.cost, k, residual_time);

        }
        
        // If no improvements change the number of fixed edges
        if (!u) { 

            k = (int)(k * 1.1);

            if (k > inst->nnodes) {

                k = (int)(0.5 * inst->nnodes); // Reset k if too large

            }

        } else { // otherwise reset it

            k = default_k;

        }
        
        iter++;

    }

    if (updated) {

        strncpy_s(sol->method, METH_NAME_LEN, LOCAL_BRANCHING, _TRUNCATE);

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

// Set the local branching constraint in the CPLEX model
void add_local_branching_constraint(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp, const int k) {
       
    // Set values for model constraints
    int nnz = 0;
    char sense = 'G';
    int izero = 0;
 
    // Memory for constraints
    int *indices = (int *)malloc(inst->ncols * sizeof(int));
    double *values = (double *)malloc(inst->ncols * sizeof(double));
    
    // Track which edges in the solution
    bool *in_solution = (bool *)calloc(inst->ncols, sizeof(bool));
    
    if (indices == NULL || values == NULL || in_solution == NULL) print_error("add_local_branching_constraint(): Cannot allocate memory");

    for (int i=0; i<inst->nnodes; i++) {

        int node1 = sol->visited_nodes[i];
        int node2 = sol->visited_nodes[i+1];

        int edge_idx = xpos(node1, node2, inst);

        in_solution[edge_idx] = true;

    }

    for (int i = 0; i < inst->ncols; i++) {

        // The constraint cons
        if (in_solution[i]) {

            indices[nnz] = i;
            values[nnz] = 1.0;
            nnz++;

        }

    }

    double rhs = inst->nnodes - k;

    if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, indices, values, NULL, NULL)) 
        print_error("add_local_branching_constraint(): Failed to add local branching constraint");

    // Free allocated memory
    free(in_solution);
    free(values);
    free(indices);

}

// Remove the local branching constraint in the CPLEX model
void remove_local_branching_constraint(CPXENVptr env, CPXLPptr lp) {
    
    // The local branching constraint is the last constraint added
    int last_row_index = CPXgetnumrows(env, lp) - 1;

    if (CPXdelrows(env, lp, last_row_index, last_row_index)) print_error("remove_local_branching_constraint(): Failed to remove constraint");
    
}