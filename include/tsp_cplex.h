#ifndef TSP_CPLEX_H
#define TSP_CPLEX_H

#include "tsp.h"
#include "utilities_instance.h"

#include <ilcplex/cplex.h>
#include <stdbool.h>

#define CONS_NAME_LEN 100

/**
 * Initialize the CPLEX enviroment and build the base model of TSP (w/o SEC's)
 * 
 * @param inst The instance that contains the problem data (input)
 * @param env CPLEX environment (output)
 * @param lp CPLEX model (output)
 */
void initialize_CPLEX(instance *inst, CPXENVptr *env, CPXLPptr *lp);

/**
 * Use the solution as warm up for the branching tree.
 * NOTE: This function assumes that the solution is feasible 
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to use as warm up (input)
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 */
void warm_up(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp);

/**
 * Obtain the optimal solution of lp w.r.t. instance and lp, store it in xstar, succ, comp, ncomp.
 * NOTE: This function assumes that the arrays are correctly allocated.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 * @param xstar The array that will contain the optimal coefficient for each edge (output)
 * @param succ The array that will contain the solution as successors (output)
 * @param comp The array that will contain the component associated for each nodes (output)
 * @param ncomp The number of components in the solution computed (output)
 * 
 * @return CPLEX status
 */
int get_optimal_solution_CPLEX(const instance *inst, CPXENVptr env, CPXLPptr lp, double *xstar, int *succ, int *comp, int *ncomp);

/**
 * Return if CPLEX found a feasible solution.
 * 
 * @param status CPLEX status (input)
 */
bool has_feasible_solution(const int status);

/**
 * Build the SEC correspondent to the given connected component.
 * NOTE: This function assumes that the arrays are correctly allocated.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param comp The component associated for each nodes (input)
 * @param sec_comp The index of connected component for which to compute the SEC (input)
 * @param index The array that will contain the indeces for non zero coefficients (output)
 * @param value The array that will contain the values for non zero coefficients (output)
 * @param nnz The number of non-zero coefficients: number of edges from each node to each node in the component (output)
 * @param rhs The right hand side: number of nodes in the connected component minus one (output)
 */
void build_SEC(const instance *inst,  const int *comp, const int sec_comp, int *index, double *value, int *nnz, double *rhs);

/**
 * Compute the index of edge between two nodes in the CPLEX model.
 * NOTE: This function assumes that the indices are within the bounds of the array. 
 * 
 * @param i The first node (input)
 * @param j The second node (input)
 * @param inst The instance that contains the problem data (input)
 * 
 * @return The edge index
 */
int xpos(const int i, const int j, const instance *inst);                                       
    
/**
 * Build the ILP model.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param env CPLEX environment (input/output)
 * @param lp CPLEX model (input/output)
 */
void build_model_CPLEX(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Build the solution based on the edges selected in xstar.
 * NOTE: This function assumes that the arrays are correctly allocated.
 * 
 * @param xstar CPLEX solution (input)
 * @param inst The instance that contains the problem data (input)
 * @param succ The array that will contain the solution as successors (output)
 * @param comp The array that will contain the component associated for each nodes (output)
 * @param ncomp The number of components in the solution computed (output)
 */
void build_sol_CPLEX(const double *xstar, const instance *inst, int *succ, int *comp, int *ncomp);

/**
 * Build a solution struct from CPLEX solution, as successors.
 * NOTE: This function assumes that there is only one connected component.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to build (output)
 * @param succ The solution as successors (input)
 */
void build_solution_from_CPLEX(const instance *inst, solution *sol, const int *succ);

/**
 * Build a CPLEX solution, as selected edges, from a solution struct.
 * NOTE: This function assumes that the array is correctly allocated.
 * 
 * @param inst The instance that contains the problem data (input)
 * @param sol The solution to convert (input)
 * @param xheu The solution as selected edges (output)
 */
void build_CPLEXsol_from_solution(const instance *inst, const solution *sol, double *xheu);

/**
 * Free memory associated with CPLEX
 * 
 * @param env CPLEX environment (output)
 * @param lp CPLEX model (output)
 */
void free_CPLEX(CPXENVptr *env, CPXLPptr *lp);

/**
 * Freeze execution until a user input is entered
 */
void press_a_key(void);

#endif //TSP_CPLEX_H