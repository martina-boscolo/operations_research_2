#include "utilities.h"

//--- instance utilities ---

void build_instance(instance *inst) {

    name_instance(inst);

    if (inst->input_file[0] != EMPTY_STRING) {

        // Using input file
        if (inst->verbose >= 50) {
            printf("Input file provided.\n\n");
        }

        basic_TSPLIB_parser(inst->input_file, inst);

    } else {

        // Create a random instance
        if (inst->verbose >= 50) {
            printf("No input file provided. Using random instance\n\n");
        }

        random_instance_generator(inst);

    }

    // Compute all edges' cost
    compute_all_costs(inst);

}

void random_instance_generator(instance *inst) {

    allocate_instance(inst);

    if (inst->verbose >= 50) {
        printf("Creating random instance:\n\n");
    }

    // set random seed
    srand(inst->seed);

    // generate random coordinate for each node
    for(int i=0; i<inst->nnodes; i++) {

        inst->coord[i].x = random01() * MAX_XCOORD;
        inst->coord[i].y = random01() * MAX_YCOORD;

        if (inst->verbose >= 50) {
            printf("Node %d \t x %lf,\ty %lf\n", i, inst->coord[i].x, inst->coord[i].y);
        }

    }

    if (inst->verbose >= 50) {
        printf("\n\n");
    }

}

void basic_TSPLIB_parser(const char *filename, instance *inst) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    char line[256]; // To store a line of the file
    inst->nnodes = 0; // Safety measure: avoid using uninitialized value 
    inst->coord = NULL; // Safety measure: avoid using uninitialized value
    
    while (fgets(line, sizeof(line), file)) {
        // Find the dimension
        if (strstr(line, "DIMENSION")) {
            sscanf(line, "DIMENSION : %d", &inst->nnodes);
            
            // Allocate memory after finding dimension
            allocate_instance(inst);
            if (!inst->coord) {
                perror("Memory allocation failed");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
        
        // Read all coordinates
        if (strstr(line, "NODE_COORD_SECTION")) {
            int index = 0;
            while (index < inst->nnodes && fgets(line, sizeof(line), file)) {
                if (strstr(line, "EOF")) break;
                
                int temp_id;
                sscanf(line, "%d %lf %lf", &temp_id, &inst->coord[index].x, &inst->coord[index].y);
                index++;
            }
            break; // Exit the main loop once we've read all coordinates
        }
    }
    
    fclose(file);
}

void name_instance(instance *inst) {

    if (inst->input_file[0] != EMPTY_STRING) {

        // file instance
        char *bar = strrchr(inst->input_file, '/');
        char *point = strrchr(inst->input_file, '.');

        int bar_pos = -1;
        int point_pos = strlen(inst->input_file);

        if (bar) {
            bar_pos = bar - inst->input_file;
        }
        if (point) {
            bar_pos = bar - inst->input_file;
        }

        strncpy(inst->name, inst->input_file+bar_pos+1, point_pos-bar_pos);

    } else {
        
        // random instance
        sprintf(inst->name, "random_n%d_s%d", inst->nnodes, inst->seed);

    }

}

void compute_all_costs(instance *inst)
{
    for (int i = 0; i < inst->nnodes; i++)
    {
        for (int j = 0; j < inst->nnodes; j++)
        {
            if (i == j)
            {
                inst->costs[i * inst->nnodes + j] = INFINITY; // Self-to-self distance set to INF
            }
            else
            {
                inst->costs[i * inst->nnodes + j] = dist(inst->coord[i], inst->coord[j]);
            }
        }
    }
}

double cost(int i, int j, instance *inst)
{
    return inst->costs[i * inst->nnodes + j];
}

void update_best_sol(instance *inst, solution *sol)
{
    if (sol->cost < inst->best_solution->cost)
    {
        inst->best_solution->cost = sol->cost;
        strcpy(inst->best_solution->method, sol->method);
        memcpy(inst->best_solution->visited_nodes, sol->visited_nodes, (inst->nnodes + 1) * sizeof(int));
    }
}

void print_instance(instance *inst) {

    printf("Name: %s\n", inst->name);
    printf("Seed: %d\n", inst->seed);
    printf("Input file %s\n", inst->input_file);
    printf("Nnodes: %d\n", inst->nnodes);
    printf("Asked method: %s\n", inst->asked_method);

    printf("\n");

    printf("Timelimit: %lf\n", inst->timelimit); 
    printf("Verbose: %d\n", inst->verbose); 

    printf("\n");

    if (inst->coord != NULL) {

        printf("Nodes' coordinates:\n");

        for (int i=0; i<inst->nnodes; i++) {
            printf("Node %d: \t x %lf,\ty %lf\n", i, inst->coord[i].x, inst->coord[i].y);
        }
        
        printf("\n");

    }

    if (inst->costs != NULL) {

        printf("Edges' cost:\n");

        for(int i=0; i<inst->nnodes; i++) {
            for(int j=i+1; j<inst->nnodes; j++) {
                printf("Edge[%d, %d]: %lf", i, j, inst->costs[i*inst->nnodes + j]);
            }
        }

        printf("\n");

    }

    if (inst->best_solution != NULL) {
        print_solution(inst->best_solution, inst->nnodes);

        printf("\n");
    }

}

void allocate_instance(instance *inst) {

    // allocate memory for nodes' coordinate
    inst->coord = (coordinate*) calloc(inst->nnodes, sizeof(coordinate));

    // allocate memory for edges' cost
    inst->costs = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));

    // allocate memory for the best solution
    inst->best_solution = (solution *) malloc(sizeof(solution));
    allocate_solution(inst->best_solution, inst->nnodes);

}

void free_instance(instance *inst) {

    if (inst->coord != NULL) {
        free(inst->coord);
        inst->coord = NULL;
    }

    if (inst->costs != NULL) {
        free(inst->costs);
        inst->costs = NULL;
    }
    
    if (inst->best_solution != NULL) {
        free_solution(inst->best_solution);
        inst->best_solution = NULL;
    }

}

//--- solution utilities ---

void initialize_solution(int *visited_nodes, int nnodes) {
    for (int i = 0; i < nnodes; i++) {
        visited_nodes[i] = i;
    }
    visited_nodes[nnodes] = 0;
}

void solve_with_method(instance *inst, solution *sol) {
    
    allocate_solution(sol, inst->nnodes);

    if (strcmp(inst->asked_method, BASE) == 0) {
        
        printf("Solving with BASE method.\n");
        make_base_solution(inst, sol);

    } else if (strcmp(inst->asked_method, NEAREST_NEIGHBOR) == 0) {

        printf("Solving with Nearest Neighbor method.\n");
        ms_2opt_nn_main(inst, sol); 

    } else {
        fprintf(stderr, "Error: Unknown method '%s'.\nPlease, select valid method\n", sol->method);
        printf("Valid methods are:\n-%s\n-%s\n", BASE, NEAREST_NEIGHBOR);
        exit(EXIT_FAILURE);
    }
    
}

int validate_node_visits(instance *inst, solution *sol)
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
            return 0; // Out-of-range node index
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
            return 0; // A node was missed
        }
    }

    // Ensure the tour closes properly
    if (sol->visited_nodes[inst->nnodes] != sol->visited_nodes[0])
    {
        fprintf(stderr, "Ending node different from starting node\n");
        free(visited);
        return 0; // Cycle not closed properly
    }

    free(visited);
    return 1; 
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

    sol->cost = INF;
    sol->visited_nodes = (int *) calloc((nnodes + 1), sizeof(int));

}

void free_solution(solution *sol) {

    if (sol->visited_nodes != NULL) {
        free(sol->visited_nodes);
        sol->visited_nodes = NULL;
    }

}

//--- main utilities ---

void print_error(const char *err) {

    fprintf(stderr,"Error: %s", err);
    fflush(NULL);
    exit(EXIT_FAILURE);

}

void parse_command_line(int argc, const char *argv[], instance *inst) {

    // set default values
    inst->nnodes = DEFAULT_NNODES;
    inst->coord = NULL;
    inst->costs = NULL;
    inst->best_solution = NULL;
    inst->name[0] = EMPTY_STRING;
    inst->seed = DEFAULT_SEED;
    inst->timelimit = DEFAULT_TIMELIMIT;
    inst->verbose = DEFAULT_VERBOSE;
    inst->input_file[0] = EMPTY_STRING;

    strcpy(inst->asked_method, BASE);

    // flags
    int need_help = 0;
    int help = 0;

    // parsing
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-file") == 0 ) { strcpy(inst->input_file,argv[++i]); continue; }       // input file
        if (strcmp(argv[i], "-n") == 0) { inst->nnodes = atoi(argv[++i]); continue; }               // number of nodes
        if (strcmp(argv[i], "-seed") == 0) { inst->seed = atoi(argv[++i]); continue; }              // random seed
        if (strcmp(argv[i], "-timelimit") == 0) { inst->timelimit = atoi(argv[++i]); continue; }    // time limit
        if (strcmp(argv[i], "-verbose") == 0) { inst->verbose = atoi(argv[++i]); continue; }        // verbosity level
        if (strcmp(argv[i], "-method") == 0) { strcpy(inst->asked_method,argv[++i]); continue; }
        if (strcmp(argv[i], "--help") == 0) { help = 1; continue; } 

        // if there is an unknown command
        need_help = 1;

    }

    // if something in the command line was written wrong
    if (need_help) {

        printf("Type \"%s --help\" to see available commands\n", argv[0]);

        exit(1);

    }

    // asked to see the available commands
    if (help) {

        printf("-file <file's path>       To pass the problem's path\n");
        printf("-seed <seed>              The seed for random generation\n");
        printf("-timelimit <time>         The time limit in seconds\n");
        printf("-verbose <level>          The verbosity level of the debugging printing\n");
        printf("-method <method>          The method used to solve the problem\n");

        exit(0);

    }

    // if requested print the result of parsing
    if (inst->verbose >= 50) {

        print_instance(inst);

        printf("\n\n");

    }

}

//--- various utilities ---

time_t seconds(void) {

    time_t curr_time;
    time(&curr_time);

    return curr_time;

}

double get_elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER frequency) {
    return (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
}

double random01(void)
{
    return ((double) rand() / RAND_MAX);
}

double dist(coordinate point1, coordinate point2) {

    return sqrt(pow((point1.x-point2.x),2) + pow((point1.y-point2.y),2));

}
