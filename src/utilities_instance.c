#include "utilities_instance.h"

void initialize_instance(instance *inst) {

    inst->nnodes = DEFAULT_NNODES;
    inst->coord = NULL;
    inst->costs = NULL;
    inst->best_solution = NULL;

    inst->name[0] = EMPTY_STRING;
    inst->seed = DEFAULT_SEED;
    inst->input_file[0] = EMPTY_STRING;
    inst->asked_method[0] = EMPTY_STRING;
    inst->param1 = -1;
    inst->param2 = -1;

    inst->verbose = DEFAULT_VERBOSE;
    inst->timelimit = DEFAULT_TIMELIMIT;
    inst->t_start = get_time_in_milliseconds();
    inst->ncols = -1;

}

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

    if (inst->verbose >= GOOD) {
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
    FILE *file;
    if (fopen_s(&file, filename, "r")) print_error("basic_TSPLIB_parser(): Error opening file");
    
    char line[256]; // To store a line of the file
    inst->nnodes = 0; // Safety measure: avoid using uninitialized value 
    inst->coord = NULL; // Safety measure: avoid using uninitialized value
    
    while (fgets(line, sizeof(line), file)) {
        // Find the dimension
        if (strstr(line, "DIMENSION")) {
            sscanf_s(line, "DIMENSION : %d", &inst->nnodes);
            
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
                sscanf_s(line, "%d %lf %lf", &temp_id, &inst->coord[index].x, &inst->coord[index].y);
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

        // search for the name of the file without path and extension
        char *bar = strrchr(inst->input_file, '/');
        char *point = strrchr(inst->input_file, '.');

        __int64 bar_pos = -1;
        __int64 point_pos = strlen(inst->input_file);

        if (bar) {
            bar_pos = bar - inst->input_file;
        }
        if (point) {
            point_pos = point - inst->input_file;
        }

        strncpy_s(inst->name, INST_NAME_LEN, inst->input_file + bar_pos + 1, point_pos - bar_pos - 1);

    } else {
        
        // random instance
        sprintf_s(inst->name, INST_NAME_LEN, "random_n%d_s%d", inst->nnodes, inst->seed);

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

double cost(const int i, const int j, const instance *inst)
{
    return inst->costs[i * inst->nnodes + j];
}

void print_instance(instance *inst) {

    printf("Name: %s\n", inst->name);
    printf("Seed: %d\n", inst->seed);
    printf("Input file %s\n", inst->input_file);
    printf("Nnodes: %d\n", inst->nnodes);
    printf("Asked method: %s\n", inst->asked_method);

    printf("\n");

    printf("Timelimit: %f\n", inst->timelimit); 
    printf("Verbose: %d\n", inst->verbose); 

    printf("\n");

    if (inst->coord != NULL) {

        printf("Nodes' coordinates:\n");

        for (int i=0; i<inst->nnodes; i++) {
            printf("Node %d: \t x %lf,\ty %lf\n", i, inst->coord[i].x, inst->coord[i].y);
        }
        
        printf("\n");

    }

    if (inst->costs != NULL && inst->verbose >= DEBUG) {

        printf("Edges' cost:\n");

        for(int i=0; i<inst->nnodes; i++) {
            for(int j=i+1; j<inst->nnodes; j++) {
                printf("Edge[%d, %d]: %lf\t\t", i, j, inst->costs[i*inst->nnodes + j]);
            }
        }

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

    if (inst->coord == NULL || inst->costs == NULL || inst->best_solution == NULL) {
        printf("Cannot allocate memory");
        exit(EXIT_FAILURE);
    }

    initialize_solution(inst->best_solution);
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