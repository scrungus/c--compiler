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
extern NODE* construct_identifier_arithmetic_triple(int);
extern NODE* construct_return_function_noargs();
extern NODE* construct_return_function_oneargs(int);
extern NODE* construct_return_function_fourargs(int);

char *named(int t)
{
    static char b[100];
    if (isgraph(t) || t==' ') {
      sprintf(b, "%c", t);
      return b;
    }
    switch (t) {
      default: return "???";
    case IDENTIFIER:
      return "id";
    case CONSTANT:
      return "constant";
    case STRING_LITERAL:
      return "string";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case EXTERN:
      return "extern";
    case AUTO:
      return "auto";
    case INT:
      return "int";
    case VOID:
      return "void";
    case APPLY:
      return "apply";
    case LEAF:
      return "leaf";
    case IF:
      return "if";
    case ELSE:
      return "else";
    case WHILE:
      return "while";
    case CONTINUE:
      return "continue";
    case BREAK:
      return "break";
    case RETURN:
      return "return";
    }
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;
    for (i=0; i<level; i++) putchar(' ');
    if (t->type == CONSTANT) printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL) printf("\"%s\"\n", t->lexeme);
    else if (t) puts(t->lexeme);
}

void print_tree0(NODE *tree, int level)
{
    int i;
    if (tree==NULL) return;
    if (tree->type==LEAF) {
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
      printf("%s\n",named(tree->type));
/*       if (tree->type=='~') { */
/*         for(i=0; i<level+2; i++) putchar(' '); */
/*         printf("%p\n", tree->left); */
/*       } */
/*       else */
        print_tree0(tree->left, level+2);
      print_tree0(tree->right, level+2);
    }
}

void print_tree(NODE *tree)
{
    print_tree0(tree, 0);
}

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

void test_case_return_variable_arithmetic_triple(int op){

    NODE* tree = construct_identifier_arithmetic_triple(op);
    VALUE *result = interpret(tree);

    assert(result->integer ==check_val_triple(op));
}

void test_case_return_function_noargs(){
    NODE* tree = construct_return_function_noargs();
    VALUE *result = interpret(tree);
    assert(result->integer == 1);
}

void test_case_return_function_oneargs(){
    NODE* tree = construct_return_function_oneargs(VAL1);
    VALUE *result = interpret(tree);
    assert(result->integer == VAL1+1);
}

void test_case_return_function_fourargs(){
    NODE* tree = construct_return_function_fourargs(VAL1);
    VALUE *result = interpret(tree);
    assert(result->integer == VAL1+VAL1+VAL1+VAL1);
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

    test_case_return_variable_arithmetic_triple('+');
    test_case_return_variable_arithmetic_triple('-');
    test_case_return_variable_arithmetic_triple('/');
    test_case_return_variable_arithmetic_triple('%');
    test_case_return_variable_arithmetic_triple('*');
    printf("Triple variable arithmetic tests passed\n");

    test_case_return_function_noargs();
    printf("Function (no args) as return value test passed\n");

    test_case_return_function_oneargs();
    printf("Function (one args) as return value test passed\n");

    test_case_return_function_fourargs();
    printf("Function (four args) as return value test passed\n");

    

    return 0;
}