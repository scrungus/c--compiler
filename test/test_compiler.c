#include "interpreter.h"
#include "nodes.h"
#include "C.tab.h"
#include "token.h"
#include "assert.h"
#include "test_utilities.h"
#include "gentac.h"

extern TAC *gen_tac(NODE*);
extern NODE* make_leaf(TOKEN*);
extern NODE* make_node(int, NODE*, NODE*);
extern TOKEN* make_token(int);
extern NODE* construct_basic_empty_function();

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
    TOKEN *t1 = new_token(CONSTANT); t1->value = VAL1;
    TOKEN *t2 = new_token(CONSTANT); t2->value = VAL1;

    NODE *x1 = make_leaf(t1);
    NODE *x2 = make_leaf(t2);

    NODE* operation = make_node(op, x1,x2);
    TOKEN *main = new_token(STRING_LITERAL);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0);
    tree->right->left = operation;

    TAC *result = gen_tac(tree);
    TOKEN *dst1, *dst2;
    
    assert(result->op == tac_proc);
    assert(result->proc.name==main);
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
    assert(result->stac.dst != NULL);
    printf("test result for '%c' TAC code literal arithmetic passed!\n",op);
}

int main(void) {
    test_case_return_literal_arithmetic('+');
    test_case_return_literal_arithmetic('-');
    test_case_return_literal_arithmetic('/');
    test_case_return_literal_arithmetic('%');
    test_case_return_literal_arithmetic('*');
    return 0;
}