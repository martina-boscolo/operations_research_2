#include "benders.h"

void benders_loop(instance *inst, solution *sol, const double timelimit) {

    time_t t_start = seconds();

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    int *succ = (int *) calloc(inst->nnodes, sizeof(int));
    int *comp = (int *) calloc(inst->nnodes, sizeof(int));
    int ncomp = -1, z, iter = 0;

    do {
        
        iter++;

        CPXsetdblparam(env, CPX_PARAM_TILIM, (timelimit - get_elapsed_time(t_start)));

        get_optimal_solution_CPLEX(inst, env, lp, succ, comp, &ncomp);

        if (CPXgetobjval(env, lp, &z)) print_error("CPXgetobjval() error");

        printf("Iteration %d,\tLower bound %f\n", iter, z);

        if (ncomp > 1) {
            add_SECs(inst, env, lp, comp, ncomp);
            // add patch_heuristic
        }

    } while (ncomp > 1);

    build_solution_form_CPLEX(inst, sol, succ);
    print_solution(sol, inst->nnodes);

    // Free allocated memory
    free(succ);
    free(comp);

    // Free and close CPLEX model
    free_CPLEX(&env, &lp);

    sprintf(sol->method, BENDERS);

}