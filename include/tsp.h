#ifndef TSP_H
#define TSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// default values for the instance
#define DEFAULT_NNODES 150
#define DEFAULT_SEED 1
#define DEFAULT_TIMELIMIT 600.0
#define DEFAULT_VERBOSE 50
#define EMPTY_STRING '\0'

// value limits
#define MIN_NNODES 10

// verbose levels
#define ONLY_INCUMBMENT 10 
#define GOOD 50
#define DEBUG 100

// useful constants
#define EPSILON 1e-5
#define METH_NAME_LEN 30
#define INST_NAME_LEN 50
#define FILE_NAME_LEN 1000

typedef struct {

    double x;                           // x coordinate
    double y;                           // y coordinate

} coordinate;

typedef struct {

    double cost;                        // cost of the solution
    int *visited_nodes;                 // sequence of visited nodes NOTE: to complete the cycle first and last nodes must be the same node
    char method[METH_NAME_LEN];         // name of method to compute the solution

} solution;

typedef struct {

    int nnodes;                         // how many nodes the graph has
    coordinate *coord;                  // (x,y) coordinate of the nodes
    double *costs;                      // array of distances between nodes
    solution *best_solution;            // best current solution

    char name[INST_NAME_LEN];           // name of instance
    int seed;                           // random seed used to generate random instance
    char input_file[FILE_NAME_LEN];     // input file 
    char asked_method[METH_NAME_LEN];   // method to compute the solution
    int param1;                         // parameter for the method
    int param2;                         // second parameter for the method

    int verbose;                        // printing level
    double timelimit;                   // numer of seconds to find the solution, if < 0 means no time limit
    double t_start;                     // initial time
    int ncols;                          // number of columns in the model

} instance;

#endif //TSP_H