#include "nodes.h"
#include "main.h"
#include <stdio.h>
#include "C.tab.h"

extern NODE *tree;
extern void print_tree(NODE *tree);

int interpret_tree(NODE *tree){
    if (tree==NULL) {printf("no tree received\n") ; return;}
    if (tree->type==LEAF){
        TOKEN *t = (TOKEN *)tree->left;
        if (t->type == CONSTANT){
            return t->value;
        }
    }
    char t = (char)tree->type;

    switch(t){
        case '~':
        case 'D':
        //case 'd':
            //interpret_tree(tree->left);
            return interpret_tree(tree->right);
        
        case '+':
            return interpret_tree(tree->left)+interpret_tree(tree->right);
        case '-':
            return interpret_tree(tree->left)-interpret_tree(tree->right);
        case '*':
            return interpret_tree(tree->left)*interpret_tree(tree->right);
        case '/':
            return interpret_tree(tree->left)/interpret_tree(tree->right);
        case '%':
            return interpret_tree(tree->left)%interpret_tree(tree->right);
    }
    switch(tree->type){
    case RETURN:  
        return interpret_tree(tree->left);
    } 
}