#include "token.h"

#ifndef MCENV
#define MCENV
#define MAXREGS 8
#define MAXARGS 4

typedef struct tac TAC;
typedef struct frme FRME;

typedef struct clsure {
FRME* env;
TAC* code;
} CLSURE;

typedef struct bnding {
  TOKEN* name;
  int type;
  union {TOKEN* loc; CLSURE* clos;};
  struct bnding* next;
} BNDING;

typedef struct frme {
  BNDING* bindings;
  int size;
  int stack_pos;
  struct frme* next;
}FRME;

TOKEN *lookup_loc(TOKEN*, FRME*);
TOKEN *assign_to_var(TOKEN*, FRME*,TOKEN*);
void declare_var(TOKEN*, FRME*);
int reg_in_use(int, FRME*);
void delete_constants(FRME*);
TOKEN* use_temp_reg(FRME *);
#endif