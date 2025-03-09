#include "utilities.h"
#include "heuristics.h"

void make_test_solution(instance *inst, solution *sol);

int main(int argc, const char *argv[]) {

    instance inst;
    solution sol;

    parse_command_line(argc, argv, &inst, &sol);

    // force to print everything
    if (inst.verbose < 100) {

        inst.verbose = 100; 

        print_instance(&inst);

        print_solution(&sol, inst.nnodes);

        printf("\n");

    } //is this really needed?

    build_instance(&inst);

    if (inst.verbose >= 50) {

        // print metadata
        printf("Test instance result: \n\n");
        print_instance(&inst);

        printf("\n\n");
    
    }
   
    solve_with_method(&inst, &sol); 

    if (inst.verbose >= 50) {

        printf("Test solution result:\n\n");
        print_solution(inst.best_solution, inst.nnodes);
        printf("\n\n");

    }

    plot_solution(&inst, inst.best_solution);
    free_instance(&inst);
    //should i free also inst.best_solution?
    free_solution(&sol);

    return 0;

}