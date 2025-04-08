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

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(&inst1, &env, &lp);

    int *succ = (int *) malloc(inst1.nnodes * sizeof(int));
    int *comp = (int *) malloc(inst1.nnodes * sizeof(int));
    double *xstar = (double *) malloc(CPXgetnumcols(env, lp) * sizeof(double));
    int ncomp = -1, iter = 0;
    double z = 0.0;

    get_optimal_solution_CPLEX(&inst1, env, lp, xstar, succ, comp, &ncomp);

    printf("BEFORE PATCH\n");
    patch_heuristic(&inst1, &sol1, succ, comp, ncomp);
    printf("AFTER PATCH\n");
    print_solution(&sol1, &inst1);*/

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

