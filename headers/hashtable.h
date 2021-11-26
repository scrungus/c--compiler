#include "token.h"
#include "gentac.h"
#ifndef HASHTABLE
#define HASHTABLE 

typedef struct DataItem {
   TAC* data;   
   TOKEN* key;
}DATA_ITEM;


BB* insert(TOKEN*,TAC*); 

#endif