#ifndef SC_GLOB__H
#define SC_GLOB__H

#include "symbol_table.h"
#include "vector_s.h"
#include "string_petey.h"

#include "sc_type.h"
#include "sc_func.h"

//number of "reserved" op codes:
#define SC_N_RESERVED 3

//type codes:
#define SC_SCALAR_T 0
#define SC_VECTOR_T 1
#define SC_FULL_T 2
#define SC_SPARSE_T 3
#define SC_SPARSE_ARRAY_T 4
#define SC_LIST_T 5
#define SC_STR_T 6
#define SC_TO_BE_DEFINED 7
#define SC_PARAMETER_T 8

//function table:
//single, global, function namespace
//you got a problem with that?
extern symbol_table<string_petey> sc_glob_funtab;
extern vector_s<sc_func_t*> sc_glob_funs;

//stack:
extern vector_s<sc_t *> sc_glob_stack;

#endif
