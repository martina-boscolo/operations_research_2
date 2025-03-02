#include "utilities.h"

//--- instance utilities ---

void random_instance(instance *inst) {

    if (inst->verbose >= 50) {
        printf("Creating random instance:\n\n");
    }

    // set random seed
    srand(inst->seed);

    // allocate memory for nodes' coordinate
    inst->coord = (coordinate*) malloc(inst->nnodes * sizeof(coordinate));

    // generate random coordinate for each node
    for(int i=0; i<inst->nnodes; i++) {

        inst->coord[i].x = rand() % MAX_XCOORD;
        inst->coord[i].y = rand() % MAX_YCOORD;

        if (inst->verbose >= 50) {
            printf("Node %d \t x %lf,\ty %lf\n", i, inst->coord[i].x, inst->coord[i].y);
        }

    }

    if (inst->verbose >= 50) {
        printf("\n\n");
    }

}

void free_instance(instance *inst) {

    free(inst->coord);

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

//--- solution utilities ---

void plot_solution(instance inst, solution sol) {

    //use gnuplot to print the solution
    FILE *gnuplot = open_plot();

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

void parse_command_line(int argc, const char *argv[], instance *inst){

    // set default values
    inst->nnodes = DEFAULT_NNODES;
    inst->seed = DEFAULT_SEED;
    inst->timelimit = DEFAULT_TIMELIMIT;
    inst->verbose = DEFAULT_VERBOSE;
    inst->input_file[0] = EMPTY_STRING;

    // flags
    int need_help = 0;
    int help = 0;

    // parsing
    for (int i = 1; i < argc; i++) {

        if ( strcmp(argv[i],"-file") == 0 ) { strcpy(inst->input_file,argv[++i]); continue; } 			// input fil
        if (strcmp(argv[i], "-n") == 0) { inst->nnodes = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-seed") == 0) { inst->seed = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-timelimit") == 0) { inst->timelimit = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-verbose") == 0) { inst->verbose = atoi(argv[++i]); continue; }
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

        printf("\n\n");

    }

}