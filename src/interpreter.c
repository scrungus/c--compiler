#include "nodes.h"
#include "interpreter.h"
#include "environment.h"
#include "main.h"
#include <stdio.h>
#include <ctype.h>
#include "C.tab.h"

extern NODE *tree;
extern void print_tree(NODE *tree);
extern VALUE *lookup_name(TOKEN*, FRAME*);
extern VALUE *assign_to_name(TOKEN*, FRAME*,VALUE*);
extern VALUE *declare_name(TOKEN*, FRAME*);

VALUE* make_value_int(int t, int val){
    VALUE *value = malloc(sizeof(VALUE));
    if (value == NULL) {perror("fatal: make_value_int failed\n"); exit(1);}

    value->type = t;
    value->integer = val;
    return value;
}

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst)
{
  TAC* ans = (TAC*)malloc(sizeof(TAC));
  if (ans==NULL) {
    printf("Error! memory not allocated.");
    exit(0);
  }
  ans->op = op;
  if(op==tac_endproc){
      return ans;
  }
  if(op==tac_proc){
      ans->src1 = src1;
      ans->src2 = src2;
      return ans;
  }
  ans->src1 = src1;
  ans->src2 = src2;
  ans->dst = dst;
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

VALUE* interpret_tree(NODE *tree, FRAME* e){

    VALUE *left, *right;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
        TOKEN *t = (TOKEN *)tree->left;
        if (t->type == CONSTANT){
            return make_value_int(INT,t->value);
        }
        else if (t->type == IDENTIFIER){
            VALUE *v = lookup_name(t,e);
            if (v==NULL){
                return declare_name(t,e);
            }
            else{return v;}
        }
    }
    char t = (char)tree->type;
    if (isgraph(t) || t==' ') {
        switch(t){
            default: printf("fatal: unknown token type '%c'\n",t); exit(1);
            
            case '~':
                //interpret_tree(tree->left,e);
                return interpret_tree(tree->right,e);
            case 'D':
            //case 'd':
                //interpret_tree(tree->left);
                return interpret_tree(tree->right, e);
            case ';':
                interpret_tree(tree->left,e);
                return interpret_tree(tree->right,e);
            case '=':
                interpret_tree(tree->left,e);
                return assign_to_name(tree->left->left,e,interpret_tree(tree->right,e));   
            case '+':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(INT,left->integer + right->integer);
            case '-':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(INT,left->integer - right->integer);
            case '*':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(INT,left->integer * right->integer);
            case '/':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(INT,left->integer / right->integer);
            case '%':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(INT,left->integer % right->integer);
        }
    }
    switch(tree->type){
        default: printf("fatal: unknown token type '%i'\n", tree->type); exit(1);
        case RETURN:  
            return interpret_tree(tree->left,e);
    } 
}

TAC *gen_tac(NODE *tree, int counter){

    TOKEN *left, *right;
    TAC *tac;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
            TOKEN *t = (TOKEN *)tree->left;
            if (t->type == CONSTANT){
                return new_tac(no_op,t,NULL,new_dest(counter));
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
                tac->next->next = new_tac(tac_endproc,NULL,NULL,NULL);
                return tac;
            case 'd':
                return gen_tac(tree->right,counter++);
            case 'F':
                left = (TOKEN *)tree->left->left;
                right = new_token(CONSTANT);
                right->value = 0;
                return new_tac(tac_proc, left, right,NULL);
            case '+':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_tac(tac_plus,left,right,new_dest(counter));
            case '-':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_tac(tac_minus,left,right,new_dest(counter));
            case '*':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_tac(tac_mult,left,right,new_dest(counter));
            case '/':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_tac(tac_div,left,right,new_dest(counter));
            case '%':
                left = (TOKEN *)tree->left->left;
                right = (TOKEN *)tree->right->left;
                return new_tac(tac_mod,left,right,new_dest(counter));
        }
    }
    switch(tree->type){
    default: printf("fatal: unknown token type '%c'\n", tree->type); exit(1);
    case RETURN:  
        return gen_tac(tree->left,counter++);
    } 

}