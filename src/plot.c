#include "plot.h"
#include "utilities.h"

// Open a Gnuplot file
FILE *open_plot(void) {

    FILE *plot = _popen("gnuplot -persistent", "w");

    if (!plot) print_error("open_plot(): Cannot opening Gnuplot");

    return plot;

}

// Save the plot in a png file
void plot_in_file(FILE *plot, const char *filename) {

    // Create the plot directory if does not exists
    _mkdir("./plot");
    
    fprintf(plot, "set terminal png size 800, 600\n");
    fprintf(plot, "set output './plot/%s.png'\n", filename);

}

// Add Gnuplot customization
void add_plot_customization(FILE *plot, const char *customization) {

    fprintf(plot, "%s\n", customization);

}

// Plot an edge between the two nodes
void plot_edge(FILE *plot, const coordinate node1, const coordinate node2) {

    // Plot edge using format: "x1 y1\nx2 y2\n\n"
    // The last '\n' is needed to separate the edges for Gnuplot
    fprintf(plot, "%lf %lf\n", node1.x, node1.y);
    fprintf(plot, "%lf %lf\n\n", node2.x, node2.y);

}

// Plot stats of the cost and best solution cost based on number of solutions
void plot_cost_evolution(FILE *plot, const char* filepath) {

    fprintf(plot, "set style data line\n");
    fprintf(plot, "set datafile separator \",\"\n");

    // Stats for both current cost (column 2) and best cost (column 3)
    fprintf(plot, "stats '%s' using 1:2 prefix \"B\" nooutput\n", filepath);
    fprintf(plot, "stats '%s' using 1:3 prefix \"Best\" nooutput\n", filepath);

    // Plot data
    fprintf(plot,
        "plot '%s' using 1:2 title \"Current Cost\" lw 1, "
        "'%s' using 1:3 title \"Best Cost\" lw 2 lc \"blue\", "
        "'%s' using (column(1)):(abs(column(2) - B_min_y) < 1e-4 ? column(2) : 1/0) "
        "with points pt 7 lc \"red\" title \"Min Current: \" . gprintf(\"%%.2f\", B_min_y)\n",
        filepath, filepath, filepath);
        
}

// Plot the cost evolution in a base file
void plot_cost_evolution_base(FILE *plot, const char* filepath) {

    fprintf(plot, "set style data line\n");
    fprintf(plot, "set datafile separator \",\"\n");
    
    // Make sure to read the full file
    fprintf(plot, "set datafile missing \"NaN\"\n");
    
    // Add labels for the axes with time in seconds
    fprintf(plot, "set xlabel 'Time (seconds)'\n");
    fprintf(plot, "set ylabel 'Cost'\n");
    fprintf(plot, "set title 'Cost Evolution Over Time'\n");
    
    // Set time format if needed
    fprintf(plot, "set format x \"%%g s\"\n");
    
    // Ensure all points are plotted by setting autoscale
    fprintf(plot, "set autoscale\n");
    
    // Plot data - using column 3 (time in seconds) as x-axis and column 2 (cost) as y-axis
    fprintf(plot,
        "plot '%s' using 3:2 title \"Cost vs Time\" with linespoints lw 1.5 pt 7 ps 1\n",
        filepath);
        
    // Flush to ensure all commands are sent
    fflush(plot);

}

// Plot stats in a file
void plot_stats_in_file(const char* filename) {
    
    FILE *plot = open_plot();
    plot_in_file(plot, filename);
    char filepath[FILE_NAME_LEN];
    sprintf_s(filepath, FILE_NAME_LEN, "results/%s.csv", filename);
    plot_cost_evolution(plot, filepath);
    free_plot(plot);

}

// Plot stats in a base file
void plot_stats_in_file_base(const char* filename) {
    
    FILE *plot = open_plot();
    plot_in_file(plot, filename);
    char filepath[FILE_NAME_LEN];
    sprintf_s(filepath, FILE_NAME_LEN, "results/%s.csv", filename);
    plot_cost_evolution_base(plot, filepath);
    free_plot(plot);

}

// Plot the subtours in the Gnuplot file
void plot_subtours(const instance *inst, const int **subtours, const int *subtour_lengths, const int nsubtours, const int iter) {
    
    FILE *gnuplot = open_plot();

    char filename[FILE_NAME_LEN];
    sprintf_s(filename, FILE_NAME_LEN, "%s_subtours_iter%d", inst->name, iter);
    plot_in_file(gnuplot, filename);

    add_plot_customization(gnuplot, "plot '-' using 1:2 w linespoints pt 7");

    for (int k = 0; k < nsubtours; k++) {

        for (int i = 0; i < subtour_lengths[k]; i++) {

            int from = subtours[k][i];
            int to = subtours[k][(i + 1) % subtour_lengths[k]];

            coordinate c1 = inst->coord[from];
            coordinate c2 = inst->coord[to];
            plot_edge(gnuplot, c1, c2);

        }

    }

    input_end(gnuplot);
    free_plot(gnuplot);

}

// Tell Gnuplot that the input data are ended
void input_end(FILE *plot) {

    fprintf(plot, "e\n");  

}

// Close the Gnuplot file
void free_plot(FILE *plot) {

    fflush(plot);
    _pclose(plot);

}