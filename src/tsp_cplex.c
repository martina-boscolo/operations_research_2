#include "tsp_cplex.h"

#define DEBUG_CPLEX

void initialize_CPLEX(instance *inst, CPXENVptr *env, CPXLPptr *lp) {

    // Open CPLEX model
    int error;
    *env = CPXopenCPLEX(&error);
    if (*env == NULL) {
        print_error("Failed to create CPLEX environment.\n");
    }

    // Create problem object
    *lp = CPXcreateprob(*env, &error, "TSP_Problem");
    if (*lp == NULL) {
        CPXcloseCPLEX(env);
        print_error("Failed to create CPLEX problem object.");
    }

    // Set CPLEX parameters for better performance
    CPXsetdblparam(*env, CPX_PARAM_EPGAP, 1e-9);
    CPXsetintparam(*env, CPX_PARAM_MIPEMPHASIS, CPX_MIPEMPHASIS_BALANCED);
    CPXsetintparam(*env, CPX_PARAM_PROBE, 2);
    CPXsetintparam(*env, CPX_PARAM_HEURFREQ, 10);
    //CPXsetintparam(*env, CPX_PARAM_THREADS, 1); //only for debug
    if(inst->verbose >= GOOD)
        CPXsetintparam(*env, CPX_PARAM_SCRIND, CPX_ON);

    // Build the model
    build_model_CPLEX(inst, *env, *lp);

}

void warm_up(const instance *inst, solution *sol, CPXENVptr env, CPXLPptr lp) {

    if (inst->verbose >= GOOD) {
        printf("Warmup with cost %lf\n", sol->cost);
    }

    double *xheu = (double *) calloc(inst->ncols, sizeof(double));
    if (xheu == NULL) print_error("warm_up(): Cannot allocate memory");

    build_CPLEXsol_from_solution(inst, sol, xheu);

    int *ind = (int *) malloc(inst->ncols * sizeof(int));
	for ( int j = 0; j < inst->ncols; j++ ) ind[j] = j;
	int effortlevel = CPX_MIPSTART_NOCHECK;  
	int beg = 0;

	if (CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, ind, xheu, &effortlevel, NULL)) print_error("CPXaddmipstarts() error");

	free(ind);
    free(xheu);

}

int get_optimal_solution_CPLEX(const instance *inst, CPXENVptr env, CPXLPptr lp, double *xstar, int *succ, int *comp, int *ncomp) {
    
	// Solve the model
	if (CPXmipopt(env, lp)) print_error("CPXmipopt() error");

    int status = CPXgetstat(env, lp);
    if (!(status == CPXMIP_OPTIMAL || status == CPXMIP_OPTIMAL_TOL || status == CPXMIP_TIME_LIM_FEAS)) return status;

    // Retrieve the optimal solution
    int ncols = CPXgetnumcols(env, lp);
    if (CPXgetx(env, lp, xstar, 0, ncols - 1)) print_error("CPXgetx() error");

    // Build the solution (succ, comp, ncomp)
    build_sol_CPLEX(xstar, inst, succ, comp, ncomp);

    if (inst->verbose >= DEBUG_V) {
        // Print the solution components
        printf("Number of components: %d\n", *ncomp);
        for (int i = 0; i < inst->nnodes; i++) {
            printf("Node %d -> Successor: %d, Component: %d\n", i + 1, succ[i] + 1, comp[i]);
        }
    }

    return 0;

}

void build_SEC(const instance *inst, const int *comp, const int sec_comp, int *index, double *value, int *nnz, double *rhs) {

    *nnz = 0;
    *rhs = -1;

    for (int i=0; i<inst->nnodes; i++) {

		if (inst->verbose >= DEBUG_V) printf("comp[%d] = %d\n", i+1, comp[i]);
        if (comp[i] != sec_comp) continue;

        (*rhs)++;

        for (int j=i+1; j<inst->nnodes; j++) {

            if (comp[j] != sec_comp) continue;

			if (inst->verbose >= DEBUG_V) printf("Add edge %d %d to SEC %d\n", i+1, j+1, sec_comp);

			index[*nnz] = xpos(i, j, inst);
			value[*nnz] = 1.0;
			(*nnz)++;

        }

    }

}

int xpos(int i, int j, const instance *inst)                                         
{ 
    if ( i == j ) print_error(" i == j in xpos" );
    if ( i > j ) return xpos(j,i,inst);
    int pos = i * inst->nnodes + j - (( i + 1 ) * ( i + 2 )) / 2;
    return pos;
}

void build_model_CPLEX(instance *inst, CPXENVptr env, CPXLPptr lp)
{    

    int izero = 0;
    char binary = 'B'; 
    
    char **cname = (char **) calloc(1, sizeof(char*));
    cname[0] = (char *) calloc(CONS_NAME_LEN, sizeof(char));

    // add binary var.s x(i,j) for i < j  
    for ( int i = 0; i < inst->nnodes; i++ )
    {
        for ( int j = i+1; j < inst->nnodes; j++ ) 
        {
            sprintf_s(cname[0], CONS_NAME_LEN, "x(%d,%d)", i+1,j+1);
            double obj = cost(i,j,inst); // cost == distance   
            double lb = 0.0;
            double ub = 1.0;
            if ( CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname) ) print_error(" wrong CPXnewcols on x var.s");
            if ( CPXgetnumcols(env,lp)-1 != xpos(i,j, inst) ) print_error(" wrong position for x var.s");
        }
    } 

    // add degree constr.s 
    int *index = (int *) malloc(inst->nnodes * sizeof(int));
    double *value = (double *) malloc(inst->nnodes * sizeof(double));  
    
    // add the degree constraints
    for ( int h = 0; h < inst->nnodes; h++ )  // degree constraints
    {
        double rhs = 2.0;
        char sense = 'E'; // Equality constraint 
        sprintf_s(cname[0], CONS_NAME_LEN, "degree(%d)", h+1); 
        int nnz = 0;
        for ( int i = 0; i < inst->nnodes; i++ )
        {
            if ( i == h ) continue;
            index[nnz] = xpos(i,h, inst);
            value[nnz] = 1.0;
            nnz++;
        }
        
        if ( CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]) ) print_error(" wrong CPXaddrows [degree]");
    } 

    free(value);
    free(index);

    if (inst->verbose >= GOOD) CPXwriteprob(env, lp, "model.lp", NULL);   

    free(cname[0]);
    free(cname);

    inst->ncols = CPXgetnumcols(env, lp);

}

void build_sol_CPLEX(const double *xstar, const instance *inst, int *succ, int *comp, int *ncomp)
{   

#ifdef DEBUG_CPLEX
    int *degree = (int *) calloc(inst->nnodes, sizeof(int));
    for ( int i = 0; i < inst->nnodes; i++ )
    {
        for ( int j = i+1; j < inst->nnodes; j++ )
        {
            int k = xpos(i,j,inst);
            if ( fabs(xstar[k]) > EPSILON && fabs(xstar[k]-1.0) > EPSILON ) print_error(" wrong xstar in build_sol()");
            if ( xstar[k] > 0.5 ) 
            {
                ++degree[i];
                ++degree[j];
            }
        }
    }
    for ( int i = 0; i < inst->nnodes; i++ )
    {
        if ( degree[i] != 2 ) print_error("wrong degree in build_sol()");
    }    
    free(degree);
#endif

    *ncomp = 0;
    for ( int i = 0; i < inst->nnodes; i++ )
    {
        succ[i] = -1;
        comp[i] = -1;
    }
    
    for ( int start = 0; start < inst->nnodes; start++ )
    {
        if ( comp[start] >= 0 ) continue;  // node "start" was already visited, just skip it

        // a new component is found
        (*ncomp)++;
        int i = start;
        bool done = false;
        while ( !done )  // go and visit the current component
        {
            comp[i] = *ncomp;
            done = true;
            for ( int j = 0; j < inst->nnodes; j++ )
            {
                if ( i != j && xstar[xpos(i,j,inst)] > 0.5 && comp[j] == -1 ) // the edge [i,j] is selected in xstar and j was not visited before 
                {
                    succ[i] = j;
                    i = j;
                    done = false;
                    break;
                }
            }
        }    
        succ[i] = start;  // last arc to close the cycle
        
        // go to the next component...
    }
}

void build_solution_from_CPLEX(const instance *inst, solution *sol, int *succ) {
    
    sol->visited_nodes[0] = 0;
    int s = succ[0];
    for (int i = 1; i <= inst->nnodes+1; i++) {
        sol->visited_nodes[i] = s;
        s = succ[s];
    }
    sol->cost = compute_solution_cost(inst, sol);
    check_sol(inst, sol);

}

void build_CPLEXsol_from_solution(const instance *inst, const solution *sol, double *xheu) {

    for (int i=0; i<inst->ncols; i++) {
        xheu[i] = 0.0;
    }

    for (int i=0; i<inst->nnodes; i++) {
        xheu[xpos(sol->visited_nodes[i],sol->visited_nodes[i+1],inst)] = 1.0;
    }

}

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

void press_a_key(){
    printf("Press any key to continue...\n");
    char ch = getchar();
    fflush(NULL);
}

