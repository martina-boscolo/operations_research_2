#ifndef UTILITIES_H
#define UTILITIES_H

#include "tsp.h"
#include "plot.h"
#include "heuristics.h" //cyclic dependence to allow the selection of the method


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <time.h>


// useful constants
#define EPSILON 1e-5

//------------------------------------ instance utilities ------------------------------------

// Maximum value for x and y coordinates for a random instance
#define MAX_XCOORD 10000
#define MAX_YCOORD 10000

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
 * Allocates the memory for the most space-consuming attributes of instance
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



//------------------------------------ solution utilities ------------------------------------

/**
 * Initialize array of nodes (ordered array) 
 *  
 * @param visited_nodes array of nodes
 * @param nnodes number of nodes
 */
void initialize_solution(int *visited_nodes, int nnodes);

/**
 * Select the method to solve the TSP
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void solve_with_method(instance *inst, solution *sol); 

/**
 * Check if all nodes are visited exactly once.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return 1 if valid, 0 otherwise.
 */
int validate_node_visits(instance *inst, solution *sol);

/**
 * Compute the total cost of the given solution path.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return the cost of the solution
 */
double compute_solution_cost(instance *inst, solution *sol);

/**
 * Main function to check the feasibility of the solution.
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * 
 * @return 1 if valid, 0 otherwise.
 */
void check_sol(instance *inst, solution *sol); 

/**
 * Plot the solution using gnuplot
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 */
void plot_solution(instance *inst, solution *sol);

/**
 * Print the hyperparameter of the solution
 * 
 * @param sol The solution pointer of the problem
 */
void print_solution(solution *sol, int nnodes);

/**
 * Initialize sol, setting cost to INF and allocating visited_nodes 
 *  
 * @param sol The solution pointer of the instance
 * @param nnodes number of nodes
 */
void allocate_solution(solution *sol, int nnodes);

/**
 * Deallocates the most space-consuming attributes of solution from memory
 * 
 * @param inst The solution pointer of the problem
 */
void free_solution(solution *sol);



//-------------------------------------- main utilities --------------------------------------

/**
 * Print the error and exit the program
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



//------------------------------------ various utilities -------------------------------------

/**
 * Calculates the seconds since a reference time.
 * 
 * @return The seconds
 */
time_t seconds(void);

/**
 * Compute the elapsed_time (duration) in seconds
 * 
 * @param start starting time
 * @param end ending time
 * @param frequency frequency of the performance counter
 * 
 * @return The elapsed_time
 */
double get_elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER frequency);

/**
 * Draw a random value between 0 and 1
 * 
 * @return The random value
 */
double random01(void);

/**
 * Compute the distance between two points
 * 
 * @param point1 The first point
 * @param point2 The second point
 * 
 * @return The distance between the points
 */
double dist(coordinate point1, coordinate point2);

#endif //UTILITIES_H