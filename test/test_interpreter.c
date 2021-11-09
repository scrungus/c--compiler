#include "interpreter.h"
#include "nodes.h"
#include "C.tab.h"
#include "token.h"
#include "assert.h"
#include "test_utilities.h"

extern VALUE* interpret_tree(NODE*,FRAME*);
extern NODE* make_leaf(TOKEN*);
extern NODE* make_node(int, NODE*, NODE*);
extern TOKEN* make_token(int);
extern NODE* construct_basic_empty_function();

int check_val(int op){
    switch(op){
        case '+': return VAL1+VAL1;
        case '-': case '%': return 0;
        case '*': return VAL1*VAL1;
        case '/': return 1;
    }
}

void test_case_return_literal_arithmetic(int op){
    TOKEN *t1 = new_token(CONSTANT); t1->value = VAL1;
    TOKEN *t2 = new_token(CONSTANT); t2->value = VAL1;

    NODE *x1 = make_leaf(t1);
    NODE *x2 = make_leaf(t2);

    NODE* operation = make_node(op, x1,x2);

    NODE* tree = construct_basic_empty_function();
    tree->right->left = operation;

    FRAME* e = malloc(sizeof(FRAME));

    VALUE *result = interpret_tree(tree->right,e);

    assert(result->integer ==check_val(op));
    printf("test for '%c' interpreter literal arithmetic passed!\n",op);
}

int main(void)  {

    //literal arithmetic operation cases:
    test_case_return_literal_arithmetic('+');
    test_case_return_literal_arithmetic('-');
    test_case_return_literal_arithmetic('/');
    test_case_return_literal_arithmetic('%');
    test_case_return_literal_arithmetic('*');
    return 0;
}