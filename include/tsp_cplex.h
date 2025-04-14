#ifndef TSP_CPLEX_H
#define TSP_CPLEX_H

#include "tsp.h"
#include "utilities_instance.h"

#include <ilcplex/cplex.h>
#include <stdbool.h>

/**
 * Initialize the CPLEX enviroment and build the base model of TSP (w/o SEC's)
 * 
 * @param inst The instance pointer of the problem
 * @param env CPLEX environment
 * @param lp CPLEX LP
 */
int initialize_CPLEX(instance *inst, CPXENVptr *env, CPXLPptr *lp);

/**
 * Obtain the optimal solution of lp w.r.t. instance, store it in xstar, succ, comp, ncomp
 * 
 * @param inst The instance pointer of the problem
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * @param xstar Contain the optimal coefficient for each edge
 * @param succ Solution as successors
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 */
int get_optimal_solution_CPLEX(instance *inst, CPXENVptr env, CPXLPptr lp, double *xstar, int *succ, int *comp, int *ncomp);

/**
 * For each connected component add the correspondent SEC to the lp model
 * 
 * @param inst The instance pointer of the problem
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 */
int build_SECs(instance *inst, CPXENVptr env, CPXLPptr lp, const int *comp, const int ncomp);

/**
 * Add the SEC to the lp model
 * 
 * @param inst The instance pointer of the problem
 * @param env CPLEX environment
 * @param lp CPLEX LP
 * @param sec_comp Number of the connected component the SEC will be associated
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 * @param cname Array for the name of the constraint 
 * @param index Index for non zero coefficients
 * @param value Value for non zero coefficients
 */
int add_SEC(instance *inst, CPXENVptr env, CPXLPptr lp, const int sec_comp, const int *comp, const int ncomp, char **cname, int *index, double *value);

/**
 * Return the index of CPLEX solution array from the edge between two nodes
 * 
 * @param i First node
 * @param j Second node
 * @param inst The instance pointer of the problem
 * @return Edge index
 */
int xpos(int i, int j, instance *inst);                                       
    
/**
 * Build the ILP model
 * 
 * @param inst The instance pointer of the problem
 * @param env CPLEX environment
 * @param lp CPLEX LP
 */
void build_model_CPLEX(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Build the solution based on the edges selected in xstar
 *
 * @param xstar CPLEX solution
 * @param inst The instance pointer of the problem
 * @param succ Solution as successors
 * @param comp Component associated for each nodes
 * @param ncomp Number of components in the solution
 */
void build_sol_CPLEX(const double *xstar, instance *inst, int *succ, int *comp, int *ncomp);

/**
 * From CPLEX solution, as successors, to a solution in the struct
 * Note: assume that there is only one connected component
 * 
 * @param inst The instance pointer of the problem
 * @param sol The solution pointer of the instance
 * @param succ Solution as successors
 */
void build_solution_form_CPLEX(instance *inst, solution *sol, int *succ);

void free_CPLEX(CPXENVptr *env, CPXLPptr *lp);

#endif //TSP_CPLEX_H