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
#include <windows.h>

//-------------------------------------- main utilities --------------------------------------

/**
 * Print "Error: " followed by the error message and exit the program.
 * 
 * @param err The error message to print (input)
 */
void print_error(const char *err);

/**
 * Parse the input from the command line and fill the instance structure with the parsed values.
 *
 * @param argc The argc from the main function (input)
 * @param argv The argv from the main function (input)
 * @param inst The instance to fill with the parsed values (output)
 */ 
void parse_command_line(const int argc, const char *argv[], instance *inst);

//------------------------------------ various utilities -------------------------------------

/**
 * Compute the time passed in seconds from starting time to current time.
 * 
 * @param start the starting time (input)
 * 
 * @return The elapsed time
 */
double get_elapsed_time(const double start);

/**
 * Get the current time in seconds using high-resolution performance counter.
 * NOTE: Windows only.
 * 
 * @return The current time in seconds
 */
double get_time_in_milliseconds();

/**
 * Draw a random value between 0 and 1.
 * NOTE: This function is not thread-safe.
 * 
 * @return The random value
 */
double random01(void);

/**
 * Draw a random value between 0 and 1 in a thread-safe way.
 * 
 * @return The random value
 */
double thread_safe_rand_01();

/**
 * Compute the Euclidean distance between two points.
 * 
 * @param point1 The first point (input)
 * @param point2 The second point (input)
 * 
 * @return The distance between the points
 */
double dist(const coordinate point1, const coordinate point2);

#endif //UTILITIES_H