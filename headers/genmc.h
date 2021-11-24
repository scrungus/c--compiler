#include "gentac.h"

#ifndef GENMC
#define GENMC
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

MC* gen_mc(TAC*);
#endif