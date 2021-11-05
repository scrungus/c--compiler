#include "gentac.h"
#include <stdlib.h>
#include "C.tab.h"
#include "nodes.h"
#include <stdio.h>
#include <ctype.h>
#include "value.h"
#include "environment.h"
#include "token.h"

extern VALUE *lookup_name(TOKEN*, FRAME*);
extern VALUE *assign_to_name(TOKEN*, FRAME*,VALUE*);
extern VALUE *declare_name(TOKEN*, FRAME*);
extern TOKEN* new_token(int);

TAC* empty_tac() {
    TAC* ans = (TAC*)malloc(sizeof(TAC));
    if (ans==NULL) {
        printf("Error! memory not allocated.");
        exit(0);
    }
    return ans;
}

TAC* find_last(TAC* tac){
    while(tac->next!=NULL){
        tac = tac->next;
    }
    return tac;
}

TAC* new_stac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst){
  TAC* ans = empty_tac();
  ans->op = op;
  ans->stac.src1 = src1;
  ans->stac.src2 = src2;
  ans->stac.dst = dst;
  return ans;
}

TAC* new_proc (TOKEN* name, int arity){
    TAC* ans = empty_tac();
    ans->op = tac_proc;
    ans->proc.name = name;
    ans->proc.arity = arity;
    return ans;
}

TAC* new_endproc(){
    TAC* ans = empty_tac();
    ans->op = tac_endproc;
    return ans;
}

TAC* new_load(TOKEN* name, TOKEN* dst){
    TAC* ans = empty_tac();
    ans->op = tac_load;
    ans->ld.src1 = name;
    ans->ld.dst = dst;
    return ans;
}

TAC* new_store(TOKEN* name, TOKEN* dst){
    TAC* ans = empty_tac();
    ans->op = tac_store;
    ans->ld.src1 = name;
    ans->ld.dst = dst;
    return ans;
}

TOKEN * new_dest(int counter){
    TOKEN* dst = (TOKEN*)malloc(sizeof(TOKEN));
    if(dst==NULL){printf("fatal: failed to generate destination\n");exit(1);}
    dst->type=IDENTIFIER;
    dst->lexeme = (char*)calloc(1,2);
    sprintf(dst->lexeme,"t%i",counter);
    return dst;
}

TAC* parse_tilde(NODE* tree, TOKEN* currdst, int counter){
    TAC *tac,*last;
    TOKEN* t;
     if(tree->left->left->type==INT){
        if(tree->right->type == LEAF){
            t = (TOKEN *)tree->right->left;
            TOKEN* new = new_token(CONSTANT);
            new->value = 0;
            tac = new_load(new,currdst);
            tac->next = new_store(currdst,t);
            return tac;
        }
        else if((char)tree->right->type == '='){
            t = (TOKEN *)tree->right->left->left;
            tac = gen_tac0(tree->right->right,currdst,counter);
            last = find_last(tac);
            if(last->stac.dst != NULL){
                last->next = new_store(last->stac.dst,t);
            }
            else{  last->next = new_store(last->ld.dst,t); }
            return tac;
        }
    }
    tac = gen_tac0(tree->left,currdst,counter);
    last = find_last(tac);
    last->next = gen_tac0(tree->right,currdst,counter);
    return tac;
}

TAC *gen_tac0(NODE *tree, TOKEN *currdst, int counter){

    TOKEN *left, *right;
    TAC *tac, *last;
    TOKEN *t;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
            t = (TOKEN *)tree->left;
            tac = new_load(t,currdst);
            /* if(t->type==IDENTIFIER){
                tac = new_store(currdst,t); //PROBLEM: How do you tell whether a variable has been declared already? i.e. whether it needs to be stored or loaded
            } */
            return tac;
        }
    char c = (char)tree->type;
    if (isgraph(c) || c==' ') {
        switch(c){
            default: printf("fatal: unknown token type '%d'\n",c); exit(1);
            
            case '~':
               return parse_tilde(tree,currdst,counter);
            case 'D':
                tac = gen_tac0(tree->left,currdst,counter);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,currdst,counter);
                last = find_last(tac);
                last->next = new_endproc();
                return tac;
            case 'd':
                return gen_tac0(tree->right,currdst,counter);
            case 'F':
                left = (TOKEN *)tree->left->left;
                return new_proc(left,0);
            case ';':
                tac = gen_tac0(tree->left,currdst,counter);
                last = find_last(tac);
                currdst = new_dest(++counter); 
                last->next = gen_tac0(tree->right,currdst,counter);
                return tac;
            case '=':
                tac = gen_tac0(tree->right,currdst,counter);
                last = find_last(tac);
                t = (TOKEN *)tree->left->left;
                if(last->stac.dst != NULL){
                    last->next = new_store(last->stac.dst,t);
                }
                else{  last->next = new_store(last->ld.dst,t); }
                return tac;
            case '+':
                left = new_dest(++counter);
                tac = gen_tac0(tree->left,left,counter);
                right = new_dest(++counter);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,right,counter);
                last = find_last(last);
                last->next = new_stac(tac_plus,left,right,currdst);
                return tac;
            case '-':
                left = new_dest(++counter);
                tac = gen_tac0(tree->left,left,counter);
                right = new_dest(++counter);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,right,counter);
                last = find_last(tac);
                last->next = new_stac(tac_minus,left,right,currdst);
                return tac;
            case '*':
                left = new_dest(++counter);
                tac = gen_tac0(tree->left,left,counter);
                right = new_dest(++counter);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,right,counter);
                last = find_last(tac);
                last->next = new_stac(tac_mult,left,right,currdst);
                return tac;
            case '/':
                left = new_dest(++counter);
                tac = gen_tac0(tree->left,left,counter);
                right = new_dest(++counter);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,right,counter);
                last = find_last(tac);
                last->next = new_stac(tac_div,left,right,currdst);
                return tac;
            case '%':
                left = new_dest(++counter);
                tac = gen_tac0(tree->left,left,counter);
                right = new_dest(++counter);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,right,counter);
                last = find_last(tac);
                last->next = new_stac(tac_mod,left,right,currdst);
                return tac;
        }
    }
    switch(tree->type){
    default: printf("fatal: unknown token type '%c'\n", tree->type); exit(1);
    case RETURN:  
        return gen_tac0(tree->left,currdst,counter);
    }

}


TAC *gen_tac(NODE* tree){
    int counter = 0;
    TOKEN *currdst = new_dest(counter);
    gen_tac0(tree,currdst,counter);
}