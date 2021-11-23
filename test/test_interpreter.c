#include "interpreter.h"
#include "nodes.h"
#include "C.tab.h"
#include "token.h"
#include "assert.h"
#include "test_utilities.h"
#include <ctype.h>

extern VALUE* interpret(NODE*);
extern NODE* make_leaf(TOKEN*);
extern NODE* make_node(int, NODE*, NODE*);
extern TOKEN* make_token(int);
extern NODE* construct_basic_empty_function();
extern NODE* construct_constant_arithmetic(int);
extern NODE* construct_constant_arithmetic_triple(int);
extern NODE* construct_identifier_arithmetic(int);

int check_val(int op){
    switch(op){
        case '+': return VAL1+VAL1;
        case '-': return VAL1-VAL1;
        case '%': return VAL1%VAL1;
        case '*': return VAL1*VAL1;
        case '/': return VAL1/VAL1;
    }
}

int check_val_triple(int op){
    switch(op){
        case '+': return VAL1+VAL1+VAL1;
        case '-': return VAL1-VAL1-VAL1;
        case '%': return VAL1%VAL1%VAL1;
        case '*': return VAL1*VAL1*VAL1;
        case '/': return VAL1/VAL1/VAL1;
    }
}

void test_case_return_literal_arithmetic(int op){
    
    NODE* tree = construct_constant_arithmetic(op);

    VALUE *result = interpret(tree);

    assert(result->integer ==check_val(op));
}

void test_case_return_literal_arithmetic_triple(int op){
    NODE* tree = construct_constant_arithmetic_triple(op);
    VALUE *result = interpret(tree);

    assert(result->integer ==check_val_triple(op));
}

void test_case_return_variable_arithmetic(int op){

    NODE* tree = construct_identifier_arithmetic(op);

    VALUE *result = interpret(tree);

    assert(result->integer ==check_val(op));
}


int main(void)  {

    //literal arithmetic operation cases:
    test_case_return_literal_arithmetic('+');
    test_case_return_literal_arithmetic('-');
    test_case_return_literal_arithmetic('/');
    test_case_return_literal_arithmetic('%');
    test_case_return_literal_arithmetic('*');
    printf("Simple literal arithmetic tests passed\n");

    test_case_return_literal_arithmetic_triple('+');
    test_case_return_literal_arithmetic_triple('-');
    test_case_return_literal_arithmetic_triple('/');
    test_case_return_literal_arithmetic_triple('%');
    test_case_return_literal_arithmetic_triple('*');
    printf("Triple literal arithmetic tests passed\n");

    test_case_return_variable_arithmetic('+');
    test_case_return_variable_arithmetic('-');
    test_case_return_variable_arithmetic('/');
    test_case_return_variable_arithmetic('%');
    test_case_return_variable_arithmetic('*');
    printf("Simple variable arithmetic tests passed\n");


    return 0;
}