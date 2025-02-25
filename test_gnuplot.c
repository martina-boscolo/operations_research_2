#include <stdio.h>
#include <stdlib.h>

void generateTestData() {
    FILE *file = fopen("test_data.dat", "w");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // Writing simple (x, y) coordinates
    fprintf(file, "1 2\n");
    fprintf(file, "2 3\n");
    fprintf(file, "3 5\n");
    fprintf(file, "4 7\n");
    fprintf(file, "5 11\n");

    fclose(file);
}

void plotTestData() {
    FILE *gnuplot = popen("gnuplot -persistent", "w");
    if (!gnuplot) {
        perror("Error opening Gnuplot");
        return;
    }

    fprintf(gnuplot, "set title 'Test Plot'\n");
    fprintf(gnuplot, "set xlabel 'X-axis'\n");
    fprintf(gnuplot, "set ylabel 'Y-axis'\n");
    fprintf(gnuplot, "plot 'test_data.dat' using 1:2 with linespoints title 'Test Data'\n");

    fflush(gnuplot);
    pclose(gnuplot);
}

int main() {
    generateTestData();  // Create data file
    plotTestData();      // Call Gnuplot to plot

    return 0;
}
