#include "token.h"
#include "nodes.h"
#ifndef GENTAC
#define GENTAC 
enum tac_op
  {
    tac_plus = 1,
    tac_minus = 2,
    tac_div = 3,
    tac_mod = 4,
    tac_mult = 5,

    tac_proc = 6,
    tac_endproc = 7,
    tac_load = 8,
    tac_store = 9
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

typedef struct load {
  TOKEN* src1;
  TOKEN* dst;
}LOAD;

typedef struct tac {
int op ;
union {STAC stac; PROC proc; LOAD ld;};
struct tac* next;
}TAC;

TAC* gen_tac(NODE*);


#endif