#ifndef PLOT_H
#define PLOT_H

#include "tsp.h"

#include <stdio.h>
#include <sys/stat.h>

/**
 * Open a Gnuplot file
 * 
 * @return The Gnuplot file pointer
 */
FILE *open_plot(void);

/**
 * Save the plot in a png file
 * 
 * @param plot The Gnuplot file pointer
 * @param filename The string name of the png file
 */
void plot_in_file(FILE *plot, const char *filename);

/**
 * Add Gnuplot customization
 * 
 * @param plot The Gnuplot file pointer
 * @param customization The string of customization to add
 */
void add_plot_customization(FILE *plot, const char *customization);

/**
 * Plot an edge between the two nodes
 * 
 * @param plot The Gnuplot file pointer
 * @param node1 The coordinate of first node
 * @param node2 The coordinate of second node
 */
void plot_edge(FILE *plot, coordinate node1, coordinate node2);

/**
 * Plot stats of the cost and best solution cost based on number of solutions
 * 
 * @param plot The Gnuplot file pointer
 * @param filepath path to csv file for data
 */
void plot_cost_evolution(FILE *plot, char* filepath);

/**
 * Plot stats in a file
 * 
 * @param filename name of the file
 */
void plot_stats_in_file(char* filename);


/**
 * Tells Gnuplot that the input data are ended
 * 
 * @param plot The Gnuplot file pointer
 */
void input_end(FILE *plot);

/**
 * Close the Gnuplot file
 * 
 * @param plot The Gnuplot file pointer
 */
void free_plot(FILE *plot);

#endif //PLOT_H