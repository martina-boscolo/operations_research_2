#include "utilities.h"
#include "heuristics.h"

int main(int argc, const char *argv[]) {
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