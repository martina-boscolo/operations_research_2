#include "utilities.h"
#include "heuristics.h"

int main(int argc, const char *argv[]) {

    instance inst;

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
    free_solution(&sol);

    return 0;

}