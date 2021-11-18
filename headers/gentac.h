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
    tac_store = 9,
    tac_if = 10,
    tac_lbl = 11,
    tac_goto = 12
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

typedef struct label {
  TOKEN* name;
}LABEL;

typedef struct iftest {
  TOKEN* op1;
  TOKEN* op2;
  int code;
  TOKEN* lbl;
}IFTEST;

typedef struct gotolbl {
  TOKEN* lbl;
}GOTO;

typedef struct tac {
int op ;
union {STAC stac; PROC proc; LOAD ld; LABEL lbl; IFTEST ift; GOTO gtl;};
struct tac* next;
}TAC;

TAC* gen_tac(NODE*);
TAC* gen_tac0(NODE*, TOKEN*, int, TOKEN*, int);


#endif