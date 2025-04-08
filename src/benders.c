#include "benders.h"

void benders_loop(instance *inst, solution *sol, const double timelimit) {

    time_t t_start = seconds();

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    int *succ = (int *) malloc(inst->nnodes * sizeof(int));
    int *comp = (int *) malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *) malloc(CPXgetnumcols(env, lp) * sizeof(double));
    int ncomp = -1, iter = 0;
    double z = 0.0;

    do {
        
        iter++;

        double residual_time = timelimit - get_elapsed_time(t_start);
        if (residual_time <= 0) break;

        CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);

        get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

        // check xstar optimality

        if (CPXgetbestobjval(env, lp, &z)) print_error("CPXgetobjval() error");

        if (inst->verbose >= ONLY_INCUMBMENT) {
            printf("Iteration %4d, Lower bound %10.2f, ncomp %4d, time %5.2f\n", iter, z, ncomp, seconds()-t_start);
            fflush(NULL);
        }

        if (ncomp > 1) {
            build_SECs(inst, env, lp, comp, ncomp);
            //patch_heuristic(inst, sol, succ, comp, ncomp);
        }

    } while (ncomp > 1);

    build_solution_form_CPLEX(inst, sol, succ);
    print_solution(sol, inst->nnodes);

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);

    // Free and close CPLEX model
    free_CPLEX(&env, &lp);

    sprintf(sol->method, BENDERS);

}

void patch_heuristic(instance *inst, solution *sol, int *succ, int *comp, int ncomp) {

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    while (ncomp > 1) {

        int i = 0, j = 1;

        while (comp[j] == comp[i]) j++;

        double min_delta = INFINITY;
        int best_j = j, h = succ[j];
        bool to_reverse = false;

        // find best patch
        do {

            double delta_d = delta_dir(i, h, inst, succ), delta_r = delta_rev(i, h, inst, succ);
            
            if (delta_d <= delta_r) {
                if (delta_d < min_delta) {
                    min_delta = delta_d;
                    to_reverse = false;
                    best_j = h;
                }
            } else {
                if (delta_r < min_delta) {
                    min_delta = delta_r;
                    to_reverse = true;
                    best_j = h;
                }
            }

            h = succ[h];
        } while (h != j);

        j = best_j;

        if (to_reverse) { 
            
            // (i, succ_i), (j,succ_j) -> (i,j), (succ_j,succ_i)
            // to check
            int succ_i = succ[i];
            int succ_j = succ[j];
            // reverse succ
            reverse_succ(j, succ);
            succ[i] = j;
            succ[succ_j] = succ_i;
            
        } else {
            // (i, succ_i), (j,succ_j) -> (i,succ_j), (j,succ_i)
            int succ_i = succ[i];
            succ[i] = succ[j];
            succ[j] = succ_i;
        }
        // update component
        update_comp(succ[i], comp[i], succ, comp);

        ncomp --;

    }

    build_solution_form_CPLEX(inst, &temp_sol, succ);
    strcpy(temp_sol.method, "PATCH");

    if (temp_sol.cost < sol->cost) {
        update_sol(inst, sol, &temp_sol, true);
    }

}

// (i, succ_i), (j,succ_j) -> (i,succ_j), (j,succ_i)
double delta_dir(const int i, const int j, const instance *inst, const int *succ) {
    double old_cost = cost(i, succ[i], inst) + cost(j, succ[j], inst);
    double new_cost = cost(i, succ[j], inst) + cost(j, succ[i], inst);
    return new_cost - old_cost;
}

// (i, succ_i), (j,succ_j) -> (i,j), (succ_j,succ_i)
double delta_rev(const int i, const int j, const instance *inst, const int *succ) {
    double old_cost = cost(i, succ[i], inst) + cost(j, succ[j], inst);
    double new_cost = cost(i, j, inst) + cost(succ[j], succ[i], inst);
    return new_cost - old_cost;
}

void reverse_succ(const int i, int *succ) {

    int j = succ[i];
    int sj = succ[j];
    int ssj = succ[sj];

    while (j != i) { //check
        succ[sj] = j;
        j = sj;
        sj = ssj;
        ssj = succ[ssj];

    }

}

void update_comp(int i, const int new_comp, const int *succ, int *comp) {

    while (comp[i] != new_comp) {
        comp[i] = new_comp;
        i = succ[i];
    }

}
