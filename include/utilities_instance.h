#ifndef UTILITIES_INSTANCE_H
#define UTILITIES_INSTANCE_H

#include "tsp.h"
#include "utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize the instance with default values.
 * NOTE: This function doesn't allocate the memory.
 * 
 * @param inst The instance to initialize (output)
 */
void initialize_instance(instance *inst);

/**
 * Instantiate a TSP problem: 
 * - it allocates the memory for the instance;
 * - if an input file is provided, it parses the file and fills the instance with the data;
 *   otherwise it generates a random instance.
 * 
 * @param inst The instance to fill with the data (input/output)
 */
void build_instance(instance *inst);

/**
 * Generate a random instance.
 * NOTE: This function allocates the memory for the instance.
 * 
 * @param inst The instance to fill with the data (input/output)
 */
void random_instance_generator(instance *inst);

/**
 * Parse a TSPLIB format file to extract only node coordinates.
 * NOTE: This function assumes a standard TSPLIB format where DIMENSION appears before NODE_COORD_SECTION.
 * NOTE: This function assumes that the input file is already set in the instance structure.
 * NOTE: This function allocates the memory for the instance.
 * 
 * @param inst The instance to fill with the data (input/output)
 */
void basic_TSPLIB_parser(instance *inst);

/**
 * Give a name to the instance:
 * - if the data are taken from a file the name would be the same of the file;
 * - if the data are generated randomly the name form is "random_nnodes_seed".
 * 
 * @param inst The instance to name (input/output)
 */
void name_instance(instance *inst);

/**
 * Compute the distance between every node to every node in the instance.
 * 
 * @param inst The instance to compute the costs for (input/output)
 */
void compute_all_costs(instance *inst);

/**
 * Return the distance between two nodes wrt the given instance.
 * NOTE: This function assumes that the costs have already been computed and that i and j are valid indices.
 * 
 * @param i The first node (input)
 * @param j The second node (input)
 * @param inst The instance containing the costs (input)
 * 
 * @return The distance between the two nodes
 */
double cost(const int i, const int j, const instance *inst);

/**
 * Print the instance.
 * 
 * @param inst The instance to print (input)
 */
void print_instance(const instance *inst);

/**
 * Allocate the memory for the most space-consuming attributes of instance. 
 * NOTE: This function also initialize the solution.
 * 
 * @param inst The instance to allocate (input/output)
 */
void allocate_instance(instance *inst);

/**
 * Free the most space-consuming attributes of instance from memory.
 * 
 * @param inst The instance to free (output)
 */
void free_instance(instance *inst);

#endif //UTILITIES_INSTANCE_H