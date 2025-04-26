#include "utilities.h"
#include "heuristics.h"

int main(int argc, const char *argv[]) {

    /*TEST PATCH
    instance inst1;
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

    /*TEST 3-OPT KICK
    int nnodes = 6, i1 = 0, i2 = 2, i3 = 4;

    instance inst;
    initialize_instance(&inst);
    inst.nnodes = nnodes;
    allocate_instance(&inst);
    build_instance(&inst);

    solution sol1, sol2, sol3, sol4;
    initialize_solution(&sol1);
    allocate_solution(&sol1, nnodes);
    initialize_tour(sol1.visited_nodes, nnodes);
    sol1.cost = compute_solution_cost(&inst, &sol1);
    
    initialize_solution(&sol2);
    allocate_solution(&sol2, nnodes);
    copy_sol(&sol2, &sol1, nnodes);

    
    initialize_solution(&sol3);
    allocate_solution(&sol3, nnodes);
    copy_sol(&sol3, &sol1, nnodes);

    
    initialize_solution(&sol4);
    allocate_solution(&sol4, nnodes);
    copy_sol(&sol4, &sol1, nnodes);

    printf("INITIAL SOLUTION\n");
    print_solution(&sol1, nnodes);

    printf("\n\nKICK1\n");
    fixed_three_opt_move1(&inst, &sol1,i1,i2,i3);
    check_sol(&inst, &sol1);
    print_solution(&sol1, nnodes);

    printf("\n\nKICK2\n");
    fixed_three_opt_move2(&inst, &sol2,i1,i2,i3);
    check_sol(&inst, &sol2);
    print_solution(&sol2, nnodes);

    printf("\n\nKICK3\n");
    fixed_three_opt_move3(&inst, &sol3,i1,i2,i3);
    check_sol(&inst, &sol3);
    print_solution(&sol3, nnodes);

    printf("\n\nKICK4\n");
    fixed_three_opt_move4(&inst, &sol4,i1,i2,i3);
    check_sol(&inst, &sol4);
    print_solution(&sol4, nnodes);
    */

    /*TEST 5-OPT KICK
    int nnodes = 12;

    instance inst;
    initialize_instance(&inst);
    inst.nnodes = nnodes;
    allocate_instance(&inst);
    build_instance(&inst);

    solution sol;
    initialize_solution(&sol);
    allocate_solution(&sol, nnodes);
    initialize_tour(sol.visited_nodes, nnodes);
    sol.cost = compute_solution_cost(&inst, &sol);

    print_solution(&sol, nnodes);

    printf("KICK\n");
    kick(&inst, &sol, 5, 1);
    check_sol(&inst, &sol);

    printf("5-OPT\n");
    sol.cost += delta5(&inst, &sol, 1,2,3,4,5);
    fixed_five_opt_move(&sol, nnodes, 1,2,3,4,5);
    check_sol(&inst, &sol);

    print_solution(&sol, nnodes);
    */
   
    /*LIKE OLD VERSION OF MAIN
    parse_command_line(argc, argv, &inst);

    // force to print everything
    if (inst.verbose < 100) {

        inst.verbose = DEBUG-10; 

        print_instance(&inst);

        printf("\n");

    } //is this really needed?

    build_instance(&inst);
    time(&inst.t_start);
    
    if (inst.verbose >= 50) {

        // print metadata
        printf("Test instance result: \n\n");
        print_instance(&inst);

        printf("\n\n");
    
    }
   
    
    solution sol;

    solve_with_method(&inst, &sol); 

    if (inst.verbose >= 50) {

        printf("Test solution result:\n\n");
        print_solution(inst.best_solution, inst.nnodes);
        printf("\n\n");

    }

    plot_solution(&inst, inst.best_solution);
    free_instance(&inst);
    free_solution(&sol);*/

    return 0;

}