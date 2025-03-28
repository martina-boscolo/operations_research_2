#include "utilities.h"
#include "heuristics.h"

int main(int argc, const char *argv[]) {

    instance inst;
    initialize_instance(&inst);
    allocate_instance(&inst);
    inst.nnodes = 10;
    inst.verbose = DEBUG;
    build_instance(&inst);
    //print_instance(&inst);

    solution sol;
    initialize_solution(&sol);
    allocate_solution(&sol, inst.nnodes);
    initialize_tour(sol.visited_nodes, inst.nnodes);
    //print_solution(&sol, inst.nnodes);

    //printf("ENTRO IN NN\n");
    nearest_neighbor(&inst, &sol, 0);
    //printf("\n\nNN solution\n\n");
    //print_solution(&sol, inst.nnodes);

    update_best_sol(&inst, &sol);
    //printf("\n\nINST BEST solution\n\n");
    //print_solution(inst.best_solution, inst.nnodes);
    //printf("\n\nNN solution AGAIN\n\n");
    //print_solution(&sol, inst.nnodes);

    multi_start_nn(&inst, &sol, inst.timelimit);

    /*parse_command_line(argc, argv, &inst);

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

    plot_solution(&inst, inst.best_solution);*/
    free_instance(&inst);
    free_solution(&sol);

    return 0;

}