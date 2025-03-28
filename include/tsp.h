#ifndef TSP_H
#define TSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// default values for the instance
#define DEFAULT_NNODES 150
#define DEFAULT_SEED 1
#define DEFAULT_TIMELIMIT 3600.0
#define DEFAULT_VERBOSE 50
#define EMPTY_STRING '\0'

// value limits
#define MIN_NNODES 5

// verbose levels
#define ONLY_INCUMBMENT 10 
#define GOOD 50
#define DEBUG 100

typedef struct {

    double x;                   // x coordinate
    double y;                   // y coordinate

} coordinate;

typedef struct {

    double cost;                // cost of the solution
    int *visited_nodes;         // sequence of visited nodes NOTE: to complete the cycle first and last nodes must be the same node
    char method[30];            // name of method to compute the solution

} solution;

typedef struct {

    int nnodes;                 // how many nodes the graph has
    coordinate *coord;          // (x,y) coordinate of the nodes
    double *costs;              // array of distances between nodes
    solution *best_solution;    // best current solution

    char name[50];              // name of instance
    int seed;                   // random seed used to generate random instance
    char input_file[1000];      // input file 
    char asked_method[30];      // method to compute the solution
    int param;                  // parameter for the method

    int verbose;                // printing level
    double timelimit;           // numer of seconds to find the solution, if < 0 means no time limit
    time_t t_start;             // initial time

} instance;

#endif //TSP_H