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

    // random seed used to generate random instance
    int seed;
    // numer of seconds to find the solution, if < 0 means no time limit
    int timelimit;
    // how much print: 50 = everything
    // todo: define the level of verbose 
    int verbose;

} instance;

typedef struct {

    // sequence of visited nodes
    // to complete the cycle first and last nodes must be the same node
    int *visited_nodes;

} solution;

#endif