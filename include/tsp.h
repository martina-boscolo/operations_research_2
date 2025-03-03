#ifndef TSP_H
#define TSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// default values for the instance
#define DEFAULT_NNODES 150
#define DEFAULT_SEED 1
#define DEFAULT_TIMELIMIT -1 
#define DEFAULT_VERBOSE 50
#define EMPTY_STRING '\0'
// default value for the solution
#define ORDER "ORDER"

typedef struct {

    // x coordinate
    double x;
    // y coordinate
    double y;

} coordinate;

typedef struct {

    // how many nodes the graph has
    int nnodes;
    // (x,y) coordinate of the nodes
    coordinate *coord;

    // name of instance
    char name[10];
    // random seed used to generate random instance
    int seed;
    // numer of seconds to find the solution, if < 0 means no time limit
    int timelimit;
    // how much print: 50 = everything
    // todo: define the level of verbose 
    int verbose;
    // input file 
    char input_file[1000];

} instance;

typedef struct {

    // cost if the solution
    double cost;
    // sequence of visited nodes
    // to complete the cycle first and last nodes must be the same node
    int *visited_nodes;
    // name of method to compute the solution
    char method[30];

} solution;

#endif //TSP_H