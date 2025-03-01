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

void add_plot_customization(FILE *plot, const char *customization) {

    fprintf(plot, "%s\n", customization);

}

void plot_edge(FILE *plot, coordinate node1, coordinate node2) {

    // Plot edge using format: "x1 y1\nx2 y2\n\n"
    // The last '\n' is needed to separate the edges for Gnuplot
    fprintf(plot, "%lf %lf\n", node1.x, node1.y);
    fprintf(plot, "%lf %lf\n\n", node2.x, node2.y);

}

void input_end(FILE *plot) {

    fprintf(plot, "e");

}

void free_plot(FILE *plot) {

    fflush(plot);
    pclose(plot);

}