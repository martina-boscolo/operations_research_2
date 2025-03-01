#include "utilities.h"

#include <windows.h>

void make_test_instance(instance *inst);
void make_test_solution(instance *inst, solution *sol);

int main() {

    instance inst;

    make_test_instance(&inst);

    if (inst.verbose >= 50) {

        // print metadata
        printf("Test instance result: \n\n");
        printf("Nnodes: %d\n", inst.nnodes);
        printf("Seed: %d\n", inst.seed);
        printf("Timelimit: %d\n", inst.timelimit); 
        printf("Verbose: %d\n", inst.verbose);

        printf("\n\n");
        
        // print nodes' coordinate
        for (int i=0; i<inst.nnodes; i++) { printf("Node %d: \t x %lf,\ty %lf\n", i, inst.coord[i].x, inst.coord[i].y); }
        
        printf("\n\n");
    }

    solution sol;

    make_test_solution(&inst, &sol);

    if (inst.verbose >= 50) {

        printf("Test solution result:\n\n");

        for (int i=0; i<inst.nnodes+1; i++) { printf("Visited node %d\n", sol.visited_nodes[i]); }
        
        printf("\n\n");

    }

    plot_solution(inst, sol);

    free_instance(&inst);
    free_solution(&sol);

    Sleep(100000);

    return 0;
}

// Make a random instance 
void make_test_instance(instance *inst) {

    inst->nnodes = 10;
    inst->seed = DEFAULT_SEED;
    inst->timelimit = DEFAULT_TIMELIMIT;
    inst->verbose = 50;

    random_instance(inst);

}

// As solution it takes the nodes in order from 0 to nnodes and then 0 again
void make_test_solution(instance *inst, solution *sol) {

    sol->visited_nodes = (int*) malloc((inst->nnodes + 1) * sizeof(int));

    for(int i=0; i<inst->nnodes; i++) { 
        sol->visited_nodes[i] = i; 
    }
    sol->visited_nodes[inst->nnodes] = 0;

}