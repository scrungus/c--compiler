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

enum tac_op
  {
    no_op = 0,
    tac_plus = 1
  };

typedef struct tac {
int op ;
TOKEN* src1;
TOKEN* src2;
TOKEN* dst;
struct tac* next;
} TAC ;

VALUE* interpret_tree(NODE*);

#endif // INTERPRETER