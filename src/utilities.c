#include "utilities.h"

//--- main utilities ---

void print_error(const char *err) 
{ 
	printf("\n\n ERROR: %s \n\n", err); 
	fflush(NULL); 
	exit(1); 
}

void parse_command_line(const int argc, const char *argv[], instance *inst) {

    // set default values
    initialize_instance(inst);

    // flags
    int need_help = 0;
    int help = 0;

    // parsing
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-file") == 0 || strcmp(argv[i], "-f") == 0)                                        // input file
            { strncpy_s(inst->input_file, FILE_NAME_LEN, argv[++i], _TRUNCATE); continue; }
        if (strcmp(argv[i], "-n") == 0)                                                                         // number of nodes
            { inst->nnodes = atoi(argv[++i]); if (inst->nnodes < MIN_NNODES) { need_help = 1; } continue; }          
        if (strcmp(argv[i], "-seed") == 0)                                                                      // random seed
            { inst->seed = atoi(argv[++i]); continue; }              
        if (strcmp(argv[i], "-timelimit") == 0)                                                                 // time limit
            { inst->timelimit = atof(argv[++i]); if (inst->timelimit < 1.0) { need_help = 1; } continue; }    
        if (strcmp(argv[i], "-verbose") == 0)                                                                   // verbosity level
            { inst->verbose = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-method") == 0)                                                                    // method to solve tsp
            { strncpy_s(inst->asked_method, METH_NAME_LEN, argv[++i], _TRUNCATE); continue; }
        if (strcmp(argv[i], "-param1") == 0)                                                                    // first parameter for the method          
            {  inst->param1 = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-param2") == 0)                                                                    // second parameter for the method          
            {  inst->param2 = atoi(argv[++i]); continue; }  
        if (strcmp(argv[i], "--help") == 0)                                                                     // help
            { help = 1; continue; } 

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

        printf("-file|-f <file's path>    To pass the problem's path\n");
        printf("-n <nnodes>               The number of nodes in the graph, must be at least %d\n", MIN_NNODES);
        printf("-seed <seed>              The seed for random generation\n");
        printf("-timelimit <time>         The time limit in seconds, must be positive\n");
        printf("-verbose <level>          The verbosity level of the debugging printing\n");
        printf("-method <method>          The method used to solve the problem\n");
        printf("-param1 <param>    The first parameter for the method\n");
        printf("-param2 <param>           The second parameter for the method\n");

        exit(0);

    }

    // if requested print the result of parsing
    if (inst->verbose >= 50) {

        print_instance(inst);

        printf("\n\n");

    }

}

//--- various utilities ---

double get_elapsed_time(const double start) {

    return get_time_in_milliseconds() - start;

}

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

double random01(void)
{
    return ((double) rand() / RAND_MAX);
}

double dist(const coordinate point1, const coordinate point2) {

    double deltax = point1.x-point2.x;
    double deltay = point1.y-point2.y;

    return sqrt(deltax * deltax + deltay * deltay);

}
