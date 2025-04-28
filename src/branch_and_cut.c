#include "branch_and_cut.h"

void branch_and_cut(instance *inst, solution *sol, const double timelimit)
{
    
    double t_start = get_time_in_milliseconds();

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol;
    copy_sol(&temp_best_sol, sol, inst->nnodes);
    
    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    if (inst->param1 >= 1) warm_up(inst, sol, env, lp);

    install_callback(inst, env, lp);
    
    // Set up file output for statistics if needed
    FILE *f;
    if (inst->verbose >= ONLY_INCUMBMENT) {
        char filename[FILE_NAME_LEN];
        sprintf_s(filename, FILE_NAME_LEN, "results/branch_and_cut.csv");
        if(fopen_s(&f, filename, "w+")) print_error("branch_and_cut(): Cannot open file");
        fprintf(f, "time,objective,elapsed_ms\n");
    }

    // Check remaining time and set time limit
    double residual_time = timelimit - get_elapsed_time(t_start);
    if (residual_time <= 0) {
        printf("Time limit reached before solving\n");
        free_CPLEX(&env, &lp);
        free_solution(&temp_best_sol);
        free_solution(&temp_sol);
        if (f != NULL) fclose(f);
        return;
    }
    CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);
    
    // Instantiate memory
    int *succ = (int *) malloc(inst->nnodes * sizeof(int));
    int *comp = (int *) malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *) malloc(inst->ncols * sizeof(double));

    if (succ == NULL || comp == NULL || xstar == NULL) print_error("allocate_CPLEXsol(): Impossible to allocate memory.");

    int ncomp;
    
    // Solve with CPLEX
    int solstat = get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

    if (solstat == 0) {
        
        // Ensure we have a valid solution (single tour)
        if (ncomp == 1) {
            build_solution_from_CPLEX(inst, &temp_sol, succ);
            update_sol(inst, &temp_best_sol, &temp_sol, true);
            
            /*
            // Apply optional improvement with 2-opt if time remains
            residual_time = timelimit - get_elapsed_time(t_start);
            if (residual_time > 0) {
                two_opt(inst, &temp_best_sol, residual_time, false);
            }*/
        } 
        else {
            
            fprintf(stderr, "[WARNING] Final solution has %d components instead of 1\n", ncomp);
            
            // Try to patch the solution if multiple components remain
            residual_time = timelimit - get_elapsed_time(t_start);

            if (residual_time > 0) {

                // More advanced patching could be implemented here
                patch_heuristic(inst, &temp_sol, succ, comp, ncomp, residual_time);
                update_sol(inst, &temp_best_sol, &temp_sol, true);

            }

        }
        
        strncpy_s(temp_best_sol.method, METH_NAME_LEN, "BranchAndCut", _TRUNCATE);
        
        // Output final stats
        if (inst->verbose >= ONLY_INCUMBMENT && f != NULL) {
            double obj_val;
            CPXgetobjval(env, lp, &obj_val);
            fprintf(f, "final,%f,%f\n", obj_val, get_time_in_milliseconds() - t_start);
        }
        
        // Optional visualization if verbose mode is on
        if (inst->verbose >= GOOD) {
            plot_solution(inst, &temp_best_sol);
        }
        
        update_sol(inst, sol, &temp_best_sol, false);
        
        if (inst->verbose >= ONLY_INCUMBMENT) {
            plot_stats_in_file_base("branch_and_cut");
        }

    } 
    else if (solstat == CPXMIP_TIME_LIM_INFEAS) {
        // Time limit reached but no feasible solution found
        printf("Time limit reached without finding a feasible solution.\n");
        
        // Keep input solution as is (which could be a heuristic solution)
        if (inst->verbose >= DEBUG) {
            printf("Keeping original solution as no better solution was found.\n");
        }
    } 
    else {
        // Other status codes
        printf("CPLEX terminated with status %d - keeping original solution.\n", solstat);
    }

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);

    // Free and close CPLEX model
    free_CPLEX(&env, &lp);
    if (f != NULL) fclose(f);

    free_solution(&temp_best_sol);
    free_solution(&temp_sol);
}

void install_callback(instance *inst, CPXENVptr env, CPXLPptr lp) {

    // Setup callback for lazy constraints (SECs)
    CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
    if (CPXcallbacksetfunc(env, lp, contextid, lazy_callback, inst))
        print_error("CPXcallbacksetfunc() error");

}

static int CPXPUBLIC lazy_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
    double t_start = get_time_in_milliseconds();

    instance* inst = (instance*) userhandle;
    
    // Thread-local memory allocation
    double* xstar = (double*) malloc(inst->ncols * sizeof(double));
    if (xstar == NULL) {
        fprintf(stderr, "Memory allocation error in callback\n");
        return 1; // Error
    }
    
    double objval = CPX_INFBOUND;
    if (CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval)) {
        free(xstar);
        print_error("CPXcallbackgetcandidatepoint error");
    }
    
    // Get thread info for logging
    int mythread = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
    int mynode = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
    double incumbent = CPX_INFBOUND;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);

    // Log information if verbose is set
    if (inst->verbose >= GOOD) {
        if (incumbent >= CPX_INFBOUND / 2) {
            printf(" ... callback at node %5d thread %2d, NO incumbent yet, candidate value %10.2lf\n",
                   mynode, mythread, objval);
        } else {
            printf(" ... callback at node %5d thread %2d, incumbent %10.2lf, candidate value %10.2lf\n",
                   mynode, mythread, incumbent, objval);
        }
    }
    
    // Thread-local memory for subtour detection
    int nnodes = inst->nnodes;
    int* succ = (int*) malloc(nnodes * sizeof(int));
    int* comp = (int*) malloc(nnodes * sizeof(int));
    if (succ == NULL || comp == NULL) {
        free(xstar);
        if (succ != NULL) free(succ);
        if (comp != NULL) free(comp);
        fprintf(stderr, "Memory allocation error in callback\n");
        return 1; // Error
    }
    
    int ncomp = -1;
    
    // Use the build_sol_CPLEX function from tsp_cplex.h
    build_sol_CPLEX(xstar, inst, succ, comp, &ncomp);
    
    // If more than one component is found, solution has subtours 
    // - add subtour elimination constraints
    // - post patch heuristic, if asked
    if (ncomp > 1) {

        // Find all components and add SECs for all of them
        if (add_SECs_to_pool(inst, context, comp, ncomp, mynode)) {
            free(xstar);
            free(succ);
            free(comp);
            return 1;
        }

        /*
        // Find all components and add SECs for all of them
        // This is more efficient than just adding one SEC per callback
        for (int c = 0; c < ncomp; c++) {
            // Count nodes in this component
            int count = 0;
            for (int i = 0; i < nnodes; i++) {
                if (comp[i] == c+1) count++; // Components are 1-indexed
            }
            
            // Skip trivial components (single nodes)
            if (count <= 1) continue;
            
            // Skip large components (likely to be the main tour)
            if (count > nnodes/2) continue;
            
            // Allocate memory for SEC constraint
            int *index = (int*) malloc(nnodes * nnodes * sizeof(int));
            double *value = (double*) malloc(nnodes * nnodes * sizeof(double));
            if (index == NULL || value == NULL) {
                if (index != NULL) free(index);
                if (value != NULL) free(value);
                free(xstar);
                free(succ);
                free(comp);
                return 1;
            }
            
            int nnz = 0;
            double rhs = 0.0;
            
            // Use the build_SEC function from tsp_cplex.h
            build_SEC(inst, comp, c+1, index, value, &nnz, &rhs);

            // Add the SEC to the model
            if (nnz > 0)
            {
                char sense = 'L';
                int izero = 0; // Used for rmatbeg

                if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value))
                    print_error("CPXcallbackrejectcandidate() error");

                if (inst->verbose >= DEBUG)
                {
                    printf("   --> Thread %d: Added SEC for component %d with %d nodes\n",
                           mythread, c + 1, count);
                }
            }

            free(index);
            free(value);
        }*/
    
        // Post heuristic
        if (inst->param1 >= 2) {

            // Get the current time
            double time;
            CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_TIME, &time);
            
            post_heuristic(inst, context, succ, comp, ncomp, (inst->timelimit - time));

        }

    }
    
    // Free memory
    free(xstar);
    free(succ);
    free(comp);
    
    return 0;
}

int add_SECs_to_pool(const instance *inst, CPXCALLBACKCONTEXTptr context, const int *comp, const int ncomp, const int tree_node) {
    
    // Instantiate memory
    int izero = 0;
    char sense = 'L'; 
    
    int *index = (int *) malloc(inst->ncols * sizeof(int));
    double *value = (double *) malloc(inst->ncols * sizeof(double)); 

    int nnz;
    double rhs;

	if (index==NULL || value==NULL) {
        if (index != NULL) free(index);
        if (value != NULL) free(value);
        return 1;
    }

    // For each connected component add the correspondent SEC to the model
    for (int k=1; k<=ncomp; k++) {
        
        build_SEC(inst, comp, k, index, value, &nnz, &rhs);

        if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value)) 
            print_error("CPXcallbackrejectcandidate() error");

        if (inst->verbose >= DEBUG)
        {
            printf("   --> Node %d: Added SEC for component %d\n", tree_node, k);
        }
    }

	free(value);
    free(index);

    return 0;

}

int post_heuristic(const instance *inst, CPXCALLBACKCONTEXTptr context, int *succ, int *comp, int ncomp, const double timelimit) {

    double t_start = get_time_in_milliseconds();

    solution sol;
    initialize_solution(&sol);
    allocate_solution(&sol, inst->nnodes);

    patch_heuristic(inst, &sol, succ, comp, ncomp, (timelimit - get_elapsed_time(t_start)));

    build_solution_from_CPLEX(inst, &sol, succ);

    double *xheu = (double *) calloc(inst->ncols, sizeof(double));
    if (xheu == NULL) {
        free_solution(&sol);
        return 1;
    }

    build_CPLEXsol_from_solution(inst, &sol, xheu);

    int *ind = (int *) malloc(inst->ncols * sizeof(int));
    if (xheu == NULL) { 
        free(xheu);
        free_solution(&sol);
        return 1;
    }

	for ( int j = 0; j < inst->ncols; j++ ) ind[j] = j;
	if ( CPXcallbackpostheursoln(context, inst->ncols, ind, xheu, sol.cost, CPXCALLBACKSOLUTION_NOCHECK) ) print_error("CPXcallbackpostheursoln() error");
	else printf("   --> Node %d: Post heuristic solution with cost %lf\n", inst->ncols, sol.cost);
    free(ind);
    free(xheu);
    free_solution(&sol);

    return 0;

}
