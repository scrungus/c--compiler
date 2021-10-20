#include "token.h"

#ifndef NODES //avoid redefinition errors
#define NODES

typedef struct node {
  int          type;
  struct node *left;
  struct node *right;
} NODE;

NODE* make_leaf(TOKEN*);
NODE* make_node(int, NODE*, NODE*);

#endif // NODES