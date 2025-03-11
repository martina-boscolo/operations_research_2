#ifndef UTILITIES_INSTANCE_H
#define UTILITIES_INSTANCE_H

#include "tsp.h"
#include "utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum value for x and y coordinates for a random instance
#define MAX_XCOORD 10000
#define MAX_YCOORD 10000

/**
 * Initialize the instance with default values.
 * Note: this function doesn't allocate the memory.
 * 
 * @param inst The instance pointer
 */
void initialize_instance(instance *inst);

/**
 * Instantiate a TSP problem 
 * 
 * @param inst The instance pointer of the problem
 */
void build_instance(instance *inst);

/**
 * Create a random instance
 * 
 * @param inst The instance pointer of the problem
 */
void random_instance_generator(instance *inst);

/**
 * Parses a TSPLIB format file to extract only node coordinates.
 * Assumes a standard TSPLIB format where DIMENSION appears before NODE_COORD_SECTION
 * 
 * @param filename Path to the TSPLIB format file to parse
 * @param inst The instance pointer of the problem
 */
void basic_TSPLIB_parser(const char *filename, instance *inst);

/**
 * Give a name to the instance:
 * - if the data are taken from a file the name would be the same of the file
 * - if the data are generated randomly the name form is "random_nnodes_seed"
 * 
 * @param inst The instance pointer of the problem
 */
void name_instance(instance *inst);

/**
 * Compute the distance between every node to every node in the instance
 * 
 * @param inst The instance pointer of the problem
 */
void compute_all_costs(instance *inst);

/**
 * Compute the distance between two nodes wrt the given instance
 * 
 * @param i The first node
 * @param j The second node
 * @param inst The instance pointer of the problem
 * 
 * @return The distance between the two nodes
 */
double cost(int i, int j, instance *inst);

/**
 * Check if the given solution is better than the curren one,
 * if so update best solution of the instance
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void update_best_sol(instance *inst, solution *sol);

/**
 * Print the hyperparameter of the instance
 * 
 * @param inst The instance pointer of the problem
 */
void print_instance(instance *inst);

/**
 * Allocates the memory for the most space-consuming attributes of instance. 
 * Note: this function also initialize the solution
 * 
 * @param inst The instance pointer of the problem
 */
void allocate_instance(instance *inst);

/**
 * Deallocates the most space-consuming attributes of instance from memory
 * 
 * @param inst The instance pointer of the problem
 */
void free_instance(instance *inst);

#endif //UTILITIES_INSTANCE_H