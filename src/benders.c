#include "benders.h"

// Benders loop algorithm
void benders_loop(instance *inst, solution *sol, const double timelimit) {

    double t_start = get_time_in_milliseconds();
    bool updated = false;
    bool is_asked_method = (strcmp(inst->asked_method, BENDERS) == 0);

    solution temp_sol;
    copy_sol(&temp_sol, sol, inst->nnodes);

    // Open CPLEX model
    CPXENVptr env;
    CPXLPptr lp;
    initialize_CPLEX(inst, &env, &lp);

    // Instantiate memory
    int *succ = (int *) malloc(inst->nnodes * sizeof(int));
    int *comp = (int *) malloc(inst->nnodes * sizeof(int));
    double *xstar = (double *) malloc(inst->ncols * sizeof(double));
    if (succ == NULL || comp == NULL || xstar == NULL) print_error("allocate_CPLEXsol(): Cannot allocate memory");

    // Initialize the number of components and the number of iterations and the objective value
    int ncomp = -1, iter = 0;
    double z = 0.0;

    // Save results in a file, if required
    FILE *f;
    if (inst->verbose >= ONLY_INCUMBMENT) {

        char filename[FILE_NAME_LEN];
        sprintf_s(filename, FILE_NAME_LEN, "results/benders.csv");
        if (fopen_s(&f, filename, "w+")) print_error("benders_loop(): Cannot open file");

    }

    do {
        
        iter++;

        double residual_time = timelimit - get_elapsed_time(t_start);
        if (residual_time <= 0) break;

        // Update CPLEX time limit
        CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time);

        // Obtain optimal solution, and check if time limit is reached 
        get_optimal_solution_CPLEX(inst, env, lp, xstar, succ, comp, &ncomp);

        if (CPXgetbestobjval(env, lp, &z)) print_error("CPXgetobjval(): Error");

        // If required print the iteration statistics
        if (inst->verbose >= ONLY_INCUMBMENT) {

            printf("Iteration %5d, Lower bound %10.6f, ncomp %5d, time %10.6f\n", iter, z, ncomp, get_time_in_milliseconds() - t_start);
            fflush(NULL);
            fprintf(f, "%d,%f,%f\n", iter, z, get_time_in_milliseconds() - t_start);

            // If required plot the subtours
            if (inst->verbose >= GOOD) {

                int **subtours = (int **)malloc(inst->nnodes * sizeof(int *));
                int *subtour_lengths = (int *)malloc(inst->nnodes * sizeof(int));
                if (subtours == NULL || subtour_lengths == NULL) print_error("benders_loop(): Cannot allocate memory");

                extract_subtours_from_successors(inst, succ, &subtours, &subtour_lengths, &ncomp);
                plot_subtours(inst, subtours, subtour_lengths, ncomp, iter);

                for (int k=0; k<ncomp; k++) {

                    free(subtours[k]);

                }

                free(subtours);
                free(subtour_lengths);
            }

        }

        // If the solution is unfeasible add the correspondent SECs and use patch heuristic
        if (ncomp > 1) {

            add_SECs_to_model(inst, env, lp, comp, ncomp, iter);

            patch_heuristic(inst, &temp_sol, succ, comp, ncomp, timelimit - get_elapsed_time(t_start));
            updated = updated || update_sol(inst, sol, &temp_sol, true);

            // If asked plot the patch
            if (inst->verbose >= GOOD) {

                sprintf_s(temp_sol.method, METH_NAME_LEN, "PatchHeuristic_iter%d", iter);
                plot_solution(inst, &temp_sol);

            }

        }

    } while (ncomp > 1);

    if (ncomp == 1) {
        build_solution_from_CPLEX(inst, &temp_sol, succ);
        updated = updated || update_sol(inst, sol, &temp_sol, true);
    }
    
    if (updated && is_asked_method) {

        sprintf_s(sol->method, METH_NAME_LEN, BENDERS);

    }

    // Free allocated memory
    free(xstar);
    free(comp);
    free(succ);

    // Free and close CPLEX model
    free_CPLEX(&env, &lp);
    fclose(f);

    free_solution(&temp_sol);

    if (inst->verbose >= ONLY_INCUMBMENT) {

        plot_stats_in_file_base("benders");

    }

    // Close the file if it was opened
    if (f != NULL) {

        fclose(f);

    }
    
}

// For each connected component add the correspondent SEC to the lp model
void add_SECs_to_model(const instance *inst, CPXENVptr env, CPXLPptr lp, const int *comp, const int ncomp, const int iter) {

    // Set values for model constraints
    int izero = 0;
    char sense = 'L'; 

    char **cname = (char **) calloc(1, sizeof(char*));
    if (cname == NULL) print_error("add_SECs_to_model(): Cannot allocate memory");
    cname[0] = (char *) calloc(CONS_NAME_LEN, sizeof(char));
    
    int *index = (int *) malloc(inst->ncols * sizeof(int));
    double *value = (double *) malloc(inst->ncols * sizeof(double)); 

	if (cname[0]==NULL || index==NULL || value==NULL) 
		print_error("Impossible to allocate memory, add_SECs_to_model()");

    int nnz;
    double rhs;

    // For each connected component add the correspondent SEC to the model
    for (int k=1; k<=ncomp; k++) {
        
        sprintf_s(cname[0], CONS_NAME_LEN, "%5d SEC(%5d)", iter, k); 

        build_SEC(inst, comp, k, index, value, &nnz, &rhs);

        if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0])) print_error("CPXaddrows(): Cannot add SEC");

    }

    // If asked store the model in a file
    if (inst->verbose >= GOOD) CPXwriteprob(env, lp, "model.lp", NULL);   

    // Free allocated memory
	free(value);
    free(index);
    free(cname[0]);
    free(cname);

}

// Build a feasible solution from different subtours
void patch_heuristic(const instance *inst, solution *sol, int *succ, int *comp, int ncomp, const double timelimit) {

    double t_start = get_time_in_milliseconds();

    // While there are more than one connected component patch the connected component of node 0 to another component
    while (ncomp > 1) {

        int i = 0, j = 1;

        while (comp[j] == comp[i]) j++;

        // Initialize values:
        // - minimum cost to add with the patch
        double min_delta = INFINITY;
        // - edges associated with the patch
        int best_i = i, best_j = j;
        // - their successors
        int i1 = succ[i], j1 = succ[j];
        // - if directed or reverse patch
        bool to_reverse = false;

        // Find best patch
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

        // Patch the connected components
        if (to_reverse) { 
            
            // Remove edges (i, succ_i), (j,succ_j)
            // Add edges (i,j), (succ_j,succ_i)
            int succ_i = succ[i];
            int succ_j = succ[j];

            // reverse succ
            reverse_succ(j, succ);

            succ[i] = j;
            succ[succ_j] = succ_i;
            
        } else {

            // Remove edges (i, succ_i), (j,succ_j) 
            // Add edges (i,succ_j), (j,succ_i)
            int succ_i = succ[i];
            succ[i] = succ[j];
            succ[j] = succ_i;
        }

        // Update component
        update_comp(succ[i], comp[i], succ, comp);

        ncomp --;

    }

    // Obtain the solution that corresponds to the patch
    build_solution_from_CPLEX(inst, sol, succ);

    // Improve the solution using 2-opt refinement
    two_opt(inst, sol, timelimit - get_elapsed_time(t_start), false);

}

// Compute the delta cost removing edges (i, succ_i), (j,succ_j) and adding the edges (i,succ_j), (j,succ_i)
double delta_dir(const int i, const int j, const instance *inst, const int *succ) {

    // Compute the cost of the two edges that would be removed
    double old_cost = cost(i, succ[i], inst) + cost(j, succ[j], inst);

    // Compute the cost of the two new edges that would be added
    double new_cost = cost(i, succ[j], inst) + cost(j, succ[i], inst);

    return new_cost - old_cost;

}

// Compute the delta cost removing edges (i, succ_i), (j,succ_j) and adding the edges (i,j), (succ_j,succ_i)
double delta_rev(const int i, const int j, const instance *inst, const int *succ) {

    // Compute the cost of the two edges that would be removed
    double old_cost = cost(i, succ[i], inst) + cost(j, succ[j], inst);
    
    // Compute the cost of the two new edges that would be added
    double new_cost = cost(i, j, inst) + cost(succ[j], succ[i], inst);

    return new_cost - old_cost;

}

// Reverse the order of the subtour that contains node i in succ 
void reverse_succ(const int i, int *succ) {

    // a node
    int j = succ[i];
    // its successor
    int sj = succ[j];
    // the successor of its successor
    int ssj = succ[sj];

    while (j != i) {

        succ[sj] = j;
        j = sj;
        sj = ssj;
        ssj = succ[ssj];

    }

}

// Update the connected component
void update_comp(int i, const int new_comp, const int *succ, int *comp) {

    while (comp[i] != new_comp) {

        comp[i] = new_comp;
        i = succ[i];

    }

}

// Function to extract subtours from the successors array
void extract_subtours_from_successors(const instance *inst, const int *succ, int **subtours, int *subtour_lengths, int *ncomp) {

    int *visited = (int *)calloc(inst->nnodes, sizeof(int));
    if (visited == NULL) print_error("extract_subtours_from_successors(): Cannot allocate memory");

    *ncomp = 0;

    for (int i=0; i<inst->nnodes; i++) {

        if (visited[i]) continue;

        int *this_subtour = (int *)malloc(inst->nnodes * sizeof(int));
        if (this_subtour == NULL) print_error("extract_subtours_from_successors(): Cannot allocate memory");

        int len = 0;
        int curr = i;

        while (!visited[curr]) {

            this_subtour[len++] = curr;
            visited[curr] = 1;
            curr = succ[curr];

            if (curr < 0 || curr >= inst->nnodes) { print_error("extract_subtours_from_successors(): Invalid successor");

            if (len > inst->nnodes) print_error("extract_subtours_from_successors(): Infinite loop in successor chain");

        }

        subtours[*ncomp] = (int *)malloc(len * sizeof(int));
        if (subtours[*ncomp] == NULL) print_error("extract_subtours_from_successors(): Cannot allocate memory");

        memcpy(subtours[*ncomp], this_subtour, len * sizeof(int));
        subtour_lengths[*ncomp] = len;

        free(this_subtour);
        
        ncomp++;

    }

    free(visited);

}
