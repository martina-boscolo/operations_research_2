#include "utilities.h"

//--- main utilities ---

// Print "Error: " followed by the error message and exit the program.
void print_error(const char *err) {

    if (err == NULL || strlen(err) == 0) {

        printf("\n\n ERROR: Unknown error occurred. \n\n");

    } else {

        printf("\n\n ERROR: %s \n\n", err); 

    }

	fflush(NULL); 
	exit(1); 

}

// Parse the input from the command line and fill the instance structure with the parsed values.
void parse_command_line(const int argc, const char *argv[], instance *inst) {

    // Set default values
    initialize_instance(inst);

    // Flags
    int need_help = 0;
    int help = 0;

    // Parse the command line arguments
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-file") == 0 || strcmp(argv[i], "-f") == 0)                                        // input file
            { strncpy_s(inst->input_file, FILE_NAME_LEN, argv[++i], _TRUNCATE); continue; }
        if (strcmp(argv[i], "-n") == 0)                                                                         // number of nodes
            { inst->nnodes = atoi(argv[++i]); if (inst->nnodes < MIN_NNODES) { need_help = 1; break; } continue; }          
        if (strcmp(argv[i], "-seed") == 0)                                                                      // random seed
            { inst->seed = atoi(argv[++i]); continue; }              
        if (strcmp(argv[i], "-timelimit") == 0)                                                                 // time limit
            { inst->timelimit = atof(argv[++i]); if (inst->timelimit < 1.0) { need_help = 1; break; } continue; }    
        if (strcmp(argv[i], "-verbose") == 0)                                                                   // verbosity level
            { inst->verbose = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-method") == 0)                                                                    // method to solve tsp
            { strncpy_s(inst->asked_method, METH_NAME_LEN, argv[++i], _TRUNCATE); continue; }
        if (strcmp(argv[i], "-param1") == 0)                                                                    // first parameter for the method          
            { inst->param1 = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-param2") == 0)                                                                    // second parameter for the method          
            { inst->param2 = atoi(argv[++i]); continue; }  
        if (strcmp(argv[i], "-param3") == 0)                                                                    // third parameter for the method
            { inst->param3 = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "--help") == 0)                                                                     // help
            { help = 1; continue; } 

        // If there is an unknown command
        need_help = 1;
        break;

    }

    // If something in the command line was written wrong
    if (need_help) {

        printf("Type \"%s --help\" to see available commands\n", argv[0]);

        exit(1);

    }

    // If asked, see the available commands
    if (help) {

        printf("-file|-f <file's path>    To pass the problem's path\n");
        printf("-n <nnodes>               The number of nodes in the graph, must be at least %d\n", MIN_NNODES);
        printf("-seed <seed>              The seed for random generation\n");
        printf("-timelimit <time>         The time limit in seconds, must be positive\n");
        printf("-verbose <level>          The verbosity level of the debugging printing\n");
        printf("-method <method>          The method used to solve the problem\n");
        printf("                          Available methods: %s, %s, %s, %s, %s, %s, %s, %s, %s", NEAREST_NEIGHBOR, 
            MULTI_START_NN, EXTRA_MILEAGE, VNS, TABU_SEARCH, BENDERS, BRANCH_AND_CUT, HARD_FIXING, LOCAL_BRANCHING);
        printf("-param1 <param>           The first parameter for the method\n");
        printf("-param2 <param>           The second parameter for the method\n");
        printf("-param3 <param>           The third parameter for the method\n");

        exit(0);

    }

    // If asked, print the result of parsing
    if (inst->verbose >= GOOD) {

        print_instance(inst);

        printf("\n\n");

    }

}

//--- various utilities ---

// Compute the time passed in seconds from starting time to current time.
double get_elapsed_time(const double start) {

    return get_time_in_milliseconds() - start;

}

// Get the current time in seconds using high-resolution performance counter.
double get_time_in_milliseconds() {

    static LARGE_INTEGER frequency;
    static BOOL initialized = FALSE;
    LARGE_INTEGER now;

    if (!initialized) {

        QueryPerformanceFrequency(&frequency);
        initialized = TRUE;

    }

    QueryPerformanceCounter(&now);
    return (double)now.QuadPart / (double)frequency.QuadPart;

}

// Draw a random value between 0 and 1.
double random01(void) {

    return ((double) rand() / RAND_MAX);

}

// Compute the Euclidean distance between two points.
double dist(const coordinate point1, const coordinate point2) {

    double deltax = point1.x-point2.x;
    double deltay = point1.y-point2.y;

    return sqrt(deltax * deltax + deltay * deltay);

}
