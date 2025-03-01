#ifndef PLOT_H
#define PLOT_H

#include "tsp.h"

#include <stdio.h>

/**
 * Open a Gnuplot file
 * 
 * @return The Gnuplot file pointer
 */
FILE *open_plot(void);

/**
 * Add Gnuplot customization
 * 
 * @param plot The Gnuplot file
 * @param customization The string of customization to add
 */
void add_plot_customization(FILE *plot, const char *customization);

/**
 * Plot an edge between the two nodes
 * 
 * @param plot The Gnuplot file
 * @param node1 The coordinate of first node
 * @param node2 The coordinate of second node
 */
void plot_edge(FILE *plot, coordinate node1, coordinate node2);

/**
 * Tells Gnuplot that the input data are ended
 * 
 * @param plot The Gnuplot file
 */
void input_end(FILE *plot);

/**
 * Close the Gnuplot file
 * 
 * @param plot The Gnuplot file
 */
void free_plot(FILE *plot);

#endif