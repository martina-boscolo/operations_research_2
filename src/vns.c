#include "vns.h"

void vns(instance *inst, solution *sol) {

    // obtain first solution
    nearest_neighbor(inst, sol, rand() % inst->nnodes);

    while (get_elapsed_time(inst->t_start) < inst->timelimit) {

        // go to local optima
        two_opt(inst, sol);

        // update current best solution
        update_best_sol(inst, sol);

        // escape local minima
        kick(inst, sol);

    }

    strcpy(sol->method, "VNS");

}

void kick(instance *inst, solution *sol) {

    const int reps = 3;

    // take 3 random points and do a fixed 3-move for reps times
    for (int i=0; i<reps; i++) {

        int index1 = rand() % inst->nnodes;
        int index2 = rand() % inst->nnodes;
        int index3 = rand() % inst->nnodes;

        // order them
        if (index1 > index2) { int temp = index1; index1 = index2; index2 = temp; }
        if (index1 > index3) { int temp = index1; index1 = index3; index3 = temp; }
        if (index2 > index3) { int temp = index2; index2 = index3; index3 = temp; }
        
        // Calculate the cost of the three edges that would be removed
        double old_cost = cost(sol->visited_nodes[index1], sol->visited_nodes[index1+1], inst) +
                          cost(sol->visited_nodes[index2], sol->visited_nodes[index2+1], inst) +
                          cost(sol->visited_nodes[index3], sol->visited_nodes[index3+1], inst);

        // Calculate the cost of the three new edges that would be added
        double new_cost = cost(sol->visited_nodes[index1], sol->visited_nodes[index2+1], inst) +
                          cost(sol->visited_nodes[index2], sol->visited_nodes[index3+1], inst) +
                          cost(sol->visited_nodes[index3], sol->visited_nodes[index1+1], inst);

        // Operate the move
        shift_segment(sol, index2, index3, index1);

        // Update the solution cost correctly
        sol->cost += (new_cost - old_cost);

        print_solution(sol, inst->nnodes);
        check_sol(inst, sol);
        
    }

}
