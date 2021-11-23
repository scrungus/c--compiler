#include "nodes.h"
#include "C.tab.h"
#include <stdlib.h>
#include "test_utilities.h"
#include "token.h"
#include "C.tab.h"

NODE* construct_basic_empty_function(TOKEN* name, int arity){
    // parameters
    TOKEN *v = new_token(INT); v->lexeme = "int";
    NODE *vn = make_leaf(v);
    //func name
    NODE *main_node = make_leaf(name);

    //function formals
    NODE *f;
    if(arity == 0){
        f = make_node('F',main_node,NULL); 
    }
    //function formals and function formals type (null)
    NODE *d = make_node('d', vn,f);
    //Function node
    NODE *tree = make_node('D',d,make_node(RETURN, NULL, NULL));
    return tree;
}

NODE* construct_sequence_empty_function(TOKEN* name, int arity){
    // parameters
    TOKEN *v = new_token(INT); v->lexeme = "int";
    NODE *vn = make_leaf(v);
    //func name
    NODE *main_node = make_leaf(name);

    //function formals
    NODE *f;
    if(arity == 0){
        f = make_node('F',main_node,NULL); 
    }
    //function formals and function formals type (null)
    NODE *d = make_node('d', vn,f);
    //Function node
    NODE* rtn = make_node(RETURN, NULL, NULL);
    NODE* seq = make_node(';',NULL,rtn);
    NODE *tree = make_node('D',d,seq);
    return tree;
}

NODE* construct_constant_arithmetic(int op){
    TOKEN *t1 = new_token(CONSTANT); t1->value = VAL1;
    TOKEN *t2 = new_token(CONSTANT); t2->value = VAL1;

    NODE *x1 = make_leaf(t1);
    NODE *x2 = make_leaf(t2);

    NODE* operation = make_node(op, x1,x2);
    TOKEN *main = new_token(STRING_LITERAL);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0);
    tree->right->left = operation;

    return tree;
}

NODE* construct_declaration(char* name, int value){
    TOKEN *t1 = new_token(IDENTIFIER); t1->lexeme = name;
    TOKEN *t2 = new_token(CONSTANT); t2->value = value;
    TOKEN *t3 = new_token(INT); t3->lexeme = "int";

    NODE *x1 = make_leaf(t1);
    NODE *x2 = make_leaf(t2);
    NODE *x3 = make_leaf(t3);

    NODE* equals = make_node('=',x1,x2);

    NODE* tilde = make_node('~',x3,equals);

    return tilde;
}

NODE* construct_constant_arithmetic_triple(int op){
    TOKEN *t1 = new_token(CONSTANT); t1->value = VAL1;
    TOKEN *t2 = new_token(CONSTANT); t2->value = VAL1;
    TOKEN *t3 = new_token(CONSTANT); t3->value = VAL1;

    NODE *x1 = make_leaf(t1);
    NODE *x2 = make_leaf(t2);
    NODE *x3 = make_leaf(t3);

    NODE* operation = make_node(op, x1,x2);
    NODE* operation2 = make_node(op, operation, x3);
    TOKEN *main = new_token(STRING_LITERAL);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0);
    tree->right->left = operation2;

    return tree;
}

NODE* construct_identifier_arithmetic(int op){
    
    NODE* x = construct_declaration("x",VAL1);
    NODE* y = construct_declaration("y",VAL1);

    NODE* ids = make_node(';',x,y);

    NODE* operation = make_node(op, ids->left->right->left,ids->right->right->left);
 
    TOKEN *main = new_token(IDENTIFIER);
    main->lexeme = "main";
    NODE* tree = construct_sequence_empty_function(main,0);
    tree->right->left = ids;
    tree->right->right->left = operation;

    return tree;
}
