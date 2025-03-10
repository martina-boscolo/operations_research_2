#include "tsp.h"
#include "utilities.h"

int main(int argc, const char *argv[]) {

    // problem instance
    instance inst;
    // initial time
    time_t t0 = seconds();

    // parse the command line
    parse_command_line(argc, argv, &inst);
    // time to parse the command line
    double t_parse = seconds();

    // build the instance
    build_instance(&inst);
    // time to build the instance
    double t_instance = seconds();
    inst.t_start = t_instance;

    
    // solution of the instance
    solution sol;
    // compute the solution
    solve_with_method(&inst, &sol); 
    // time to compute the solution
    time_t t_solution = seconds();

    // plot the solution
    plot_solution(&inst, &sol);

    // free the instance and its solution
    free_instance(&inst);
    free_solution(&sol);

    return 0;
}
