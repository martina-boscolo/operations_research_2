#ifndef BRANCH_AND_CUT_H
#define BRANCH_AND_CUT_H

#include "tsp.h"
#include "tsp_cplex.h"
#include "utilities.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ilcplex/cplex.h>

#define BRANCH_AND_CUT "C"

void branch_and_cut(instance *inst, solution *sol, const double timelimit);

void warm_up(const instance *inst, const solution *sol, CPXENVptr env, CPXLPptr lp);

void install_callback(instance *inst, CPXENVptr env, CPXLPptr lp);

int CPXPUBLIC lazy_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

// return 0 ok, 1 error
int add_SECs_to_pool(const instance *inst, CPXCALLBACKCONTEXTptr context, const int *comp, const int ncomp, const int tree_node);

#endif // BRANCH_AND_CUT_H