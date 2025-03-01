#ifndef UTILITIES_H
#define UTILITIES_H

#include "tsp.h"
#include "plot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// useful constants
#define EPSILON 1e-5

//--- instance utilities ---

// Maximum value for x and y coordinates
#define MAX_XCOORD 10000
#define MAX_YCOORD 10000

/**
 * Create a random instance
 * 
 * @param inst The instance pointer of the problem
 */
void random_instance(instance *inst);

/**
 * Deallocates the most space-consuming attributes of instance from memory
 * 
 * @param inst The instance pointer of the problem
 */
void free_instance(instance *inst);



//--- solution utilities ---

/**
 * Plot the solution using gnuplot
 * 
 * @param inst The instance of the problem
 * @param sol The solution of the instance
 */
void plot_solution(instance inst, solution sol);

/**
 * Deallocates the most space-consuming attributes of solution from memory
 * 
 * @param inst The solution pointer of the problem
 */
void free_solution(solution *sol);



//--- main utilities ---

/**
 * Print the error
 * 
 * @param err The error string
 */
void print_error(const char *err);

/**
 * Parses the input from the command line
 *
 * @param argc The argc from the main function
 * @param argv The argv from the main function
 * @param inst The instance pointer of the problem
 */ 
void parse_command_line(int argc, const char *argv[], instance *inst);

#endif