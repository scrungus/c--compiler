#include "gentac.h"
#include <stdlib.h>
#include "C.tab.h"
#include "nodes.h"
#include <stdio.h>
#include <ctype.h>
#include "value.h"
#include "environment.h"

extern VALUE *lookup_name(TOKEN*, FRAME*);
extern VALUE *assign_to_name(TOKEN*, FRAME*,VALUE*);
extern VALUE *declare_name(TOKEN*, FRAME*);

TAC* empty_tac() {
    TAC* ans = (TAC*)malloc(sizeof(TAC));
    if (ans==NULL) {
        printf("Error! memory not allocated.");
        exit(0);
    }
    return ans;
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

TAC* new_lit(TOKEN* name, TOKEN* dst){
    TAC* ans = empty_tac();
    ans->op = no_op;
    ans->lit.src1 = name;
    ans->lit.dst = dst;
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


TAC *gen_tac(NODE *tree, int counter){

    TOKEN *left, *right;
    TAC *tac;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
            TOKEN *t = (TOKEN *)tree->left;
            if (t->type == CONSTANT){
                return new_lit(t,new_dest(counter));
            }
        }
    char t = (char)tree->type;
    if (isgraph(t) || t==' ') {
        switch(t){
            default: printf("fatal: unknown token type '%c'\n",t); exit(1);
            
            case '~':
                tac = gen_tac(tree->left,counter++);
                tac->next = gen_tac(tree->right,counter++);
                return tac;
            case 'D':
                tac = gen_tac(tree->left,counter);
                tac->next = gen_tac(tree->right,counter++);
                tac->next->next = new_endproc();
                return tac;
            case 'd':
                return gen_tac(tree->right,counter++);
            case 'F':
                left = (TOKEN *)tree->left->left;
                return new_proc(left,0);
            case '+':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_stac(tac_plus,left,right,new_dest(counter));
            case '-':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_stac(tac_minus,left,right,new_dest(counter));
            case '*':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_stac(tac_mult,left,right,new_dest(counter));
            case '/':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_stac(tac_div,left,right,new_dest(counter));
            case '%':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_stac(tac_mod,left,right,new_dest(counter));
        }
    }
    switch(tree->type){
    default: printf("fatal: unknown token type '%c'\n", tree->type); exit(1);
    case RETURN:  
        return gen_tac(tree->left,counter++);
    } 

}