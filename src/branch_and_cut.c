#include "branch_and_cut.h"

void branch_and_cut(instance *inst, solution *sol, const double timelimit)
{
    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol;
    copy_sol(&temp_best_sol, sol, inst->nnodes);
    
    double t_start = get_time_in_milliseconds();

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    double lb = -CPX_INFBOUND;
    double ub = sol->cost;
    printf("Initial upper bound: %f\n", ub);

    // Set CPLEX parameters
    CPXsetintparam(env, CPX_PARAM_CUTUP, ub);
    CPXsetintparam(env, CPX_PARAM_TILIM, timelimit - get_elapsed_time(t_start));
    CPXsetintparam(env, CPX_PARAM_THREADS, 1);
    
    // Optional: Set other CPLEX parameters for better performance
    CPXsetintparam(env, CPX_PARAM_MIPEMPHASIS, CPX_MIPEMPHASIS_BALANCED);
    CPXsetintparam(env, CPX_PARAM_PROBE, 2);
    CPXsetintparam(env, CPX_PARAM_HEURFREQ, 10);
    
    inst->ncols = CPXgetnumcols(env, lp);

    double *xstar = (double *) malloc(inst->ncols * sizeof(double));
    if (xstar == NULL) print_error("branch_and_cut(): Impossible to allocate memory for xstar.");

    // Setup callback for lazy constraints (SECs)
    CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
    if (CPXcallbacksetfunc(env, lp, contextid, my_callback, inst))
        print_error("CPXcallbacksetfunc() error");
    
    // Set up file output for statistics if needed
    FILE* f = NULL;
    if (inst->verbose >= ONLY_INCUMBMENT) {
        char filename[65];
        sprintf(filename, "results/branch_and_cut.csv");
        f = fopen(filename, "w+");
        if (f != NULL) {
            fprintf(f, "time,objective,elapsed_ms\n");
        }
    }

    // Check remaining time and set time limit
    double residual_time = timelimit - get_elapsed_time(t_start);
    if (residual_time <= 0) {
        printf("Time limit reached before solving\n");
        free(xstar);
        free_CPLEX(&env, &lp);
        free_solution(&temp_best_sol);
        free_solution(&temp_sol);
        if (f != NULL) fclose(f);
        return;
    }
    CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);
    
    // Solve with CPLEX
    if (CPXmipopt(env, lp)) 
        print_error("CPXmipopt() error");

    // Check solution status
    int solstat = CPXgetstat(env, lp);
    
    if (solstat == CPXMIP_TIME_LIM_FEAS || solstat == CPXMIP_OPTIMAL || solstat == CPXMIP_OPTIMAL_TOL) {
        // Solution found (either optimal or feasible when time limit was reached)
        if (CPXgetx(env, lp, xstar, 0, inst->ncols-1))
            print_error("CPXgetx() error");
        
        // Build solution from CPLEX results
        int *succ = (int *) malloc(inst->nnodes * sizeof(int));
        int *comp = (int *) malloc(inst->nnodes * sizeof(int));
        int ncomp = -1;
        
        if (succ == NULL || comp == NULL) print_error("branch_and_cut(): Impossible to allocate memory for succ/comp arrays.");
        
        // Build successor array and check for components
        build_sol_CPLEX(xstar, inst, succ, comp, &ncomp);
        
        // Ensure we have a valid solution (single tour)
        if (ncomp == 1) {
            build_solution_form_CPLEX(inst, &temp_sol, succ);
            update_sol(inst, &temp_best_sol, &temp_sol, true);
            
            // Apply optional improvement with 2-opt if time remains
            residual_time = timelimit - get_elapsed_time(t_start);
            if (residual_time > 0) {
                two_opt(inst, &temp_best_sol, residual_time, false);
            }
        } 
        else {
            fprintf(stderr, "[WARNING] Final solution has %d components instead of 1\n", ncomp);
            
            // Try to patch the solution if multiple components remain
            residual_time = timelimit - get_elapsed_time(t_start);
            if (residual_time > 0) {
                // More advanced patching could be implemented here
                //patch_heuristic(inst, &temp_sol, succ, comp, ncomp, residual_time);
                update_sol(inst, &temp_best_sol, &temp_sol, true);
            }
        }
        
        strcpy(temp_best_sol.method, "BranchAndCut");
        
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
        
        // Free component-related memory
        free(succ);
        free(comp);
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

    // Free and close CPLEX model
    free_CPLEX(&env, &lp);
    if (f != NULL) fclose(f);

    free_solution(&temp_best_sol);
    free_solution(&temp_sol);
}

static int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
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
    if (inst->verbose >= DEBUG) {
        if (incumbent >= CPX_INFBOUND / 2) {
            printf(" ... callback at node %5d thread %2d, NO incumbent yet, candidate value %10.2lf\n",
                   mynode, mythread, objval);
        } else {
            printf(" ... callback at node %5d thread %2d incumbent %10.2lf, candidate value %10.2lf\n",
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
    
    // If more than one component is found, solution has subtours - add subtour elimination constraints
    if (ncomp > 1) {
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
            build_SEC(inst, comp, ncomp, c+1, index, value, &nnz, &rhs);

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
        }
    }
    
    // Free memory
    free(xstar);
    free(succ);
    free(comp);
    
    return 0;
}


