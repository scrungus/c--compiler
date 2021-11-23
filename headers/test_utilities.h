#include "nodes.h"

#ifndef TEST_UTILITIES
#define TEST_UTILITIES

enum { 
    VAL1 = 5,
    VAL2 = 3,
    VAL3 = 1};

NODE* construct_basic_empty_function();
NODE* construct_constant_arithmetic(int);
NODE* construct_constant_arithmetic_triple(int);
void print_tree(NODE*);

#endif