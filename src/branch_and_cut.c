#include "branch_and_cut.h"

// Branch and Cut algorithm
void branch_and_cut(instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, BRANCH_AND_CUT) == 0);

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    // Warm up if asked
    if (inst->param1 == 1) {

        warm_up(inst, sol, env, lp);

    }

    install_callback(inst, env, lp);

    // Check remaining time and set time limit
    double residual_time = timelimit - get_elapsed_time(t_start);
    if (residual_time < 0) {

        if (inst->verbose >= GOOD) {

            printf("Time limit reached before solving\n");

        }

        free_CPLEX(&env, &lp);
        free_solution(&temp_sol);
        return;

    }

    // Set residual time
    CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);

    // Instantiate memory
    int *succ = (int *)malloc(inst->nnodes * sizeof(int));
    int *comp = (int *)malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *)malloc(inst->ncols * sizeof(double));

    if (succ == NULL || comp == NULL || xstar == NULL) print_error("branch_and_cut(): Cannot allocate memory");

    // Number of components of the solution
    int ncomp;

    // Solve with CPLEX
    get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

    // Ensure we have a valid solution (single tour)
    if (ncomp != 1) print_error("branch_and_cut(): The solution has more than one connected component");
    
    build_solution_from_CPLEX(inst, &temp_sol, succ);
    bool u = update_sol(inst, sol, &temp_sol, is_asked_method);
    updated = updated || u;

    if (updated) {

        strncpy_s(sol->method, METH_NAME_LEN, BRANCH_AND_CUT, _TRUNCATE);

    }
      
    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);

    CPXcallbacksetfunc(env, lp, 0, NULL, NULL);
    free_CPLEX(&env, &lp);

    free_solution(&temp_sol);

}

// Install the callback for the branching tree
void install_callback(instance *inst, CPXENVptr env, CPXLPptr lp) {

    // ID for candidate solution and fractional solution
    CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;

    // Register callback for both candidate and relaxation contexts
    if (CPXcallbacksetfunc(env, lp, contextid, callback_branch_and_cut, inst)) print_error("CPXcallbacksetfunc(): Cannot install callback");

}

// Callback for the branching tree
static int CPXPUBLIC callback_branch_and_cut(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle) {

    instance *inst = (instance *) userhandle;
	
	switch (contextid) {

	case CPX_CALLBACKCONTEXT_CANDIDATE:

        // Lazy callback always
		return lazy_callback(context, contextid, userhandle);

	case CPX_CALLBACKCONTEXT_RELAXATION:

        // Relaxation callback only if asked
        if (inst->param2 == 1) {
            
            return relaxation_callback(context, contextid, userhandle);
        
        }

        return 0;

	default:

        print_error("callback_branch_and_cut(): Callback error");

	}

}

// Lazy callback: check candidate feasibility and add SEC's
static int CPXPUBLIC lazy_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle) {
    
    instance* inst = (instance*) userhandle;
    
    // Thread-local memory allocation
    double* xstar = (double*) malloc(inst->ncols * sizeof(double));
    if (xstar == NULL) print_error("lazy_callback(): Cannot allocate mempry");
    
    double objval = CPX_INFBOUND;
    if (CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval)) print_error("CPXcallbackgetcandidatepoint(): Error");
    
    // Get thread info for logging
    int mythread = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
    int mynode = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
    double incumbent = CPX_INFBOUND;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
       
    // Log information if verbose is set
    if (inst->verbose >= GOOD) {

        printf("Node %5d, Thread %5d, ", mynode, mythread);

        if (incumbent >= CPX_INFBOUND / 2) {

            printf("NO incumbent yet");
        
        } else {

            printf("Incumbent %10.6lf", incumbent);
        
        }

        printf(", candidate value %10.6lf\n", objval);

    }
     
    // Thread-local memory for subtour detection
    int nnodes = inst->nnodes;
    int* succ = (int*) malloc(nnodes * sizeof(int));
    int* comp = (int*) malloc(nnodes * sizeof(int));
    if (succ == NULL || comp == NULL) print_error("lazy_callback(): Cannot allocate mempry");
    
    int ncomp = -1;
    
    build_sol_CPLEX(xstar, inst, succ, comp, &ncomp);
    
    // If more than one component is found, solution has subtours 
    // - add subtour elimination constraints
    // - post patch heuristic, if asked
    if (ncomp > 1) {

        // Find all components and add SECs for all of them
        add_SECs_to_pool(inst, context, comp, ncomp, mynode);

        // Post heuristic if asked
        if (inst->param3 == 1) {

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

// Relaxation callback: add fractional SEC's
static int CPXPUBLIC relaxation_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle) {

    instance* inst = (instance*) userhandle;

    // Get thread info
    int mythread = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
    int mynode = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
    int mydepth = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODEDEPTH, &mydepth);
    double incumbent = CPX_INFBOUND;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);

    // Skip some callback invocations for performance optimization
    
    // Option 1: Skip based on node depth
    if (mydepth > 10 || (mynode > 1000 && mynode % 50 != 0)) return 0;
    
    // Option 2: Skip based on node count
    // Skip most nodes, process more frequently early in search
    if ((mynode < 100 && mynode % 5 != 0) || (mynode >= 100 && mynode % 20 != 0)) return 0;

    // Get the relaxation solution
    double *xstar = (double *)calloc(inst->ncols, sizeof(double));
    if (xstar == NULL) print_error("relaxation_callback(): Cannot allocate memory");

    double objval = CPX_INFBOUND;
    
    if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1, &objval)) print_error("CPXcallbackgetrelaxationpoint(): Error");
    
    // Log information if verbose is set
    if (inst->verbose >= GOOD) {

        printf("Node %5d, Thread %5d, ", mynode, mythread);

        if (incumbent >= CPX_INFBOUND / 2) {

            printf("NO incumbent yet");
        
        } else {

            printf("Incumbent %10.6lf", incumbent);
        
        }

        printf(", candidate value %10.6lf\n", objval);

    }
    
    // Add violated cuts to the model
    add_violated_cuts_to_model(inst, context, xstar);
    
    free(xstar);

    return 0;

}

// Add the SECs in the pool
void add_SECs_to_pool(const instance *inst, CPXCALLBACKCONTEXTptr context, const int *comp, const int ncomp, const int tree_node) {
    
    // Set values for SEC's
    int izero = 0;
    char sense = 'L'; 
    
    int *index = (int *) malloc(inst->ncols * sizeof(int));
    double *value = (double *) malloc(inst->ncols * sizeof(double)); 
    if (index==NULL || value==NULL) print_error("add_SECs_to_pool(): Cannot allocate memory");

    int nnz;
    double rhs;
    int cuts_added = 0;

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

        if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value)) print_error("CPXcallbackrejectcandidate(): Error");

        cuts_added++;
        
        if (inst->verbose >= DEBUG_V) {

            printf("Node %5d, Added SEC for component %5d with %5d nodes\n", tree_node, k, count);

        }

    }
    
    if (cuts_added > 0 && inst->verbose >= GOOD) {

        printf("Node %5d, Added %5d SEC cuts\n", tree_node, cuts_added);

    }

    // Free allocated memory
    free(value);
    free(index);

}

// Post the patched solution
void post_heuristic(const instance *inst, CPXCALLBACKCONTEXTptr context, int *succ, int *comp, int ncomp, const double timelimit) {

    double t_start = get_time_in_milliseconds();

    // Get current incumbent value and node number
    double incumbent = CPX_INFBOUND;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
    int mynode = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);

    solution sol;
    initialize_solution(&sol);
    allocate_solution(&sol, inst->nnodes);

    patch_heuristic(inst, &sol, succ, comp, ncomp, (timelimit - get_elapsed_time(t_start)));

    build_solution_from_CPLEX(inst, &sol, succ);

    // Check if the solution we found is better than the incumbent
    if (incumbent < CPX_INFBOUND && sol.cost >= incumbent) {

        if (inst->verbose >= GOOD) {

            printf("   Node %5d, Incumbment %10.6lf, Heuristic solution cost %10.6lf\n", mynode, incumbent, sol.cost);
        
        }

        free_solution(&sol);

        return;

    }

    double *xheu = (double *) calloc(inst->ncols, sizeof(double));
    if (xheu == NULL) print_error("post_heuristic(): Cannot allocate memory");

    build_CPLEXsol_from_solution(inst, &sol, xheu);

    int *ind = (int *) malloc(inst->ncols * sizeof(int));
    if (ind == NULL) print_error("post_heuristic(): Cannot allocate memory");

    for (int j = 0; j < inst->ncols; j++) ind[j] = j;

    if (CPXcallbackpostheursoln(context, inst->ncols, ind, xheu, sol.cost, CPXCALLBACKSOLUTION_NOCHECK)) 
        print_error("CPXcallbackpostheursoln(): Error");
    
    if (inst->verbose >= GOOD) {

        printf(" * Node %5d, Incumbment %10.6lf, Heuristic solution cost %10.6lf\n", mynode, incumbent, sol.cost);

    }
    
    // Free allocated memory
    free(ind);
    free(xheu);
    free_solution(&sol);

}

// Add the violated SEC from a fractional solution
static int add_violated_sec(double cutval, int cutcount, int *cutlist, void *pass_param) {

    cut_callback_data *data = (cut_callback_data *) pass_param;
    const instance *inst = data->inst;
    CPXCALLBACKCONTEXTptr context = data->context;

    // Set values for SEC's
    char sense = 'L';
    int izero = 0;
    double rhs = (double) (cutcount - 1);  // SEC: sum(x_ij) <= |S| - 1

    int *index = (int *) malloc(sizeof(int) * cutcount * (cutcount - 1) / 2);
    double *coef = (double *) malloc(sizeof(double) * cutcount * (cutcount - 1) / 2);
    if (index == NULL || coef == NULL) print_error("add_violated_sec(): Cannot allocate memory");

    int nnz = 0;

    // Compute the SEC
    for (int i=0; i<cutcount; i++) {

        for (int j=i+1; j<cutcount; j++) {

            int pos = xpos(cutlist[i], cutlist[j], inst);
            index[nnz] = pos;
            coef[nnz] = 1.0;
            nnz++;

        }

    }

    if (nnz > 0) {

        int purgeable = CPX_USECUT_FILTER; // Let CPLEX decide which cuts to keep
        int local = 0; // Global cut (valid for the entire problem)

        if (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, index, coef, &purgeable, &local)) 
            print_error("CPXcallbackaddusercuts(): Error");

    }

    // Free allocated memory
    free(index);
    free(coef);

    return 0;

}

// Extract the SEC's violated by a fractional solution and add them to the user's cuts
void add_violated_cuts_to_model(const instance *inst, CPXCALLBACKCONTEXTptr context, double *xstar) {

    // Maximum number of edges
    int max_edges = inst->ncols;

    int *elist = (int *)calloc(2 * max_edges, sizeof(int));
    double *elist_val = (double *)calloc(max_edges, sizeof(double));
    if (elist == NULL || elist_val == NULL) print_error("add_violated_cuts_to_model(): Cannot allocate memory");

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

        }

    }

    int ncomp = 0;
    int *comps = NULL;
    int *compscount = NULL;

    if (CCcut_connect_components(inst->nnodes, nedges, elist, elist_val, &ncomp, &compscount, &comps)) 
        print_error("CCcut_connect_components(): Error");

    if (inst->verbose >= GOOD) {

        printf("Relaxation callback: Found %5d components\n", ncomp);

    }

    if (ncomp == 1) {

        // Fully connected: find violated cuts
        cut_callback_data data = { context, inst };

        if (CCcut_violated_cuts(inst->nnodes, nedges, elist, elist_val, 2.0 - 1e-5, add_violated_sec, &data)) 
            print_error("CCcut_violated_cuts(): Error");

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
            if (nodes_in_comp == NULL) print_error("add_violated_cuts_to_model(): Cannot allocate memory");
            
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
            if (index == NULL || coef == NULL) print_error("add_violated_cuts_to_model(): Cannot allocate memory");

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

                printf("Relaxation callback: Adding SEC for component %5d with %5d nodes\n", c + 1, compscount[c]);

            }

            int purgeable = CPX_USECUT_FILTER; // Let CPLEX decide which cuts to keep
            int local = 0; // Global cut (valid for the entire problem)

            if (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, index, coef, &purgeable, &local)) 
                print_error("CPXcallbackaddusercuts(): Error in multi-component SEC");

            // Free allocated memory
            free(index);
            free(coef);
            free(nodes_in_comp);

        }

    }

    // Free allocated memory
    free(elist);
    free(elist_val);
    free(comps);
    free(compscount);

    return 0;

}
