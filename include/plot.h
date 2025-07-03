#ifndef PLOT_H
#define PLOT_H

#include "tsp.h"

#include <stdio.h>
#include <sys/stat.h>
#include <direct.h>  // for _mkdir

/**
 * Open a Gnuplot file.
 * 
 * @return The Gnuplot file pointer (onutput)
 */
FILE *open_plot(void);

/**
 * Save the plot in a png file.
 * 
 * @param plot The Gnuplot file pointer (input/output)
 * @param filename The string name of the png file (input)
 */
void plot_in_file(FILE *plot, const char *filename);

/**
 * Add Gnuplot customization.
 * 
 * @param plot The Gnuplot file pointer (input/output)
 * @param customization The string of customization to add (input)
 */
void add_plot_customization(FILE *plot, const char *customization);

/**
 * Plot an edge between the two nodes.
 * 
 * @param plot The Gnuplot file pointer (input/output)
 * @param node1 The coordinate of first node (input)
 * @param node2 The coordinate of second node (input)
 */
void plot_edge(FILE *plot, const coordinate node1, const coordinate node2);

/**
 * Plot stats of the cost and best solution cost based on number of solutions.
 * 
 * @param plot The Gnuplot file pointer (input/output)
 * @param filepath The path to csv file for data (input)
 */
void plot_cost_evolution(FILE *plot, const char* filepath);

/**
 * Plot the cost evolution in a base file.
 * 
 * @param plot The Gnuplot file pointer (input/output)
 * @param filepath The path to csv file for data (input)
 */
void plot_cost_evolution_base(FILE *plot, const char* filepath);

/**
 * Plot stats in a file.
 * 
 * @param filename The name of the file (input)
 */
void plot_stats_in_file(const char* filename);

/**
 * Plot stats in a base file.
 * 
 * @param filename The name of the file
 */
void plot_stats_in_file_base(const char* filename);

/**
 * Plot the subtours in the Gnuplot file.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param subtours The array of subtours (input)
 * @param subtour_lengths The lengths of each subtour (input)
 * @param nsubtours The number of subtours (input)
 * @param iter The iteration number (input)
 */
void plot_subtours(const instance *inst, const int **subtours, const int *subtour_lengths, const int nsubtours, const int iter);

/**
 * Tell Gnuplot that the input data are ended.
 * 
 * @param plot The Gnuplot file pointer (input/output)
 */
void input_end(FILE *plot);

/**
 * Close the Gnuplot file.
 * 
 * @param plot The Gnuplot file pointer (input/output)
 */
void free_plot(FILE *plot);

#endif //PLOT_H