#include "benders.h"

void benders_loop(const instance *inst, solution *sol, const double timelimit) {

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    solution temp_best_sol;
    copy_sol(&temp_best_sol, sol, inst->nnodes);

    double t_start = get_time_in_milliseconds();

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    int *succ = (int *) malloc(inst->nnodes * sizeof(int));
    int *comp = (int *) malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *) malloc(CPXgetnumcols(env, lp) * sizeof(double));
    int ncomp = -1, iter = 0;
    double z = 0.0;

    if (succ == NULL || comp == NULL || xstar == NULL) print_error("benders_loop(): Impossible to allocate memory.");

    FILE* f;
    if (inst->verbose >= ONLY_INCUMBMENT) {
        char filename[65];
        sprintf(filename, "results/benders.csv");
        f = fopen(filename, "w+");
    }
    do {
        
        iter++;

        double residual_time = timelimit - get_elapsed_time(t_start);
        if (residual_time <= 0) break;

        CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);

        get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

        // check xstar optimality

        if (CPXgetbestobjval(env, lp, &z)) print_error("CPXgetobjval() error");

        if (inst->verbose >= ONLY_INCUMBMENT)
        {
            printf("Iteration %4d, Lower bound %10.2f, ncomp %4d, time %5.2f\n", iter, z, ncomp, get_time_in_milliseconds() - t_start);
            fflush(NULL);
            fprintf(f, "%d,%f,%f\n", iter, z, get_time_in_milliseconds() - t_start);


            if (inst->verbose >= GOOD)
            {

                int **subtours;
                int *subtour_lengths;

                extract_subtours_from_successors(inst, succ, &subtours, &subtour_lengths, &ncomp);
                plot_subtours(inst, subtours, subtour_lengths, ncomp, iter);

                for (int k = 0; k < ncomp; k++)
                {
                    free(subtours[k]);
                }
                free(subtours);
                free(subtour_lengths);
            }
        }

        if (ncomp > 1) {

            //build_SECs(inst, env, lp, comp, ncomp);
            add_SECs(inst, env, lp, comp, ncomp);

            patch_heuristic(inst, &temp_sol, succ, comp, ncomp, timelimit - get_elapsed_time(t_start));
            update_sol(inst, &temp_best_sol, &temp_sol, true);

            if (inst->verbose >= GOOD) {
                sprintf(temp_sol.method, "PatchHeuristic_subtours_iter%d", iter);
                plot_solution(inst, &temp_sol);
            }

        }

    } while (ncomp > 1);

    build_solution_form_CPLEX(inst, &temp_sol, succ);
    update_sol(inst, &temp_best_sol, &temp_sol, true);
    sprintf(temp_best_sol.method, BENDERS);

    update_sol(inst, sol, &temp_best_sol, false);

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);

    // Free and close CPLEX model
    free_CPLEX(&env, &lp);
    fclose(f);

    free_solution(&temp_best_sol);
    free_solution(&temp_sol);

    if (inst->verbose >= ONLY_INCUMBMENT) {
        plot_stats_in_file_base("benders");
    }

}

void patch_heuristic(const instance *inst, solution *sol, int *succ, int *comp, int ncomp, const double timelimit) {

    double t_start = get_time_in_milliseconds();

    while (ncomp > 1) {

        int i = 0, j = 1;

        while (comp[j] == comp[i]) j++;

        double min_delta = INFINITY;
        int best_j = j, best_i = i;
        int i1 = succ[i], j1 = succ[j];
        bool to_reverse = false;

        // find best patch
        do {
            do {

                double delta_d = delta_dir(i1, j1, inst, succ), delta_r = delta_rev(i1, j1, inst, succ);
                
                if (delta_d <= delta_r) {
                    if (delta_d < min_delta) {
                        min_delta = delta_d;
                        to_reverse = false;
                        best_i = i1;
                        best_j = j1;
                    }
                } else {
                    if (delta_r < min_delta) {
                        min_delta = delta_r;
                        to_reverse = true;
                        best_i = i1;
                        best_j = j1;
                    }
                }

                j1 = succ[j1];
            } while (j1 != j);
            i1 = succ[i1];
        } while (i1 != i);

        j = best_j;
        i = best_i;

        if (to_reverse) { 
            
            // (i, succ_i), (j,succ_j) -> (i,j), (succ_j,succ_i)
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

    build_solution_form_CPLEX(inst, sol, succ);

    two_opt(inst, sol, timelimit - get_elapsed_time(t_start), false);

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

    while (j != i) {
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

void extract_subtours_from_successors(const instance *inst, int *succ,
                                      int ***subtours_out, int **subtour_lengths_out, int *ncomp_out)
{
    int **subtours = (int **)malloc(inst->nnodes * sizeof(int *));
    int *subtour_lengths = (int *)malloc(inst->nnodes * sizeof(int));
    int *visited = (int *)calloc(inst->nnodes, sizeof(int));

    int ncomp = 0;

    for (int i = 0; i < inst->nnodes; i++)
    {
        if (visited[i])
            continue;

        int *this_subtour = (int *)malloc(inst->nnodes * sizeof(int));
        int len = 0;
        int curr = i;

        while (!visited[curr])
        {
            this_subtour[len++] = curr;
            visited[curr] = 1;
            curr = succ[curr];

            if (curr < 0 || curr >= inst->nnodes)
            {
                printf("Error: Invalid succ[%d] = %d\n", curr, succ[curr]);
                break;
            }

            if (len > inst->nnodes)
            {
                printf("Error: Infinite loop in successor chain starting at node %d\n", i);
                break;
            }
        }

        subtours[ncomp] = (int *)malloc(len * sizeof(int));
        memcpy(subtours[ncomp], this_subtour, len * sizeof(int));
        subtour_lengths[ncomp] = len;
        free(this_subtour);
        ncomp++;
    }

    *subtours_out = subtours;
    *subtour_lengths_out = subtour_lengths;
    *ncomp_out = ncomp;

    free(visited);

}
