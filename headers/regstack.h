#include "token.h"

#ifndef STACK
#define STACK
int isempty() ;
int isfull() ;
TOKEN* pop();
int push(TOKEN*);
#endif