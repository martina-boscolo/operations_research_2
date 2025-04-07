#include "tsp_cplex.h"

#define DEBUG_CPLEX      

int TSPopt(instance *inst, const double timelimit)
{  
    // Open CPLEX model
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (env == NULL) {
        print_error("Failed to create CPLEX environment.\n");
    }
    if (inst->verbose >= DEBUG) {
        printf("CPLEX environment created successfully.\n");
    }

    // Create problem object
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP_Problem");
    if (lp == NULL) {
        CPXcloseCPLEX(&env);
        print_error("Failed to create CPLEX problem object.");
    }
    if (inst->verbose >= DEBUG) {
        printf("CPLEX problem object created successfully.\n");
    }

    // Build the model
    build_model_CPLEX(inst, env, lp);
    if (inst->verbose >= DEBUG) {
        printf("Model built successfully.\n");
    }
    CPXsetintparam(env, CPXPARAM_RandomSeed, inst->seed);

    // Solve the model
    if (CPXmipopt(env, lp)) print_error("CPXmipopt() error");

    // Retrieve the optimal solution
    int ncols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(ncols, sizeof(double));
    if (CPXgetx(env, lp, xstar, 0, ncols - 1)) print_error("CPXgetx() error");

    if (inst->verbose >= GOOD) {
        // Print the solution
        for (int i = 0; i < inst->nnodes; i++) {
            for (int j = i + 1; j < inst->nnodes; j++) {
                if (xstar[xpos(i, j, inst)] > 0.5) {
                    printf("  ... x(%3d,%3d) = 1\n", i + 1, j + 1);
                }
            }
        }
    }

    // Build the solution (succ, comp, ncomp)
    int *succ = (int *) calloc(inst->nnodes, sizeof(int));
    int *comp = (int *) calloc(inst->nnodes, sizeof(int));
    int ncomp;
    build_cplex_sol(xstar, inst, succ, comp, &ncomp);

    if (inst->verbose >= GOOD) {
        // Print the solution components
        printf("Number of components: %d\n", ncomp);
        for (int i = 0; i < inst->nnodes; i++) {
            printf("Node %d -> Successor: %d, Component: %d\n", i + 1, succ[i] + 1, comp[i]);
        }
    }

    // Free allocated memory
    free(succ);
    free(comp);
    free(xstar);

    // Free and close CPLEX model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return 0; // or an appropriate nonzero error code
}

int xpos(int i, int j, instance *inst)      // to be verified                                           
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
    cname[0] = (char *) calloc(100, sizeof(char));

    // add binary var.s x(i,j) for i < j  
    for ( int i = 0; i < inst->nnodes; i++ )
    {
        for ( int j = i+1; j < inst->nnodes; j++ ) 
        {
            sprintf(cname[0], "x(%d,%d)", i+1,j+1);
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
        char sense = 'E';                     // 'E' for equality constraint 
        sprintf(cname[0], "degree(%d)", h+1); 
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

    if ( inst->verbose >= -100 ) CPXwriteprob(env, lp, "model.lp", NULL);   

    free(cname[0]);
    free(cname);

}

void build_cplex_sol(const double *xstar, instance *inst, int *succ, int *comp, int *ncomp) // build succ() and comp() wrt xstar()...
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

void build_solution(instance *inst, solution *sol, int *succ) {
	sol->visited_nodes[0] = 0;
	int s = succ[0];
	for (int i = 1; i <= inst->nnodes; i++) {
		sol->visited_nodes[i] = s;
		s = succ[s];
	}
	if (sol->visited_nodes[inst->nnodes] != 0) print_error("Infeasible solution, build_solution");
	sol->cost = compute_solution_cost(inst, sol);
}
