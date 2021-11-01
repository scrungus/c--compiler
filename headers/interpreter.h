#include "nodes.h"
#include <stdio.h>
#include <stdlib.h>
#include "environment.h"

#ifndef INTERPRETER
#define INTERPRETER


enum tac_op
  {
    no_op = 0,
    tac_plus = 1,
    tac_minus = 2,
    tac_div = 3,
    tac_mod = 4,
    tac_mult = 5,

    tac_proc = 6,
    tac_endproc = 7
  };

typedef struct tac {
int op ;
TOKEN* src1;
TOKEN* src2;
TOKEN* dst;
struct tac* next;
}TAC;


VALUE* interpret_tree(NODE*,FRAME*);
VALUE* make_value_int(int,int);


#endif // INTERPRETER