#include "utilities.h"

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

double get_elapsed_time(time_t start) {

    // get current time
    time_t curr_time;
    time(&curr_time);

    return difftime(start, curr_time);

}

double random01(void)
{
    return ((double) rand() / RAND_MAX);
}

double dist(coordinate point1, coordinate point2) {

    double deltax = point1.x-point2.x;
    double deltay = point1.y-point2.y;

    return sqrt(deltax * deltax + deltay * deltay);

}
