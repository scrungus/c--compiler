#include "interpreter.h"
#include "nodes.h"
#include "C.tab.h"
#include "token.h"
#include "assert.h"
#include "test_utilities.h"
#include "gentac.h"

extern BB **gen_tac(NODE*);
extern NODE* make_leaf(TOKEN*);
extern NODE* make_node(int, NODE*, NODE*);
extern TOKEN* make_token(int);
extern NODE* construct_basic_empty_function();
extern NODE* construct_constant_arithmetic(int);
extern NODE* construct_constant_arithmetic_triple(int);
extern NODE* construct_identifier_arithmetic(int);
extern NODE* construct_identifier_arithmetic_triple(int);

int find_tac_code_for_op(int op){
    switch(op) {
        case '+': return tac_plus;
        case '-': return tac_minus;
        case '*': return tac_mult;
        case '/': return tac_div;
        case '%': return tac_mod;
    }
}

void test_case_return_literal_arithmetic(int op){
    
    NODE* tree = construct_constant_arithmetic(op);

    BB **r = gen_tac(tree);
    TAC* result = (*r)->leader;
    TOKEN *dst1, *dst2;
    
    assert(result->op == tac_proc);
    assert(result->proc.arity==0);

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst1 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst2 = result->ld.dst;

    result = result->next;
    assert(result->op == find_tac_code_for_op(op));
    assert(result->stac.src1 == dst1);
    assert(result->stac.src2 == dst2);
    dst1 = result->stac.dst;

    result = result->next;
    assert(result->op = tac_rtn);
    assert(result->rtn.v == dst1);

}

void test_case_return_literal_arithmetic_triple(int op){

    NODE* tree = construct_constant_arithmetic_triple(op);

    BB **r = gen_tac(tree);
    TAC* result = (*r)->leader;
    TOKEN *dst1, *dst2;
    
    assert(result->op == tac_proc);
    assert(result->proc.arity==0);

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst1 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst2 = result->ld.dst;

    result = result->next;
    assert(result->op == find_tac_code_for_op(op));
    assert(result->stac.src1 == dst1);
    assert(result->stac.src2 == dst2);
    dst2 = result->stac.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst1 = result->ld.dst;

    result = result->next;
    assert(result->op == find_tac_code_for_op(op));
    assert(result->stac.src1 == dst2);
    assert(result->stac.src2 == dst1);
    dst1 = result->stac.dst;

    result = result->next;
    assert(result->op = tac_rtn);
    assert(result->rtn.v == dst1);
}

void test_case_return_variable_arithmetic(int op){

    NODE* tree = construct_identifier_arithmetic(op);

    BB **r = gen_tac(tree);
    TAC* result = (*r)->leader;
    TOKEN *dst1, *dst2, *dst3, *dst4;
    
    assert(result->op == tac_proc);
    assert(result->proc.arity==0);

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst1 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_store);
    assert(result->ld.src1 == dst1);
    dst2 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst3 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_store);
    assert(result->ld.src1 == dst3);
    dst4 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1 == dst2);
    dst1 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1 == dst4);
    dst2 = result->ld.dst;

    result = result->next;
    assert(result->op == find_tac_code_for_op(op));
    assert(result->stac.src1 == dst1);
    assert(result->stac.src2 == dst2);
    dst1 = result->stac.dst;

    result = result->next;
    assert(result->op = tac_rtn);
    assert(result->rtn.v == dst1);
}

void test_case_return_variable_arithmetic_triple(int op){

    NODE* tree = construct_identifier_arithmetic_triple(op);

    BB **r = gen_tac(tree);
    TAC* result = (*r)->leader;
    TOKEN *dst1, *dst2, *dst3, *dst4, *dst5;
    
    assert(result->op == tac_proc);
    assert(result->proc.arity==0);

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst1 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_store);
    assert(result->ld.src1 == dst1);
    dst2 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst3 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_store);
    assert(result->ld.src1 == dst3);
    dst4 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1->value == VAL1);
    dst5 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_store);
    assert(result->ld.src1 == dst5);
    dst5 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1 == dst2);
    dst1 = result->ld.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1 == dst4);
    dst2 = result->ld.dst;

    result = result->next;
    assert(result->op == find_tac_code_for_op(op));
    assert(result->stac.src1 == dst1);
    assert(result->stac.src2 == dst2);
    dst1 = result->stac.dst;

    result = result->next;
    assert(result->op == tac_load);
    assert(result->ld.src1 == dst5);
    dst2 = result->ld.dst;

    result = result->next;
    assert(result->op == find_tac_code_for_op(op));
    assert(result->stac.src1 == dst1);
    assert(result->stac.src2 == dst2);
    dst1 = result->stac.dst;

    result = result->next;
    assert(result->op = tac_rtn);
    assert(result->rtn.v == dst1);
}

int main(void) {
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

    test_case_return_variable_arithmetic_triple('+');
    test_case_return_variable_arithmetic_triple('-');
    test_case_return_variable_arithmetic_triple('/');
    test_case_return_variable_arithmetic_triple('%');
    test_case_return_variable_arithmetic_triple('*');
    printf("Triple variable arithmetic tests passed\n");

    return 0;
}