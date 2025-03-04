#ifndef TSP_H
#define TSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// default values for the instance
#define DEFAULT_NNODES 150
#define DEFAULT_SEED 1
#define DEFAULT_TIMELIMIT 3600.0
#define DEFAULT_VERBOSE 50
#define EMPTY_STRING '\0'

// default value for the solution
#define ORDER "ORDER"
#define INFINITE_COST 10e38

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
    solution *best_solution;     // best current solution

    char name[10];              // name of instance
    int seed;                   // random seed used to generate random instance
    char input_file[1000];      // input file 

    int verbose;                // printing level  (=10 only incumbent, =20 little output, =50-60 good, =70 verbose, >=100 cplex log)
    double timelimit;           // numer of seconds to find the solution, if < 0 means no time limit
    double t_start;             // initial time

} instance;

#endif //TSP_H