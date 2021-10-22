#include "nodes.h"
#include "interpreter.h"
#include "main.h"
#include <stdio.h>
#include "C.tab.h"

extern NODE *tree;
extern void print_tree(NODE *tree);


VALUE* make_value_int(int t, int val){
    VALUE *value = malloc(sizeof(VALUE));
    if (value == NULL) {perror("fatal: make_value_int failed\n"); exit(1);}

    value->type = t;
    value->integer = val;
}

VALUE* interpret_tree(NODE *tree){

    VALUE *left, *right;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
        TOKEN *t = (TOKEN *)tree->left;
        if (t->type == CONSTANT){
            return make_value_int(INT,t->value);
        }
    }
    char t = (char)tree->type;

    switch(t){
        default: printf("fatal: unknown token type '%c'\n",t); exit(1);
        
        case '~':
        case 'D':
        //case 'd':
            //interpret_tree(tree->left);
            return interpret_tree(tree->right);
        
        case '+':
            left = interpret_tree(tree->left);
            right = interpret_tree(tree->right);
            return make_value_int(INT,left->integer + right->integer);
        case '-':
            left = interpret_tree(tree->left);
            right = interpret_tree(tree->right);
            return make_value_int(INT,left->integer - right->integer);
        case '*':
            left = interpret_tree(tree->left);
            right = interpret_tree(tree->right);
            return make_value_int(INT,left->integer * right->integer);
        case '/':
            left = interpret_tree(tree->left);
            right = interpret_tree(tree->right);
            return make_value_int(INT,left->integer / right->integer);
        case '%':
            left = interpret_tree(tree->left);
            right = interpret_tree(tree->right);
            return make_value_int(INT,left->integer % right->integer);
    }
    switch(tree->type){
    default: printf("fatal: unknown token type '%c'\n", tree->type); exit(1);
    case RETURN:  
        return interpret_tree(tree->left);
    } 
}