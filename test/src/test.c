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

    }

    build_instance(&inst);

    if (inst.verbose >= 50) {

        // print metadata
        printf("Test instance result: \n\n");
        print_instance(&inst);

        printf("\n\n");
    
    }

    //make_test_solution(&inst, &sol);

    nn_main(&inst, &sol);

    if (inst.verbose >= 50) {

        printf("Test solution result:\n\n");
        print_solution(&sol, inst.nnodes);
        printf("\n\n");

    }

    plot_solution(&inst, &sol);

    free_instance(&inst);
    free_solution(&sol);

    return 0;

}

// As solution it takes the nodes in order from 0 to nnodes and then 0 again
void make_test_solution(instance *inst, solution *sol) {

    sol->visited_nodes = (int*) malloc((inst->nnodes + 1) * sizeof(int));

    for(int i=0; i<inst->nnodes; i++) { 
        sol->visited_nodes[i] = i; 
    }
    sol->visited_nodes[inst->nnodes] = 0;

}