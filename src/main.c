#include "tsp.h"
#include "utilities.h"
#include "utilities_instance.h"
#include "utilities_solution.h"

int main(int argc, const char *argv[]) {

    // problem instance
    instance inst;

    // parse the command line
    parse_command_line(argc, argv, &inst);

    // build the instance
    build_instance(&inst);
    
    // solution of the instance
    solution sol;

    // starting solving time
    inst.t_start = seconds();
    
    // compute the solution
    solve_with_method(&inst, &sol); 
    update_best_sol(&inst, &sol);

    if (inst.verbose >= GOOD) {
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
    free_solution(&sol); 

    return 0;
}
