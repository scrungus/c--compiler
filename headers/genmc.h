#include "gentac.h"

#ifndef GENMC
#define GENMC
#define M 4
#define N 8
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
unsigned int fp; // save caller ’s AR
unsigned int pc; // save caller ’s PC
unsigned int sl; // this function ’s static link
unsigned int param[M]; // param0 , ... paramm ,
unsigned int local[N]; // local0 , ... localn ,
//unsigned int tmp[k];
} AR;

MC* gen_mc(BB**);
#endif