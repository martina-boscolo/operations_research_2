#include "utilities_solution.h"

void initialize_solution(solution *sol) {

    sol->cost = INFINITY;
    sol->visited_nodes = NULL;
    sol->method[0] = EMPTY_STRING;

}

void initialize_tour(int *visited_nodes, int nnodes) {
    for (int i = 0; i < nnodes; i++) {
        visited_nodes[i] = i;
    }
    visited_nodes[nnodes] = 0;
}

void solve_with_method(instance *inst, solution *sol) {
    
    initialize_solution(sol);
    allocate_solution(sol, inst->nnodes);
    mkdir("./results", 0777);

    if (strcmp(inst->asked_method, NEAREST_NEIGHBOR) == 0) {

        printf("Solving with Nearest Neighbor method.\n");
        ms_2opt_nn_main(inst, sol); 

    } else if (strcmp(inst->asked_method, VNS) == 0) {

        printf("Solving with VNS method.\n");
        vns(inst, sol, VNS_REPS);
        
    } else if (strcmp(inst->asked_method, TABU_SEARCH) == 0) {

        printf("Solving with TABU_SEARCH method.\n");
        tabu_search(inst, sol);
        
    } else if (strcmp(inst->asked_method, COMPARE) == 0) {
        
        printf("Comparing available algorithms.\n");
        compare(inst);
        
    } else {
        fprintf(stderr, "Error: Unknown method '%s'.\nPlease, select valid method\n", sol->method);
        printf("Valid methods are:\n-%s\n-%s\n-%s\n", NEAREST_NEIGHBOR, VNS, TABU_SEARCH);
        exit(EXIT_FAILURE);
    }
    
}

bool validate_node_visits(instance *inst, solution *sol)
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

double compute_solution_cost(instance *inst, solution *sol)
{
    double computed_cost = 0;
    for (int i = 0; i < inst->nnodes; i++)
    {
        computed_cost += cost(sol->visited_nodes[i], sol->visited_nodes[i + 1], inst);
    }
    return computed_cost;
}

int validate_cost(instance *inst, solution *sol)
{
    double computed_cost = compute_solution_cost(inst, sol);
    if (fabs(computed_cost - sol->cost) > EPSILON)
    { // Floating-point error tolerance
        fprintf(stderr, "Cost mismatch! Expected: %lf, Computed: %lf\n", sol->cost, computed_cost);
        return 0;
    }
    return 1;
}

void check_sol(instance *inst, solution *sol)
{
    if (inst->verbose >= 50)
    { // only for debug
        if (!validate_node_visits(inst, sol))
        {
            exit(EXIT_FAILURE); // problem in the visit of the nodes
        }
        if (!validate_cost(inst, sol))
        {
            exit(EXIT_FAILURE); // mismatch in the cost
        }
        // Solution is valid
    }
    
}

void plot_solution(instance *inst, solution *sol) {

    //use gnuplot to print the solution
    FILE *gnuplot = open_plot();

    // in a file
    char filename[50];
    sprintf(filename, "%s_%s", inst->name, sol->method);
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

void print_solution(solution *sol, int nnodes) {

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

void allocate_solution(solution *sol, int nnodes) {

    free_solution(sol);

    sol->visited_nodes = (int *) calloc((nnodes + 1), sizeof(int));

}

void free_solution(solution *sol) {

    if (sol->visited_nodes != NULL) {
        free(sol->visited_nodes);
        sol->visited_nodes = NULL;
    }

}