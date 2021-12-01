#include "token.h"

#ifndef MCENV
#define MCENV
#define MAXREGS 8
#define MAXARGS 4
typedef struct bnding {
  TOKEN* name;
  TOKEN* loc;
  struct bnding* next;
} BNDING;

typedef struct frme {
  BNDING* bindings;
  struct frme* next;
}FRME;

TOKEN *lookup_loc(TOKEN*, FRME*);
TOKEN *assign_to_var(TOKEN*, FRME*,TOKEN*);
void declare_var(TOKEN*, FRME*);
int reg_in_use(int, FRME*);
void delete_constants(FRME*);
#endif