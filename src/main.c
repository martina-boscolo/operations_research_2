#include "tsp.h"
#include "utilities.h"
#include "utilities_instance.h"
#include "utilities_solution.h"
#include <stdio.h>



int main(int argc, const char *argv[]) {

    /*instance inst1;
    initialize_instance(&inst1);
    inst1.nnodes = 10;
    build_instance(&inst1);

    solution sol1;
    initialize_solution(&sol1);
    allocate_solution(&sol1, inst1.nnodes);
    initialize_tour(sol1.visited_nodes, inst1.nnodes);
    sol1.cost = compute_solution_cost(&inst1, &sol1);
    print_solution(&sol1, inst1.nnodes);

    printf("BEFORE CPLEX\n");
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP_Problem");
    build_model_CPLEX(&inst1, env, lp);
    printf("AFTER CPLEX\n");

    int *succ = (int *) malloc(inst1.nnodes * sizeof(int));
    int *comp = (int *) malloc(inst1.nnodes * sizeof(int));
    double *xstar = (double *) malloc(CPXgetnumcols(env, lp) * sizeof(double));
    int ncomp = -1;

    if (succ==NULL || comp==NULL || xstar==NULL) print_error("MEMORY MAIN");

    printf("BEFORE OPT SOL\n");
    get_optimal_solution_CPLEX(&inst1, env, lp, xstar, succ, comp, &ncomp);
    printf("AFTER OPT SOL\n");

    printf("BEFORE PATCH\n");
    patch_heuristic(&inst1, succ, comp, ncomp);
    printf("Number of components: %d\n", ncomp);
        for (int i = 0; i < inst1.nnodes; i++) {
            printf("Node %d -> Successor: %d, Component: %d\n", i, succ[i], comp[i]);
        }
    printf("AFTER PATCH\n");
    build_solution_form_CPLEX(&inst1, &sol1, succ);
    print_solution(&sol1, inst1.nnodes);*/

    //////////////////////////////////////////////////////////////////////////////////////////

    // problem instance
    instance inst;

    // parse the command line
    parse_command_line(argc, argv, &inst);

    // build the instance
    build_instance(&inst);

    // starting solving time
    inst.t_start = seconds();
    
    // compute the solution
    solve_with_method(&inst, inst.best_solution); 

    if (inst.verbose >= DEBUG) {
        // print solution
        printf("\n\n*********SOLUTION*********\n\n");
        print_solution(inst.best_solution, inst.nnodes);
        printf("\n\n");
    }

    // plot the solution
    plot_solution(&inst, inst.best_solution);

    printf("$STAT;%s;%d;%lf;", inst.best_solution->method, inst.seed, inst.best_solution->cost);

    // free the instance and its solution
    free_instance(&inst);
 
    return 0;
}

