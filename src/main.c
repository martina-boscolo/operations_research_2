#include "tsp.h"
#include "utilities.h"

int main(int argc, const char *argv[]) {

    // problem instance
    instance inst;
    // initial time
    double t0 = seconds();

    // parse the command line
    parse_command_line(argc, argv, &inst);
    // time to parse the command line
    double t_parse = seconds();

    // build the instance
    // for now onli random instance is available
    // todo add tsplib instances
    random_instance(&inst);
    // time to build the instance
    double t_instance = seconds();

    // solution of the instance
    //solution sol;

    // compute the solution
    // todo compute solution
    // time to compute the solution
    //double t_solution = seconds();

    // plot the solution
    //plot_solution(&inst, &sol);

    // free the instance and its solution
    free_instance(&inst);
    //free_solution(&sol);

    return 0;
}
