#include "tsp_cplex.h"

// Initialize the CPLEX enviroment and build the base model of TSP
void initialize_CPLEX(instance *inst, CPXENVptr *env, CPXLPptr *lp) {

    // Open CPLEX model
    int error;
    *env = CPXopenCPLEX(&error);
    if (*env == NULL) print_error("initialize_CPLEX(): Failed to create CPLEX environment");

    // Create problem object
    *lp = CPXcreateprob(*env, &error, "TSP_Problem");
    if (*lp == NULL) print_error("initialize_CPLEX(): Failed to create CPLEX problem object");

    // Set CPLEX parameters for better performance
    CPXsetdblparam(*env, CPX_PARAM_EPGAP, 1e-9);
    CPXsetintparam(*env, CPX_PARAM_MIPEMPHASIS, CPX_MIPEMPHASIS_BALANCED);
    CPXsetintparam(*env, CPX_PARAM_PROBE, 2);
    CPXsetintparam(*env, CPX_PARAM_HEURFREQ, 10);
    
    if(inst->verbose >= GOOD) {
        
        CPXsetintparam(*env, CPX_PARAM_SCRIND, CPX_ON);

    }

    // Build the model
    build_model_CPLEX(inst, *env, *lp);

}

// Use the solution as warm up for the branching tree
void warm_up(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp) {

    if (inst->verbose >= GOOD) {

        printf("Warmup with cost %10.6lf\n", sol->cost);

    }

    // Change the solution format: from solution struct to CPLEX format

    double *xheu = (double *) calloc(inst->ncols, sizeof(double));
    int *ind = (int *) malloc(inst->ncols * sizeof(int));

    if (xheu == NULL || ind == NULL) print_error("warm_up(): Cannot allocate memory");

    build_CPLEXsol_from_solution(inst, sol, xheu);

	for ( int j = 0; j < inst->ncols; j++ ) ind[j] = j;
	int effortlevel = CPX_MIPSTART_NOCHECK;  
	int beg = 0;

    // Warm up
	if (CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, ind, xheu, &effortlevel, NULL)) print_error("CPXaddmipstarts(): Error");

	free(ind);
    free(xheu);

}

// Obtain the optimal solution
int get_optimal_solution_CPLEX(const instance *inst, CPXENVptr env, CPXLPptr lp, double *xstar, int *succ, int *comp, int *ncomp) {
    
	// Solve the model
	if (CPXmipopt(env, lp)) print_error("CPXmipopt(): Error");

    int status = CPXgetstat(env, lp);

    if (inst->verbose >= DEBUG_V) {

        printf("status = %5d\n", status);

    }
    
    // If feasible solution was found continue
    if (!has_feasible_solution(status)) print_error("CPXmipopt(): Strange optimality status");

    // Retrieve the optimal solution
    int ncols = CPXgetnumcols(env, lp);
    if (CPXgetx(env, lp, xstar, 0, ncols - 1)) print_error("CPXgetx(): Error");

    // Build the solution
    build_sol_CPLEX(xstar, inst, succ, comp, ncomp);

    // If asked print the solution components
    if (inst->verbose >= DEBUG_V) {

        printf("Number of components: %5d\n", *ncomp);

        for (int i = 0; i < inst->nnodes; i++) {

            printf("Node %5d -> Successor: %5d, Component: %5d\n", i + 1, succ[i] + 1, comp[i]);

        }

    }

    return status;

}

// Return if CPLEX found a feasible solution
bool has_feasible_solution(const int status) {

    return (status == CPXMIP_OPTIMAL || status == CPXMIP_OPTIMAL_TOL ||
            status == CPXMIP_TIME_LIM_FEAS || status == CPXMIP_NODE_LIM_FEAS ||
            status == CPXMIP_ABORT_FEAS || status == CPXMIP_MEM_LIM_FEAS);

}

// Build the SEC correspondent to the given connected component
void build_SEC(const instance *inst, const int *comp, const int sec_comp, int *index, double *value, int *nnz, double *rhs) {

    *nnz = 0;
    *rhs = -1;

    // For every node in the connected component 
    for (int i=0; i<inst->nnodes; i++) {

        if (comp[i] != sec_comp) continue;
		if (inst->verbose >= DEBUG_V) printf("comp[%5d] = %5d\n", i+1, comp[i]);

        // - increment right hand side (to compute the number of nodes in the connected component)
        (*rhs)++;

        // - add each edge that connect the node with all the other nodes in the component to the SEC
        // NOTE: start from i+1 since the possible edges with nodes from 0 to i-1 have been already considered
        for (int j=i+1; j<inst->nnodes; j++) {

            if (comp[j] != sec_comp) continue;

			if (inst->verbose >= DEBUG_V) printf("Add edge %5d %5d to SEC %5d\n", i+1, j+1, sec_comp);

			index[*nnz] = xpos(i, j, inst);
			value[*nnz] = 1.0;
			(*nnz)++;

        }

    }

}

// Compute the index of edge between two nodes in the CPLEX model
int xpos(const int i, const int j, const instance *inst) { 

    if (i == j) print_error("xpos(): i == j");

    if (i > j) return xpos(j,i,inst);

    return i * inst->nnodes + j - ((i + 1) * (i + 2)) / 2;

}

// Build the ILP model
void build_model_CPLEX(instance *inst, CPXENVptr env, CPXLPptr lp) {    

    // Set values for model constraints
    int izero = 0;
    char binary = 'B'; 
    
    char **cname = (char **) calloc(1, sizeof(char*));
    if (cname == NULL) print_error("build_model_CPLEX(): Cannot allocate memory");
    cname[0] = (char *) calloc(CONS_NAME_LEN, sizeof(char));
    if (cname[0] == NULL) print_error("build_model_CPLEX(): Cannot allocate memory");

    // Add binary variable x(i,j) for each i < j and create the objective function  
    for (int i=0; i<inst->nnodes; i++) {

        for (int j=i+1; j<inst->nnodes; j++) {

            sprintf_s(cname[0], CONS_NAME_LEN, "x(%5d,%5d)", i+1,j+1);
            double obj = cost(i,j,inst); // objective coefficient   
            double lb = 0.0; // lower bound
            double ub = 1.0; // upper bound
            if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) print_error("CPXnewcols(): Cannot add variable");
            if (CPXgetnumcols(env,lp)-1 != xpos(i,j, inst)) print_error("build_model_CPLEX(): Wrong number of columns in the model");
        
        }
    
    } 

    // Add degree constraints 
    int *index = (int *) malloc(inst->nnodes * sizeof(int));
    double *value = (double *) malloc(inst->nnodes * sizeof(double));  
    if (index == NULL || value == NULL) print_error("build_model_CPLEX(): Cannot allocate memory");
    
    for ( int h = 0; h < inst->nnodes; h++ ) {

        double rhs = 2.0; // Each node must have 2 neighbors
        char sense = 'E'; // Equality constraint 

        sprintf_s(cname[0], CONS_NAME_LEN, "degree(%%d)", h+1); 

        int nnz = 0;
        for (int i=0; i<inst->nnodes; i++) {

            if (i == h) continue;

            index[nnz] = xpos(i,h, inst);
            value[nnz] = 1.0;
            nnz++;

        }
        
        if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0])) print_error("CPXaddrows(): Cannot add degree constraint");
    
    } 

    free(value);
    free(index);

    // If asked store the model in a file
    if (inst->verbose >= GOOD) CPXwriteprob(env, lp, "model.lp", NULL);   

    free(cname[0]);
    free(cname);

    inst->ncols = CPXgetnumcols(env, lp);

}

// Build the solution based on the edges selected in xstar
void build_sol_CPLEX(const double *xstar, const instance *inst, int *succ, int *comp, int *ncomp) {   

    // If asked check CPLEX solution
    if (inst->verbose >= DEBUG_V) {

        int *degree = (int *) calloc(inst->nnodes, sizeof(int));

        for (int i=0; i<inst->nnodes; i++) {

            for (int j=i+1; j<inst->nnodes; j++) {

                int k = xpos(i,j,inst);
                if (fabs(xstar[k]) > EPSILON && fabs(xstar[k]-1.0) > EPSILON) print_error("build_sol_CPLEX(): Wrong xstar");
                if (xstar[k] > 0.5) {

                    ++degree[i];
                    ++degree[j];

                }

            }

        }

        for (int i=0; i<inst->nnodes; i++) {

            if (degree[i] != 2) print_error("build_sol_CPLEX(): Wrong degree");

        }    

        free(degree);
    
    }

    // Initialize arrays
    *ncomp = 0;
    for (int i=0; i<inst->nnodes; i++) {

        succ[i] = -1;
        comp[i] = -1;

    }
    
    // For every connected component
    for (int start=0; start<inst->nnodes; start++) {

        if (comp[start] >= 0) continue;  // node "start" was already visited, just skip it

        // A new connected component is found
        (*ncomp)++;

        int i = start;
        bool done = false;

        // Update array for every node in the connected component
        while (!done) {

            comp[i] = *ncomp;
            done = true;

            // Search for the successor of node i
            for (int j=0; j<inst->nnodes; j++) {

                // If the edge [i,j] is selected in xstar and j was not visited before 
                if (i != j && xstar[xpos(i,j,inst)] > 0.5 && comp[j] == -1) {

                    succ[i] = j;
                    i = j;
                    done = false;
                    break;

                }

            }

        }   

        succ[i] = start;  // Last arc to close the cycle
        
        // Go to the next component...

    }

}

// Build a solution struct from CPLEX solution
void build_solution_from_CPLEX(const instance *inst, solution *sol, const int *succ) {
    
    // In array succ the successor of node i is in succ[i]
    // In array visited_nodes the successor of node visited_nodes[i] is in visited_nodes[i+1]

    // Compute visited_nodes
    sol->visited_nodes[0] = 0;
    int s = succ[0];

    for (int i = 1; i <= inst->nnodes+1; i++) {

        sol->visited_nodes[i] = s;
        s = succ[s];

    }

    // Compute the cost of the solution
    sol->cost = compute_solution_cost(inst, sol);

    if (inst->verbose >= GOOD) {

        check_sol(inst, sol);

    }

}

// Build a CPLEX solution from a solution struct
void build_CPLEXsol_from_solution(const instance *inst, const solution *sol, double *xheu) {

    // Initialize every edge as not selected
    for (int i=0; i<inst->ncols; i++) {

        xheu[i] = 0.0;

    }

    // Select the edges in the solution
    for (int i=0; i<inst->nnodes; i++) {

        xheu[xpos(sol->visited_nodes[i],sol->visited_nodes[i+1],inst)] = 1.0;

    }

}

// Free memory associated with CPLEX
void free_CPLEX(CPXENVptr *env, CPXLPptr *lp) {

    // Free and close CPLEX model
    if (env != NULL && *env != NULL) {

        if (lp != NULL && *lp != NULL) {

            CPXfreeprob(*env, lp);
            *lp = NULL;

        }

        CPXcloseCPLEX(env);
        *env = NULL;

    }

}

// Freeze execution until a user input is entered
void press_a_key(void) {

    printf("Press any key to continue...\n");
    char ch = getchar();
    fflush(NULL);

}

