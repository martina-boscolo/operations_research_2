#include "utilities.h"

//--- instance utilities ---

void build_instance(instance *inst) {

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

}

void random_instance_generator(instance *inst) {

    if (inst->verbose >= 50) {
        printf("Creating random instance:\n\n");
    }

    sprintf(inst->name, "random_n%d_s%d", inst->nnodes, inst->seed);

    // set random seed
    srand(inst->seed);

    // allocate memory for nodes' coordinate
    inst->coord = (coordinate*) malloc(inst->nnodes * sizeof(coordinate));

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
            inst->coord = (coordinate *)malloc(inst->nnodes * sizeof(coordinate));
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

void free_instance(instance *inst) {

    free(inst->coord);

}

//--- solution utilities ---

void plot_solution(instance inst, solution sol) {

    //use gnuplot to print the solution
    FILE *gnuplot = open_plot();

    // in a file
    char filename[50];
    sprintf(filename, "%s_%s", inst.name, sol.method);
    plot_in_file(gnuplot, filename);

    // specify the customization
    add_plot_customization(gnuplot, "plot '-' using 1:2 w linespoints pt 7"); // notitle with lines");

    // plot edges
    for(int i=0; i<inst.nnodes; i++) {

        coordinate first = inst.coord[sol.visited_nodes[i]];
        coordinate second = inst.coord[sol.visited_nodes[i+1]];
        
        plot_edge(gnuplot, first, second);

    }

    input_end(gnuplot);

    free_plot(gnuplot);

}

void free_solution(solution *sol) {

    free(sol->visited_nodes);

}

//--- main utilities ---

void print_error(const char *err) {

    printf("Error: %s", err);
    fflush(NULL);
    exit(1);

}

void parse_command_line(int argc, const char *argv[], instance *inst, solution *sol){

    // set default values
    inst->nnodes = DEFAULT_NNODES;
    inst->coord = NULL;
    inst->seed = DEFAULT_SEED;
    inst->timelimit = DEFAULT_TIMELIMIT;
    inst->verbose = DEFAULT_VERBOSE;
    inst->input_file[0] = EMPTY_STRING;

    sol->cost = 0;
    sol->visited_nodes = NULL;
    strcpy(sol->method, ORDER);

    // flags
    int need_help = 0;
    int help = 0;

    // parsing
    for (int i = 1; i < argc; i++) {

        if ( strcmp(argv[i],"-file") == 0 ) { strcpy(inst->input_file,argv[++i]); continue; }       // input file
        if (strcmp(argv[i], "-n") == 0) { inst->nnodes = atoi(argv[++i]); continue; }               // number of nodes
        if (strcmp(argv[i], "-seed") == 0) { inst->seed = atoi(argv[++i]); continue; }              // random seed
        if (strcmp(argv[i], "-timelimit") == 0) { inst->timelimit = atoi(argv[++i]); continue; }    // time limit
        if (strcmp(argv[i], "-verbose") == 0) { inst->verbose = atoi(argv[++i]); continue; }        // verbosity level
        if (strcmp(argv[i], "-method") == 0) { strcpy(sol->method,argv[++i]); continue; }
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

        printf("Instance metadata:\n\n");

        printf("Nnodes: %d\n", inst->nnodes);
        printf("Seed: %d\n", inst->seed);
        printf("Timelimit: %d\n", inst->timelimit); 
        printf("Verbose: %d\n", inst->verbose);
        printf("Input file %s\n", inst->input_file); 

        printf("\n");

        printf("Solution metadata:\n\n");

        printf("Method: %s\n", sol->method);

        printf("\n\n");

    }

}

//--- various utilities ---

double random01() { 

    return ((double) rand() / RAND_MAX); 

}