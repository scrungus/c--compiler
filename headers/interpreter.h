#include "nodes.h"
#include <stdio.h>
#include <stdlib.h>
#include "environment.h"

#ifndef INTERPRETER
#define INTERPRETER

VALUE* interpret_tree(NODE*,FRAME*);
VALUE* make_value_int(int,int);


#endif // INTERPRETER