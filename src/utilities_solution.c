#include "utilities_solution.h"

void initialize_solution(solution *sol) {

    sol->cost = INFINITY;
    sol->visited_nodes = NULL;
    sol->method[0] = EMPTY_STRING;

}

void initialize_tour(int *visited_nodes, const int nnodes) {
    for (int i = 0; i < nnodes; i++) {
        visited_nodes[i] = i;
    }
    visited_nodes[nnodes] = 0;
}

void solve_with_method(instance *inst, solution *sol) {

    initialize_solution(sol);
    allocate_solution(sol, inst->nnodes);
    _mkdir("./results"); //, 0777);

    double timelimit = inst->timelimit - get_elapsed_time(inst->t_start);

    if (strcmp(inst->asked_method, NEAREST_NEIGHBOR) == 0) {

        printf("Solving with Nearest Neighbor method.\n");
        multi_start_nn(inst, sol, timelimit); 

    } else if (strcmp(inst->asked_method, VNS) == 0) {

        printf("Solving with VNS method.\n");
        nearest_neighbor(inst, sol, rand() % inst->nnodes);
        if (inst->param1 != 3 && inst->param1 != 5) { inst->param1 = DEAULT_K; }
        if (inst->param2 < 1) { inst->param2 = DEFAULT_REPS; }
        double elapsed_time = get_elapsed_time(inst->t_start);
        vns(inst, sol, (timelimit-elapsed_time), inst->param1, inst->param2);
        
    } else if (strcmp(inst->asked_method, TABU_SEARCH) == 0) {

        printf("Solving with TABU_SEARCH method.\n");
        nearest_neighbor(inst, sol, rand() % inst->nnodes);
        tabu_search(inst, sol, timelimit);
        
    } else if (strcmp(inst->asked_method, BENDERS) == 0) {

        printf("Solving with Benders' loop method.\n");
        benders_loop(inst, sol, timelimit);
            
    } else if (strcmp(inst->asked_method, BRANCH_AND_CUT) == 0) {
        double start = get_time_in_milliseconds();
        printf("Solving with Branch and Cut method.\n");
        if (inst->param1 == 1) { // Warm-up if needed
            nearest_neighbor(inst, sol, rand() % inst->nnodes);
            double timelimit1 = timelimit * 0.1;
            timelimit1 = (timelimit1 > 1) ? 1 : timelimit1;
            two_opt(inst, sol, timelimit1, false);
        }
        double timelimit2 = timelimit - get_elapsed_time(start);
        branch_and_cut(inst, sol, timelimit2);
            
    } else if (strcmp(inst->asked_method, HF) == 0) {
        double start = get_time_in_milliseconds();
        printf("Solving with Hard fixing method.\n");
        // moved set parameters inside hard fixing
        // Warm-up always
        nearest_neighbor(inst, sol, rand() % inst->nnodes);
        double timelimit1 = timelimit * 0.1;
        tabu_search(inst, sol, timelimit1);
        two_opt(inst, sol, timelimit1, false); //will take less time
        double timelimit2 = timelimit - get_elapsed_time(start);
        hard_fixing(inst, sol, timelimit2);
            
    } else if (strcmp(inst->asked_method, LB) == 0) {
        double start = get_time_in_milliseconds();
        printf("Solving with Local Branching method.\n");
        inst->param2 = 1;
        inst->param3 = 1;
        // Warm-up always
        nearest_neighbor(inst, sol, rand() % inst->nnodes);
        double timelimit1 = timelimit * 0.1;
        tabu_search(inst, sol, timelimit1);
        two_opt(inst, sol, timelimit1, false); //will take less time
        double timelimit2 = timelimit - get_elapsed_time(start);
        local_branching(inst, sol, timelimit2 );
            
    } else {
        fprintf(stderr, "Error: Unknown method '%s'.\nPlease, select valid method\n", sol->method);
        printf("Valid methods are:\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n", NEAREST_NEIGHBOR, VNS, TABU_SEARCH, BENDERS, BRANCH_AND_CUT, HF, LB);
        exit(EXIT_FAILURE);
    }
    
}

bool validate_node_visits(const instance *inst, const solution *sol)
{
    int *visited = (int *)calloc(inst->nnodes, sizeof(int));
    if (!visited)
    {
        fprintf(stderr, "Memory allocation failed for visited array\n");
        exit(EXIT_FAILURE);
    }

    // Mark each visited node
    for (int i = 0; i < inst->nnodes; i++)
    {
        if (sol->visited_nodes[i] < 0 || sol->visited_nodes[i] >= inst->nnodes)
        {
            free(visited);
            return false; // Out-of-range node index
        }
        visited[sol->visited_nodes[i]] = 1;
    }

    // Check if all nodes are visited exactly once
    for (int i = 0; i < inst->nnodes; i++)
    {
        if (visited[i] != 1)
        {
            fprintf(stderr, "Node %d was not visited or visited more than once.\n", i);
            free(visited);
            return false; // A node was missed
        }
    }

    // Ensure the tour closes properly
    if (sol->visited_nodes[inst->nnodes] != sol->visited_nodes[0])
    {
        fprintf(stderr, "Ending node different from starting node\n");
        free(visited);
        return false; // Cycle not closed properly
    }

    free(visited);
    return true; 
}

double compute_solution_cost(const instance *inst, const solution *sol)
{
    double computed_cost = 0;
    for (int i = 0; i < inst->nnodes; i++)
    {
        computed_cost += cost(sol->visited_nodes[i], sol->visited_nodes[i + 1], inst);
    }
    return computed_cost;
}

bool validate_cost(const instance *inst, const solution *sol)
{
    double computed_cost = compute_solution_cost(inst, sol);
    if (fabs(computed_cost - sol->cost) > EPSILON)
    { // Floating-point error tolerance
        fprintf(stderr, "Cost mismatch! Expected: %lf, Computed: %lf\n", sol->cost, computed_cost);
        return false;
    }
    return true;
}

void check_sol(const instance *inst, solution *sol)
{
    if (!validate_node_visits(inst, sol))
    {
        printf("Problem in the visit of the nodes\n");
        exit(EXIT_FAILURE);
    }
    if (!validate_cost(inst, sol))
    {
        printf("Mismatch in the cost\n");
        exit(EXIT_FAILURE);
    }
    // Solution is valid
    if (inst->verbose >= DEBUG_V) {
        printf("Solution is valid\n");
    }

}

bool update_sol(const instance *inst, solution *sol1, const solution *sol2, bool print)
{
    if (sol2->cost < (sol1->cost - EPSILON))
    {
        if(print && inst->verbose >= ONLY_INCUMBMENT) {
            printf("Incumbment updated\nOld cost: %lf,\tNew cost: %lf\n", sol1->cost, sol2->cost);
        }
        sol1->cost = sol2->cost;
        strncpy_s(sol1->method, METH_NAME_LEN, sol2->method, _TRUNCATE);
        memcpy(sol1->visited_nodes, sol2->visited_nodes, (inst->nnodes + 1) * sizeof(int));
        return true;
    }
    return false;
}

void copy_sol(solution *sol1, const solution *sol2, const int nnodes) {
    initialize_solution(sol1);
    allocate_solution(sol1, nnodes);
    sol1->cost = sol2->cost;
    strncpy(sol1->method, sol2->method, METH_NAME_LEN - 1);
sol1->method[METH_NAME_LEN - 1] = '\0'; // ensure null-termination
    memcpy(sol1->visited_nodes, sol2->visited_nodes, (nnodes + 1) * sizeof(int));
}

void plot_solution(const instance *inst, const solution *sol) {

    //use gnuplot to print the solution
    FILE *gnuplot = open_plot();

    // in a file
    char filename[FILE_NAME_LEN];
    sprintf_s(filename, FILE_NAME_LEN, "%s_%s", inst->name, sol->method);
    plot_in_file(gnuplot, filename);

    // specify the customization
    add_plot_customization(gnuplot, "plot '-' using 1:2 w linespoints pt 7"); // notitle with lines");

    // plot edges
    for(int i=0; i<inst->nnodes; i++) {

        coordinate first = inst->coord[sol->visited_nodes[i]];
        coordinate second = inst->coord[sol->visited_nodes[i+1]];
        
        plot_edge(gnuplot, first, second);

    }

    input_end(gnuplot);

    free_plot(gnuplot);

}

void print_solution(const solution *sol, const int nnodes) {

    printf("Cost: %lf\n", sol->cost);
    printf("Method: %s\n", sol->method);

    printf("\n");

    if (sol->visited_nodes != NULL) {

        printf("Visited nodes:\n");

        for (int i=0; i<nnodes+1; i++) {
            printf("Node %d\n", sol->visited_nodes[i]);
        }

        printf("\n");

    }

}

void allocate_solution(solution *sol, const int nnodes) {

    free_solution(sol);

    sol->visited_nodes = (int *) calloc((nnodes + 1), sizeof(int));

    if (sol->visited_nodes == NULL) {
        printf("Cannot allocate memory");
        exit(EXIT_FAILURE);
    }

}

void free_solution(solution *sol) {
    if (sol == NULL){
        printf("Solution is NULL\n");
        return;
    } 
    
    if (sol->visited_nodes != NULL) {
        free(sol->visited_nodes);
        sol->visited_nodes = NULL;
    }
}