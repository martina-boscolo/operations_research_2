#ifndef TSP_H
#define TSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Default values for the instance
#define DEFAULT_NNODES 150
#define DEFAULT_SEED 1
#define DEFAULT_TIMELIMIT 600.0
#define DEFAULT_VERBOSE 50
#define EMPTY_STRING '\0'
#define DEFAULT_PARAMETER -1

// Value limit
#define MIN_NNODES 10

// Verbose levels
#define ONLY_INCUMBENT 10 
#define LOW 30
#define GOOD 50
#define DEBUG_V 100

// Useful constants
#define EPSILON 1e-5
#define METH_NAME_LEN 30
#define INST_NAME_LEN 50
#define FILE_NAME_LEN 1000

// Structure to hold coordinate values
typedef struct {

    double x;                           // x coordinate
    double y;                           // y coordinate

} coordinate;

// Structure to hold solution data
typedef struct {

    double cost;                        // Cost of the solution
    int *visited_nodes;                 // Sequence of visited nodes 
                                        // NOTE: to complete the cycle first and last nodes must be the same node
    char method[METH_NAME_LEN];         // Name of method used to compute the solution

} solution;

// Structure to hold problem data and general informations
typedef struct {

    int nnodes;                         // How many nodes the graph has
    coordinate *coord;                  // (x,y) coordinate of the nodes
    double *costs;                      // Array of distances between nodes
    solution *best_solution;            // Best current solution

    char name[INST_NAME_LEN];           // Name of instance
    int seed;                           // Random seed
    char input_file[FILE_NAME_LEN];     // Input file 
    char asked_method[METH_NAME_LEN];   // Method to compute the solution
    int param1;                         // First parameter used by the method
    int param2;                         // Second parameter used by the method
    int param3;                         // Third parameter used by the method

    int verbose;                        // Printing level
    double timelimit;                   // Time in seconds to find the solution, if < 0 means no time limit
    double t_start;                     // Initial time
    int ncols;                          // Number of columns in the cplex model

} instance;

#endif //TSP_H