#ifndef UTILITIES_H
#define UTILITIES_H

#include "tsp.h"
#include "heuristics.h"
#include "utilities_instance.h"
#include "utilities_solution.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


// useful constants
#define EPSILON 1e-5

//-------------------------------------- main utilities --------------------------------------

/**
 * Parses the input from the command line
 *
 * @param argc The argc from the main function
 * @param argv The argv from the main function
 * @param inst The instance pointer of the problem
 */ 
void parse_command_line(const int argc, const char *argv[], instance *inst);



//------------------------------------ various utilities -------------------------------------

/**
 * Calculates the seconds since a reference time.
 * 
 * @return The seconds
 */
time_t seconds(void);

/**
 * Compute the elapsed_time (duration) in seconds from starting time to current time.
 * 
 * @param start starting time
 * 
 * @return The elapsed_time
 */
int get_elapsed_time(const time_t start);

/**
 * Draw a random value between 0 and 1
 * 
 * @return The random value
 */
double random01(void);

/**
 * Compute the Euclidean distance between two points
 * 
 * @param point1 The first point
 * @param point2 The second point
 * 
 * @return The distance between the points
 */
double dist(const coordinate point1, const coordinate point2);

#endif //UTILITIES_H