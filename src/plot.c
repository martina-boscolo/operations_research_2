#include "plot.h"
#include "utilities.h"

FILE *open_plot(void) {

    FILE *plot = popen("gnuplot -persistent", "w");

    // if cannot open file
    if (!plot) {
        print_error("Open Gnuplot");
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

void plot_edge(FILE *plot, coordinate node1, coordinate node2) {

    // Plot edge using format: "x1 y1\nx2 y2\n\n"
    // The last '\n' is needed to separate the edges for Gnuplot
    fprintf(plot, "%lf %lf\n", node1.x, node1.y);
    fprintf(plot, "%lf %lf\n\n", node2.x, node2.y);

}

void plot_stats(FILE *plot, char* filepath) {
    
    fprintf(plot, "set style data line\n");
    fprintf(plot, "set datafile separator \",\"\n");
    fprintf(plot, "stats '%s' using 1:2 prefix \"B\" nooutput\n", filepath);
    fprintf(plot, "plot '%s' using 1:2 title \"  Data\" lw 2, \
        B_slope * x + B_intercept with lines title \"  Linear fit\", \
        '%s' using (column(1)):(abs(column(2) - B_min_y) < 1e-4 ? column(2) : 1/0) \
        with points pt 7 lc \"red\" title \"Minimum: \" . gprintf(\"%%.2f\", B_min_y) \n", 
       filepath, filepath);
}

void input_end(FILE *plot) {

    fprintf(plot, "e");  
}

void free_plot(FILE *plot) {

    fflush(plot);
    pclose(plot);

}