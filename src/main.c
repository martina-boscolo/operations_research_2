#include "tsp.h"
#include "utilities.h"
#include "utilities_instance.h"
#include "utilities_solution.h"
#include <stdio.h>



int main(int argc, const char *argv[]) {

    // problem instance
    instance inst;

    // parse the command line
    parse_command_line(argc, argv, &inst);

    // build the instance
    build_instance(&inst);

    // starting solving time
    inst.t_start = get_time_in_milliseconds();    
    
    // compute the solution
    solve_with_method(&inst, inst.best_solution); 

    if (inst.verbose >= DEBUG_V) {
        // print solution
        printf("\n\n*********SOLUTION*********\n\n");
        print_solution(inst.best_solution, inst.nnodes);
        printf("\n\n");
    }

    // plot the solution
    plot_solution(&inst, inst.best_solution);

    printf("$STAT;%s;%d;%lf;%lf;", inst.best_solution->method, inst.seed, inst.best_solution->cost, get_elapsed_time(inst.t_start));

    // free the instance and its solution
    free_instance(&inst);
 
    return 0;
}

