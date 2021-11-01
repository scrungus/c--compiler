#include "token.h"
#ifndef GENTAC
#define GENTAC 
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

typedef struct simple_tac {
  TOKEN* src1;
  TOKEN* src2;
  TOKEN* dst;
}STAC;

typedef struct proc {
  TOKEN* name;
  int arity;
}PROC;

typedef struct literal {
  TOKEN* src1;
  TOKEN* dst;
}LITERAL;

typedef struct tac {
int op ;
union {STAC stac; PROC proc; LITERAL lit};
struct tac* next;
}TAC;




#endif