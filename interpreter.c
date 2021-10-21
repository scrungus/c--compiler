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

    if(t == 'D'){ 
        interpret_tree(tree->left);
        interpret_tree(tree->right);
    }
    else if (t == 'd'){ 
        interpret_tree(tree->right);
        interpret_tree(tree->left);
    }
    else if (t== 'F'){ 
    }
    switch(tree->type){
    case RETURN:  
        printf("RESULT : %i\n",interpret_tree(tree->left));
    }
    return 1; 
}