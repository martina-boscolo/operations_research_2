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

    inst->ncols = CPXgetnumcols(env, lp);
    printf("Number of columns: %d\n", inst->ncols);

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
        if (f == NULL) print_error("Could not open file for writing statistics");
        fprintf(f, "iteration,objective,time_ms\n");
    }

    // Set time limit
    double residual_time = timelimit - get_elapsed_time(t_start);
    if (residual_time <= 0) {
        printf("Time limit reached before solving\n");
        printf("Work in progress, this need to be managed better\n");
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

    // Get the solution
    if (CPXgetx(env, lp, xstar, 0, inst->ncols-1))
        print_error("CPXgetx() error");
    
    // Output final stats
    if (inst->verbose >= ONLY_INCUMBMENT && f != NULL) {
        double obj_val;
        CPXgetobjval(env, lp, &obj_val);
        fprintf(f, "final,%f,%f\n", obj_val, get_time_in_milliseconds() - t_start);
    }
    
    // Build solution from CPLEX results using the function similar to Benders
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
            patch_heuristic(inst, &temp_sol, succ, comp, ncomp, residual_time);
            update_sol(inst, &temp_best_sol, &temp_sol, true);
        }
    }
    
    sprintf(temp_best_sol.method, "BranchAndCut");
    
    // Optional visualization if verbose mode is on
    if (inst->verbose >= GOOD) {
        plot_solution(inst, &temp_best_sol);
    }
    
    update_sol(inst, sol, &temp_best_sol, false);

    // Free allocated memory
    free(xstar);
    free(succ);
    free(comp);

    // Free and close CPLEX model
    free_CPLEX(&env, &lp);
    if (f != NULL) fclose(f);

    free_solution(&temp_best_sol);
    free_solution(&temp_sol);

    if (inst->verbose >= ONLY_INCUMBMENT) {
        plot_stats_in_file_base("branch_and_cut");
    }
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

    if (incumbent >= CPX_INFBOUND / 2) {
        // No incumbent yet
        if (inst->verbose >= DEBUG) {
            printf(" ... callback at node %5d thread %2d, NO incumbent yet, candidate value %10.2lf\n",
                   mynode, mythread, objval);
        }
    } else {
        if (inst->verbose >= DEBUG) {
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
    
    // Build successor array from xstar (using the same approach as in build_sol_CPLEX)
    for (int i = 0; i < nnodes; i++) succ[i] = -1;
    
    for (int i = 0; i < nnodes; i++) {
        for (int j = 0; j < nnodes; j++) {
            if (i == j) continue;
            if (xstar[xpos(i, j, inst)] > 0.5) {
                succ[i] = j;
                break;
            }
        }
    }
    
    // Find connected components
    for (int i = 0; i < nnodes; i++) comp[i] = -1;
    ncomp = 0;
    
    for (int start = 0; start < nnodes; start++) {
        if (comp[start] >= 0) continue;  // already visited
        comp[start] = ncomp;
        int i = start;
        while (succ[i] != start && succ[i] >= 0 && comp[succ[i]] < 0) {
            i = succ[i];
            comp[i] = ncomp;
        }
        if (succ[i] == start) ncomp++;  // found a cycle
    }
    
    // Prepare for adding SECs if needed
    int* index = NULL;
    double* value = NULL;
    int nnz = 0;
    double rhs = 0.0;
    char sense = 'L';  // <= constraint
    
    // If more than one component is found, solution has subtours - add subtour elimination constraint
    if (ncomp > 1) {
        // Find smallest component
        int mincomp = 0;
        int mincount = nnodes + 1;
        for (int c = 0; c < ncomp; c++) {
            int count = 0;
            for (int i = 0; i < nnodes; i++) {
                if (comp[i] == c) count++;
            }
            if (count < mincount && count > 1) {  // Ensure component has at least 2 nodes
                mincount = count;
                mincomp = c;
            }
        }
        
        // Allocate memory for SEC constraint
        int *index = (int*) malloc(nnodes * nnodes * sizeof(int));
        double *value = (double*) malloc(nnodes * nnodes * sizeof(double));
        int nnz = 0;
        double rhs = 0.0;
        
        if (index == NULL || value == NULL) {
            // Handle memory allocation failure
            if (index != NULL) free(index);
            if (value != NULL) free(value);
            free(xstar);
            free(succ);
            free(comp);
            return 1;
        }
        
        // Use your existing build_SEC function without env, lp parameters
        build_SEC(inst, comp, ncomp, mincomp, index, value, &nnz, &rhs);
        
        // Add the SEC to the model
        if (nnz > 0) {
            int izero = 0;
            char sense = 'L';
            if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value))
                print_error("CPXcallbackrejectcandidate() error");
                
            if (inst->verbose >= GOOD) {
                printf("   --> Thread %d: Added SEC for component %d with %d nodes\n", 
                       mythread, mincomp, mincount);
            }
        }
        
        free(index);
        free(value);
    }
    
    // Free thread-local memory
    free(succ);
    free(comp);
    free(xstar);
    if (index != NULL) free(index);
    if (value != NULL) free(value);
    
    return 0;
}

