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

    if (inst->param1 >= 1)
        warm_up(inst, sol, env, lp);

    install_callback(inst, env, lp);

    // Check remaining time and set time limit
    double residual_time = timelimit - get_elapsed_time(t_start);
    if (residual_time <= 0)
    {
        printf("Time limit reached before solving\n");
        free_CPLEX(&env, &lp);
        free_solution(&temp_best_sol);
        free_solution(&temp_sol);
        return;
    }
    CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);

    // Instantiate memory
    int *succ = (int *)malloc(inst->nnodes * sizeof(int));
    int *comp = (int *)malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *)malloc(inst->ncols * sizeof(double));

    if (succ == NULL || comp == NULL || xstar == NULL)
        print_error("allocate_CPLEXsol(): Impossible to allocate memory.");

    int ncomp;

    // Solve with CPLEX
    int solstat = get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

    if (solstat == 0)
    {

        // Ensure we have a valid solution (single tour)
        if (ncomp == 1)
        {
            build_solution_from_CPLEX(inst, &temp_sol, succ);
            update_sol(inst, &temp_best_sol, &temp_sol, true);
        }
        else
        {
            fprintf(stderr, "[WARNING] Final solution has %d components instead of 1\n", ncomp);  
        }

        strncpy(temp_best_sol.method, "BranchAndCut", METH_NAME_LEN - 1);
        temp_best_sol.method[METH_NAME_LEN - 1] = '\0'; // Ensure null-termination

        // Optional visualization if verbose mode is on
        if (inst->verbose >= GOOD)
        {
            plot_solution(inst, &temp_best_sol);
        }
        
        update_sol(inst, sol, &temp_best_sol, false);
    
    }
    else if (solstat == CPXMIP_TIME_LIM_INFEAS)
    {
        // Time limit reached but no feasible solution found
        printf("Time limit reached without finding a feasible solution.\n");

        // Keep input solution as is (which could be a heuristic solution)
        if (inst->verbose >= DEBUG_V)
        {
            printf("Keeping original solution as no better solution was found.\n");
        }
    }
    else
    {
        // Other status codes
        printf("CPLEX terminated with status %d - keeping original solution.\n", solstat);
    }

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);

    // Free and close CPLEX model
    int status = CPXcallbacksetfunc(env, lp, 0, NULL, NULL);
    if (status)
    {
        printf("Warning: Failed to unregister callbacks, status %d\n", status);
    }
    free_CPLEX(&env, &lp);

    //THIS IS A MEMORY LEAK, DO NOT FREE THE SOLUTION HERE
    //  free_solution(&temp_best_sol);
    //  free_solution(&temp_sol);

}

static int CPXPUBLIC callback_branch_and_cut(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
    instance *inst = (instance *) userhandle;
	
	switch (contextid)
	{
	case CPX_CALLBACKCONTEXT_CANDIDATE:
		return lazy_callback(context, contextid, userhandle);
	case CPX_CALLBACKCONTEXT_RELAXATION:
        if (inst->param2 == 1) 
		    return relaxation_callback(context, contextid, userhandle);
        return 0;
	default:
        print_error("Callback error");
		return 1;
	}
}

void install_callback(instance *inst, CPXENVptr env, CPXLPptr lp) {
    CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
    // Register callback for both candidate and relaxation contexts
    if (CPXcallbacksetfunc(env, lp, contextid, callback_branch_and_cut, inst))
        print_error("CPXcallbacksetfunc() error for callback");
}

static int CPXPUBLIC lazy_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
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

        // Post heuristic
        if (inst->param1 >= 2) {

            // Get the current time
            double time = get_elapsed_time(inst->t_start);
            
            post_heuristic(inst, context, succ, comp, ncomp, (inst->timelimit - time));

        }

    }
    
    // Free memory
    free(xstar);
    free(succ);
    free(comp);
    
    return 0;
}

// In your add_SECs_to_pool function:

int add_SECs_to_pool(const instance *inst, CPXCALLBACKCONTEXTptr context, const int *comp, const int ncomp, const int tree_node) {
    
    // Instantiate memory
    int izero = 0;
    char sense = 'L'; 
    
    int *index = (int *) malloc(inst->ncols * sizeof(int));
    double *value = (double *) malloc(inst->ncols * sizeof(double)); 

    int nnz;
    double rhs;
    int cuts_added = 0;

    if (index==NULL || value==NULL) {
        if (index != NULL) free(index);
        if (value != NULL) free(value);
        return 1;
    }

    // For each connected component add the correspondent SEC to the model
    for (int k=1; k<=ncomp; k++) {
        // Count nodes in this component
        int count = 0;
        for (int i = 0; i < inst->nnodes; i++) {
            if (comp[i] == k) count++;
        }
        
        // Skip components that are too large or too small
        if (count <= 1 || count >= inst->nnodes - 1) continue;
        
        build_SEC(inst, comp, k, index, value, &nnz, &rhs);

        if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value)) 
            print_error("CPXcallbackrejectcandidate() error");

        cuts_added++;
        
        if (inst->verbose >= DEBUG_V) {
            printf("   --> Node %d: Added SEC for component %d with %d nodes\n", 
                   tree_node, k, count);
        }
    }

    free(value);
    free(index);
    
    if (cuts_added > 0 && inst->verbose >= GOOD) {
        printf("   --> Node %d: Added %d SEC cuts\n", tree_node, cuts_added);
    }

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
    if (ind == NULL) { 
        free(xheu);
        free_solution(&sol);
        return 1;
    }

	for ( int j = 0; j < inst->ncols; j++ ) ind[j] = j;
	if ( CPXcallbackpostheursoln(context, inst->ncols, ind, xheu, sol.cost, CPXCALLBACKSOLUTION_NOCHECK) ) print_error("CPXcallbackpostheursoln() error");
	
    if (inst->verbose >= GOOD) {
        int mynode = -1;
        CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
        printf("   --> Node %d: Post heuristic solution with cost %lf\n", mynode, sol.cost);
    }
    
    free(ind);
    free(xheu);
    free_solution(&sol);

    return 0;

}


static int CPXPUBLIC relaxation_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
    instance* inst = (instance*) userhandle;

    // Performance optimization - skip some callback invocations
    int depth = -1;
    int nodes = -1;
    int threadid = -1;
    
    // Get thread info
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &threadid);
    
    // Option 1: Skip based on node depth
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODEDEPTH, &depth);
    if (depth > 10 || (nodes > 1000 && nodes % 50 != 0)) {
        return 0;  // Skip this callback invocation
    }
    
    // Option 2: Skip based on node count
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &nodes);
    if ((nodes < 100 && nodes % 5 != 0) || (nodes >= 100 && nodes % 20 != 0)) {
        // Skip most nodes, process more frequently early in search
        return 0;
    }
    
    // Get the relaxation solution
    double *xstar = (double *)calloc(inst->ncols, sizeof(double));
    if (xstar == NULL)
        return 1;

    double objval = CPX_INFBOUND;
    
    if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1, &objval)) {
        free(xstar);
        print_error("CPXcallbackgetrelaxationpoint error");
    }
    
    if (inst->verbose >= GOOD) {
        printf("Relaxation callback: Relaxation solution value: %lf\n", objval);
    }
    
    // Add violated cuts to the model
    int result = add_violated_cuts_to_model(inst, context, xstar);
    
    free(xstar);
    return result;
}

typedef struct {
    CPXCALLBACKCONTEXTptr context;
    const instance *inst;
} cut_callback_data;

static int add_violated_sec(double cutval, int cutcount, int *cutlist, void *pass_param) 
{
    cut_callback_data *data = (cut_callback_data *) pass_param;
    const instance *inst = data->inst;
    CPXCALLBACKCONTEXTptr context = data->context;

    char sense = 'L';
    int izero = 0;
    double rhs = (double)(cutcount - 1);  // SEC: sum(x_ij) <= |S| - 1

    int *index = (int *) malloc(sizeof(int) * cutcount * (cutcount - 1) / 2);
    double *coef = (double *) malloc(sizeof(double) * cutcount * (cutcount - 1) / 2);
    if (index == NULL || coef == NULL) {
        if (index != NULL) free(index);
        if (coef != NULL) free(coef);
        return 1;
    }

    int nnz = 0;
    for (int ii = 0; ii < cutcount; ii++) {
        for (int jj = ii + 1; jj < cutcount; jj++) {
            int pos = xpos(cutlist[ii], cutlist[jj], inst);
            index[nnz] = pos;
            coef[nnz] = 1.0;
            nnz++;
        }
    }

    if (nnz > 0) {
        int purgeable = CPX_USECUT_FILTER; // Let CPLEX decide which cuts to keep
        int local = 0; // Global cut (valid for the entire problem)

        if (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, index, coef, &purgeable, &local))
            print_error("CPXcallbackaddusercuts() error in add_violated_sec");
    }

    free(index);
    free(coef);

    return 0;
}

int add_violated_cuts_to_model(const instance *inst, CPXCALLBACKCONTEXTptr context, double *xstar)
{
    // Maximum number of edges in a complete graph is n*(n-1)/2
    int max_edges = (inst->nnodes * (inst->nnodes - 1)) / 2;
    int *elist = (int *)calloc(2 * max_edges, sizeof(int));
    double *elist_val = (double *)calloc(max_edges, sizeof(double));
    if (elist == NULL || elist_val == NULL) {
        if (elist) free(elist);
        if (elist_val) free(elist_val);
        return 1;
    }

    int nedges = 0;
    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = i + 1; j < inst->nnodes; j++) {
            int index = xpos(i, j, inst);  // Use the correct function
            // Add only significantly violated cuts
            if (xstar[index] > 0.01) {  // Only consider non-zero edges
                // Add to edge list for cut generation
                elist[2 * nedges] = i;
                elist[2 * nedges + 1] = j;
                elist_val[nedges] = xstar[index];
               
                nedges++;
            }
            if (nedges >= max_edges) {
                fprintf(stderr, "Warning: Edge buffer is full\n");
                break;  // Break out of the inner loop
            }
        }
    }
    int ncomp = 0;
    int *comps = NULL;
    int *compscount = NULL;

    if (CCcut_connect_components(inst->nnodes, nedges, elist, elist_val, &ncomp, &compscount, &comps)) {
        fprintf(stderr, "Error in CCcut_connect_components\n");
        free(elist);
        free(elist_val);
        return 1;
    }
    if (inst->verbose >= GOOD) {
        printf("Relaxation callback: Found %d components\n", ncomp);
    }

    if (ncomp == 1) {
        // Fully connected: find violated cuts
        cut_callback_data data = { context, inst };

        if (CCcut_violated_cuts(inst->nnodes, nedges, elist, elist_val, 2.0 - 1e-5, add_violated_sec, &data)) {
            fprintf(stderr, "Error in CCcut_violated_cuts\n");
            free(elist);
            free(elist_val);
            free(comps);
            free(compscount);
            return 1;
        }
    } else {
        // Multiple components, add SECs
        int comps_offset = 0;
        for (int c = 0; c < ncomp; c++) {
            // Skip small components
            if (compscount[c] <= 1) {
                comps_offset += compscount[c];
                continue;
            }

            int *nodes_in_comp = (int *) malloc(compscount[c] * sizeof(int));
            
            // Correctly copy nodes from this component
            for (int j = 0; j < compscount[c]; j++) {
                nodes_in_comp[j] = comps[comps_offset + j];
            }
            comps_offset += compscount[c];

            // Build SEC manually
            char sense = 'L';
            int izero = 0;
            double rhs = compscount[c] - 1;
            int nnz = compscount[c] * (compscount[c] - 1) / 2;

            int *index = (int *) malloc(nnz * sizeof(int));
            double *coef = (double *) malloc(nnz * sizeof(double));
            if (index == NULL || coef == NULL) {
                if (index) free(index);
                if (coef) free(coef);
                free(nodes_in_comp);
                free(elist);
                free(elist_val);
                free(comps);
                free(compscount);
                return 1;
            }

            int cnt = 0;
            for (int i = 0; i < compscount[c]; i++) {
                for (int j = i + 1; j < compscount[c]; j++) {
                    int pos = xpos(nodes_in_comp[i], nodes_in_comp[j], inst);
                    index[cnt] = pos;
                    coef[cnt] = 1.0;
                    cnt++;
                }
            }
            if (inst->verbose >= DEBUG_V) {
                printf("Relaxation callback: Adding SEC for component %d with %d nodes\n", c + 1, compscount[c]);
            }
            int purgeable = CPX_USECUT_FILTER; // Let CPLEX decide which cuts to keep
            int local = 0; // Global cut (valid for the entire problem)

            if (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, index, coef, &purgeable, &local))
                print_error("CPXcallbackaddusercuts() error in multi-component SEC");

            free(index);
            free(coef);
            free(nodes_in_comp);
        }
    }

    free(elist);
    free(elist_val);
    free(comps);
    free(compscount);

    return 0;
}

