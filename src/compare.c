#include  "compare.h"

int run_nn_algorithm(instance *inst, solution *sol) {
    initialize_solution(sol);
    allocate_solution(sol, inst->nnodes);
    inst->t_start = seconds();
    ms_2opt_nn_main(inst, sol);
    return 0;
}

int run_vns_algorithm(instance *inst, solution *sol) {
    initialize_solution(sol);
    allocate_solution(sol, inst->nnodes);
    inst->t_start = seconds();
    vns(inst, sol, 5);

    return 0;
}

int run_tabu_search_algorithm(instance *inst, solution *sol) {
    initialize_solution(sol);
    allocate_solution(sol, inst->nnodes);
    inst->t_start = seconds();
    tabu_search(inst, sol);
    return 0;
}
int compare(instance *inst) {

    // Open CSV file for results
    FILE *f = fopen("results/AlgorithmComparison.csv", "w+");
    if (!f) {
        perror("Failed to open results file");
        return -1;
    }

    // Write CSV header
    fprintf(f, "Seed,NN,VNS,Tabu Search\n");

    // Loop through seeds 1 to 50
    for (int seed = 1; seed <= INSTANCE_COMPARE; seed++) {
        printf("---------ITERATION: %d--------------------\n", seed);
        fprintf(f, "%d", seed);  // Write the seed in the first column
        inst->seed = seed;      // Set the seed
        inst->timelimit = TIMELIMIT_COMPARE; // Set the time limit

        solution nn_sol, vns_sol, tabu_sol;

        // Run NN algorithm
        if (run_nn_algorithm(inst, &nn_sol) != 0) {
            fclose(f);
            return -1;
        }
        fprintf(f, ",%f", nn_sol.cost);

        // Run VNS algorithm
        if (run_vns_algorithm(inst, &vns_sol) != 0) {
            fclose(f);
            return -1;
        }
        fprintf(f, ",%f", vns_sol.cost);

        // Run Tabu Search algorithm
        if (run_tabu_search_algorithm(inst, &tabu_sol) != 0) {
            fclose(f);
            return -1;
        }
        fprintf(f, ",%f\n", tabu_sol.cost);

        // Free solutions
        free_solution(&nn_sol);
        free_solution(&vns_sol);
        free_solution(&tabu_sol);
    }

    // Close the CSV file
    fclose(f);

    return 0;
}
