#include "token.h"
#include <stdlib.h>
#include "regstack.h"

#define MAXREGSIZE 8 
#define MAXARGS 4
#define MAXLBLS 10

TOKEN* stack[MAXREGSIZE];
TOKEN* arg_stack[MAXARGS];
TOKEN* lbl_stack[MAXLBLS];

int top = -1;
int top_args = -1;
int top_lbls = -1;

int isempty() {

   if(top == -1)
      return 1;
   else
      return 0;
}

int isempty_args() {

   if(top_args == -1)
      return 1;
   else
      return 0;
}

int isempty_lbls() {

   if(top_lbls == -1)
      return 1;
   else
      return 0;
}
   
int isfull() {

   if(top == MAXREGSIZE)
      return 1;
   else
      return 0;
}

int isfull_args() {

   if(top_args == MAXARGS)
      return 1;
   else
      return 0;
}

int isfull_lbls() {

   if(top_lbls == MAXLBLS)
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

TOKEN* peep_lbl(){
    TOKEN* data;
    if(!isempty_lbls()) {
      data = lbl_stack[top_lbls]; 
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

int push_arg(TOKEN* data) {

if(!isfull_args()) {
   top_args = top_args + 1;   
   arg_stack[top_args] = data;
   return 0;
} else {
   return -1;
}
}

TOKEN* pop_arg() {
   TOKEN* data;
	
   if(!isempty_args()) {
      data = arg_stack[top_args];
      top_args = top_args - 1;   
      return data;
   } else {
      return NULL;
   }
}

int push_lbl(TOKEN* data) {

if(!isfull_lbls()) {
   top_lbls = top_lbls + 1;   
   lbl_stack[top_lbls] = data;
   return 0;
} else {
   return -1;
}
}

TOKEN* pop_lbl() {
   TOKEN* data;
	
   if(!isempty_lbls()) {
      data = lbl_stack[top_lbls];
      top_lbls = top_lbls - 1;   
      return data;
   } else {
      return NULL;
   }
}