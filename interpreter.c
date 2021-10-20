#include "nodes.h"
#include "main.h"
#include <stdio.h>

extern NODE *tree;
extern void print_tree(NODE *tree);

char *interpret_tree(NODE *tree){
    printf("tree received\n");
    print_tree(tree);
    return "tree received in interpreter";
}