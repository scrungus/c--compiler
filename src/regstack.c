#include "token.h"
#include <stdlib.h>
#include "regstack.h"

#define MAXSIZE 32 

TOKEN* stack[MAXSIZE];

int top = -1;

int isempty() {

   if(top == -1)
      return 1;
   else
      return 0;
}
   
int isfull() {

   if(top == MAXSIZE)
      return 1;
   else
      return 0;
}

TOKEN* pop() {
   TOKEN* data;
	
   if(!isempty()) {
      data = stack[top];
      top = top - 1;   
      return data;
   } else {
      return NULL;
   }
}

TOKEN* peep(){
    TOKEN* data;
    if(!isempty()) {
      data = stack[top]; 
      return data;
   } else {
      return NULL;
   }
}

int push(TOKEN* data) {

   if(!isfull()) {
      top = top + 1;   
      stack[top] = data;
      return 0;
   } else {
      return -1;
   }
}