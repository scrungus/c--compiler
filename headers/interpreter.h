#include "nodes.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef INTERPRETER
#define INTERPRETER

typedef struct value {
    int type;
    union 
    {
        int integer;
        int boolean;
        char* string;

    } ;
    
}VALUE;

VALUE* interpret_tree(NODE*);

#endif // INTERPRETER