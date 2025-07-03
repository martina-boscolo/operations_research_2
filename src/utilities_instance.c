#include "utilities_instance.h"

// Initialize the instance with default values.
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

// Instantiate a TSP problem
void build_instance(instance *inst) {

    name_instance(inst);

    if (inst->input_file[0] != EMPTY_STRING) {

        // Using input file
        if (inst->verbose >= GOOD) {

            printf("Input file provided.\n\n");

        }

        basic_TSPLIB_parser(inst);

    } else {

        // Create a random instance
        if (inst->verbose >= GOOD) {

            printf("No input file provided. Using random instance\n\n");

        }

        random_instance_generator(inst);

    }

    // Compute all edges' cost
    compute_all_costs(inst);

}

// Generate a random instance
void random_instance_generator(instance *inst) {

    allocate_instance(inst);

    if (inst->verbose >= GOOD) {

        printf("Creating random instance:\n\n");

    }

    // Set random seed
    srand(inst->seed);

    // Generate random coordinate for each node
    for(int i=0; i<inst->nnodes; i++) {

        inst->coord[i].x = random01() * MAX_XCOORD;
        inst->coord[i].y = random01() * MAX_YCOORD;

        // If asked, print the coordinates
        if (inst->verbose >= GOOD) {

            printf("Node %5d, x %10.6lf, y %10.6lf\n", i, inst->coord[i].x, inst->coord[i].y);

        }

    }

    if (inst->verbose >= GOOD) {

        printf("\n\n");

    }

}

// Parse a TSPLIB format file to extract only node coordinates
void basic_TSPLIB_parser(instance *inst) {
    
    FILE *file;
    if (fopen_s(&file, inst->input_file, "r")) print_error("basic_TSPLIB_parser(): Cannot open file");
    
    char line[256]; // To store a line of the file
    inst->nnodes = 0; // Safety measure: avoid using uninitialized value 
    inst->coord = NULL; // Safety measure: avoid using uninitialized value
    
    while (fgets(line, sizeof(line), file)) {

        // Find the dimension
        if (strstr(line, "DIMENSION")) {

            sscanf_s(line, "DIMENSION : %d", &inst->nnodes);
            
            // Allocate memory after finding dimension
            allocate_instance(inst);

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

// Give a name to the instance
void name_instance(instance *inst) {

    if (inst->input_file[0] != EMPTY_STRING) { // File instance

        // Search for the name of the file without path and extension
        char *fslash = strrchr(inst->input_file, '/');    // Forward slash for Unix paths
        char *bslash = strrchr(inst->input_file, '\\');   // Backslash for Windows paths
        char *point = strrchr(inst->input_file, '.');

        __int64 slash_pos = -1;
        __int64 point_pos = strlen(inst->input_file);

        // Use the last slash/backslash found (whichever is furthest in the string)
        if (fslash && bslash) {

            slash_pos = (fslash > bslash) ? (fslash - inst->input_file) : (bslash - inst->input_file);
        
        } else if (fslash) {

            slash_pos = fslash - inst->input_file;
        
        } else if (bslash) {

            slash_pos = bslash - inst->input_file;
        
        }

        if (point) {

            point_pos = point - inst->input_file;

        }

        strncpy_s(inst->name, INST_NAME_LEN, inst->input_file + slash_pos + 1, point_pos - slash_pos - 1);

    } else { // Random instance

        sprintf_s(inst->name, INST_NAME_LEN, "random_n%d_s%d", inst->nnodes, inst->seed);

    }

}

// Compute the distance between every node to every node in the instance
void compute_all_costs(instance *inst) {

    for (int i = 0; i < inst->nnodes; i++) {

        for (int j = 0; j < inst->nnodes; j++) {

            if (i == j) { // Self-to-self distance set to INF

                inst->costs[i * inst->nnodes + j] = INFINITY;

            } else {

                inst->costs[i * inst->nnodes + j] = dist(inst->coord[i], inst->coord[j]);

            }

        }

    }

}

// Return the distance between two nodes w.r.t. the given instance
double cost(const int i, const int j, const instance *inst) {

    return inst->costs[i * inst->nnodes + j];

}

void print_instance(const instance *inst) {

    printf("Name: %s\n", inst->name);
    printf("Seed: %5d\n", inst->seed);
    printf("Input file %s\n", inst->input_file);
    printf("Nnodes: %5d\n", inst->nnodes);
    printf("Asked method: %s\n", inst->asked_method);

    printf("\n");

    printf("Timelimit: %.5f\n", inst->timelimit); 
    printf("Verbose: %d\n", inst->verbose); 

    printf("\n");

    if (inst->coord != NULL) {

        printf("Nodes' coordinates:\n");

        for (int i=0; i<inst->nnodes; i++) {

            printf("Node %5d, x %10.6lf, y %10.6lf\n", i, inst->coord[i].x, inst->coord[i].y);

        }
        
        printf("\n");

    }

    if (inst->costs != NULL && inst->verbose >= DEBUG_V) {

        printf("Edges' cost:\n");

        for(int i=0; i<inst->nnodes; i++) {

            for(int j=i+1; j<inst->nnodes; j++) {

                printf("Edge[%5d, %5d]: %10.6lf\t\t", i, j, inst->costs[i*inst->nnodes + j]);

            }

        }

        printf("\n");

    }

}

// Allocate the memory for the most space-consuming attributes of instance
void allocate_instance(instance *inst) {

    // Allocate memory for nodes' coordinate
    inst->coord = (coordinate*) calloc(inst->nnodes, sizeof(coordinate));

    // Allocate memory for edges' cost
    inst->costs = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));

    // Allocate memory for the best solution
    inst->best_solution = (solution *) malloc(sizeof(solution));

    // Check if memory allocation was successful
    if (inst->coord == NULL || inst->costs == NULL || inst->best_solution == NULL) {

        print_error("allocate_instance(): Cannot allocate memory");

    }

    // Initialize and allocate the best solution
    initialize_solution(inst->best_solution);
    allocate_solution(inst->best_solution, inst->nnodes);

}

// Free the most space-consuming attributes of instance from memory
void free_instance(instance *inst) {

    // Deallocate memory for nodes' coordinate
    if (inst->coord != NULL) {

        free(inst->coord);
        inst->coord = NULL;

    }

    // Deallocate memory for edges' cost
    if (inst->costs != NULL) {

        free(inst->costs);
        inst->costs = NULL;

    }
    
    // Deallocate memory for the best solution
    if (inst->best_solution != NULL) {

        free_solution(inst->best_solution);
        inst->best_solution = NULL;

    }

}