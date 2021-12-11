#include "gentac.h"

#ifndef GENMC
#define GENMC


enum{
  PRINT_INT = 1,
  PRINT_CHAR = 11,
  SBRK = 9,
  EXIT = 10
};

typedef struct mc {
  char* insn;
  struct mc* next;
} MC;

typedef struct var {
  TOKEN* name;
  TOKEN* reg;
  struct var* next;
}VAR;

typedef struct sr {
  int srcnt;
}SR;

typedef struct ar {
int size;
int arity;
unsigned int pc; // save caller ’s PC
unsigned int sl; // this function ’s static link
//unsigned int param[MAXARGS]; // param0 , ... paramm ,
unsigned int local[MAXREGS]; // local0 , ... localn ,
//unsigned int tmp[k];
} AR;

MC* gen_mc(TAC*);
MC* gen_mc0(TAC*, FRME*, AR*);
TOKEN * new_dst(FRME *);
#endif