#include "nodes.h"
#include "interpreter.h"
#include "environment.h"
#include "main.h"
#include <stdio.h>
#include <ctype.h>
#include "C.tab.h"
#include "value.h"

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

VALUE* interpret_tilde(NODE*tree, FRAME* e){
    TOKEN* t;
    if(tree->left->left->type==INT){
        if(tree->right->type == LEAF){
            t = (TOKEN *)tree->right->left;
            if(lookup_name(t,e) == NULL){return declare_name(t,e);}
            else {printf("error: multiple declarations of %s",t->lexeme);exit(1);}
        }
        else if((char)tree->right->type == '='){
                t = (TOKEN *)tree->right->left->left;
            if(lookup_name(t,e) == NULL){declare_name(t,e);}
            else {printf("error: multiple declarations of variable '%s'\n",t->lexeme);exit(1);}
            return assign_to_name(t,e,interpret_tree(tree->right->right,e));
        }
    }
    interpret_tree(tree->left,e);
    return interpret_tree(tree->right,e);
}

VALUE* interpret_tree(NODE *tree, FRAME* e){

    VALUE *left, *right;
    TOKEN *t;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
        t = (TOKEN *)tree->left;
        if (t->type == CONSTANT){
            return make_value_int(INT,t->value);
        }
        else if (t->type == IDENTIFIER){
            VALUE *v = lookup_name(t,e);
            if (v==NULL){
                printf("error: undefined variable %s\n",t->lexeme);
            }
            else{return v;}
        }
    }
    char c = (char)tree->type;
    if (isgraph(c) || c==' ') {
        switch(c){
            default: printf("fatal: unknown token type '%c'\n",c); exit(1);
            
            case '~':
                return interpret_tilde(tree,e);
            case 'D':
            //case 'd':
                //interpret_tree(tree->left);
                return interpret_tree(tree->right, e);
            case ';':
                interpret_tree(tree->left,e);
                return interpret_tree(tree->right,e);
            case '=':
                interpret_tree(tree->left,e);
                t = (TOKEN *)tree->left->left;
                return assign_to_name(t,e,interpret_tree(tree->right,e));   
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