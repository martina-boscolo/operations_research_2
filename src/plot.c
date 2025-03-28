#include "plot.h"
#include "utilities.h"

FILE *open_plot(void) {

    FILE *plot = popen("gnuplot -persistent", "w");

    // if cannot open file
    if (!plot) {
        printf("Open Gnuplot");
        exit(EXIT_FAILURE);
    }

    return plot;

}

void plot_in_file(FILE *plot, const char *filename) {

    // Create the plot directory if does not exists
    mkdir("./plot", 0777); 
    
    fprintf(plot, "set terminal png size 800, 600\n");
    fprintf(plot, "set output './plot/%s.png'\n", filename);

}

void add_plot_customization(FILE *plot, const char *customization) {

    fprintf(plot, "%s\n", customization);

}

void plot_edge(FILE *plot, const coordinate node1, const coordinate node2) {

    // Plot edge using format: "x1 y1\nx2 y2\n\n"
    // The last '\n' is needed to separate the edges for Gnuplot
    fprintf(plot, "%lf %lf\n", node1.x, node1.y);
    fprintf(plot, "%lf %lf\n\n", node2.x, node2.y);

}

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
        "B_slope * x + B_intercept with lines title \"Current Linear Fit\", "
        "'%s' using (column(1)):(abs(column(2) - B_min_y) < 1e-4 ? column(2) : 1/0) "
        "with points pt 7 lc \"red\" title \"Min Current: \" . gprintf(\"%%.2f\", B_min_y)\n",
        filepath, filepath, filepath);
        
}

void plot_stats_in_file(const char* filename){
    
    FILE *plot = open_plot();
    plot_in_file(plot, filename);
    char filepath[50];
    sprintf(filepath, "results/%s.csv", filename);
    plot_cost_evolution(plot, filepath);
    free_plot(plot);

}

void input_end(FILE *plot) {

    fprintf(plot, "e");  

}

void free_plot(FILE *plot) {

    fflush(plot);
    pclose(plot);

}