#include "nodes.h"
#include "C.tab.h"
#include <stdlib.h>
#include "test_utilities.h"
#include "token.h"
#include "C.tab.h"


NODE* construct_fourarg_empty_function(TOKEN* name, TOKEN* param_name1, TOKEN* param_name2, TOKEN* param_name3, TOKEN* param_name4){
    // parameters
    TOKEN *v = new_token(INT); v->lexeme = "int";
    NODE *vn = make_leaf(v);
    //func name
    NODE *main_node = make_leaf(name);

    //function formals
    NODE *f;

    NODE *p1 = make_leaf(param_name1);
    NODE *p2 = make_leaf(param_name2);
    NODE *p3 = make_leaf(param_name3);
    NODE *p4 = make_leaf(param_name4);

    NODE *t1 = make_node('~',vn,p1);
    NODE *t2 = make_node('~',vn,p2);
    NODE *t3 = make_node('~',vn,p3);
    NODE *t4 = make_node('~',vn,p4);

    NODE *c1 = make_node(',',t1,t2);
    NODE *c2 = make_node(',',c1,t3);
    NODE *c3 = make_node(',',c2,t4);

    f = make_node('F',main_node,c3);
    //function formals and function formals type (null)
    NODE *d = make_node('d', vn,f);
    //Function node
    NODE *tree = make_node('D',d,make_node(RETURN, NULL, NULL));
    return tree;
}

NODE* construct_basic_empty_function(TOKEN* name, int arity, TOKEN* param_name){
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
    else if(arity == 1){
        NODE *nn = make_leaf(param_name);
        NODE *tilde = make_node('~',vn,nn);
        f = make_node('F',main_node,tilde);
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
    TOKEN *main = new_token(IDENTIFIER);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0,NULL);
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
    TOKEN *main = new_token(IDENTIFIER);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0,NULL);
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

NODE* construct_identifier_arithmetic_triple(int op){
    
    NODE* x = construct_declaration("x",VAL1);
    NODE* y = construct_declaration("y",VAL1);
    NODE* z = construct_declaration("z",VAL1);

    NODE* ids1 = make_node(';',x,y);
    NODE* ids2 = make_node(';',ids1,z);

    NODE* operation = make_node(op, ids1->left->right->left,ids1->right->right->left);
    NODE* operation2 = make_node(op, operation,ids2->right->right->left);
 
    TOKEN *main = new_token(IDENTIFIER);
    main->lexeme = "main";
    NODE* tree = construct_sequence_empty_function(main,0);
    tree->right->left = ids2;
    tree->right->right->left = operation2;

    return tree;
}

NODE* construct_return_function_noargs(){

    TOKEN *x = new_token(IDENTIFIER);
    x->lexeme = "x";
    NODE *x_leaf = make_leaf(x);
    NODE* x1 = construct_basic_empty_function(x,0,NULL);
    TOKEN* one = new_token(CONSTANT);
    one->value = 1;
    NODE* one_leaf = make_leaf(one);
    x1->right->left = one_leaf;

    TOKEN *main = new_token(IDENTIFIER);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0,NULL);
    NODE* apply = make_node(APPLY,x_leaf,NULL);
    tree->right->left = apply;

    NODE* tilde = make_node('~',x1,tree);

    return tilde;
}

NODE* construct_return_function_oneargs(int v){

    //function name
    TOKEN *x = new_token(IDENTIFIER);x->lexeme = "x";
    //param name
    TOKEN *n = new_token(IDENTIFIER);n->lexeme = "n";

    NODE *x_leaf = make_leaf(x);
    NODE *n_leaf = make_leaf(n);

    NODE* x1 = construct_basic_empty_function(x,1,n);

    //function adds one to param
    TOKEN* one = new_token(CONSTANT);
    one->value = 1;
    NODE* one_leaf = make_leaf(one);
    NODE* add = make_node('+',n_leaf,one_leaf);
    x1->right->left = add;

    //main
    TOKEN *main = new_token(IDENTIFIER);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0,NULL);
    //parameter supplied
    TOKEN *val = new_token(CONSTANT); val->value = v;
    NODE* v_leaf = make_leaf(val);
    NODE* apply = make_node(APPLY,x_leaf,v_leaf);
    tree->right->left = apply;

    NODE* tilde = make_node('~',x1,tree);

    return tilde;
}

NODE* construct_return_function_fourargs(int v){

    //function name
    TOKEN *x = new_token(IDENTIFIER);x->lexeme = "x";
    NODE *x_leaf = make_leaf(x);
    //param name
    TOKEN *p1 = new_token(IDENTIFIER);p1->lexeme = "p1";
    TOKEN *p2 = new_token(IDENTIFIER);p2->lexeme = "p2";
    TOKEN *p3 = new_token(IDENTIFIER);p3->lexeme = "p3";
    TOKEN *p4 = new_token(IDENTIFIER);p4->lexeme = "p4";

    NODE *p1_leaf = make_leaf(p1);
    NODE *p2_leaf = make_leaf(p2);
    NODE *p3_leaf = make_leaf(p3);
    NODE *p4_leaf = make_leaf(p4);

    NODE* x1 = construct_fourarg_empty_function(x,p1,p2,p3,p4);

    //function adds one to param
    NODE* add = make_node('+',p1_leaf,p2_leaf);
    NODE* add2 = make_node('+',add,p3_leaf);
    NODE* add3 = make_node('+',add2,p4_leaf);
    x1->right->left = add3;

    //main
    TOKEN *main = new_token(IDENTIFIER);
    main->lexeme = "main";
    NODE* tree = construct_basic_empty_function(main,0,NULL);
    //parameter supplied
    TOKEN *v1 = new_token(CONSTANT); v1->value = v;

    NODE* v_leaf = make_leaf(v1);
    NODE* c1 = make_node(',',v_leaf,v_leaf);
    NODE* c2 = make_node(',',c1,v_leaf);
    NODE* c3 = make_node(',',c2,v_leaf);

    NODE* apply = make_node(APPLY,x_leaf,c3);
    tree->right->left = apply;
    
    NODE* tilde = make_node('~',x1,tree);

    return tilde;
}
