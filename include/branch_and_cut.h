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

void branch_and_cut(const instance *inst, solution *sol, const double timelimit);

int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle );

#endif // BRANCH_AND_CUT_H