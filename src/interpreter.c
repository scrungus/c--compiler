#include "nodes.h"
#include "interpreter.h"
#include "environment.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "C.tab.h"
#include "value.h"

extern NODE *tree;
extern void print_tree(NODE *tree);
extern VALUE *lookup_name(TOKEN*, FRAME*);
extern VALUE *assign_to_name(TOKEN*, FRAME*,VALUE*);
extern VALUE *declare_name(TOKEN*, FRAME*);
extern VALUE *declare_func(TOKEN*,VALUE*, FRAME*);

VALUE* new_closure(NODE* t, FRAME* e){
    CLOSURE* c = malloc(sizeof(CLOSURE));
    VALUE* v = malloc(sizeof(VALUE));
    if(c == NULL || v == NULL){printf("fatal: cannot allocate memory for closure\n");exit(1);}
    c->code=t;
    c->env=e;
    v->type = CLOS;
    v->closure = c;
    return v;
}

FRAME *extend_frame(FRAME* e, NODE *ids, NODE *args){
    FRAME* new_frame = malloc(sizeof(FRAME));
    BINDING *bindings = NULL;
    new_frame->bindings = bindings;
    //while (ids != NULL && args != NULL) {
       declare_name((TOKEN*)ids->right->left,new_frame);
       assign_to_name((TOKEN*)ids->right->right,new_frame,interpret_tree(args,e));
    return new_frame;
}

VALUE* make_value_int(int val){
    VALUE *value = malloc(sizeof(VALUE));
    if (value == NULL) {perror("fatal: make_value_int failed\n"); exit(1);}

    value->type = INT;
    value->integer = val;
    return value;
}

VALUE* make_value_bool(int val){
    VALUE *value = malloc(sizeof(VALUE));
    if (value == NULL) {perror("fatal: make_value_int failed\n"); exit(1);}

    value->type = BOOL;
    value->boolean = val;
    return value;
}

VALUE* interpret_tilde(NODE*tree, FRAME* e){
    TOKEN* t;
    if(tree->left->left->type==INT){
        if(tree->right->type == LEAF){
            t = (TOKEN *)tree->right->left;
            if(lookup_name(t,e) == NULL){return declare_name(t,e);}
            else {printf("error: multiple declarations of %s",t->lexeme);exit(1);}
        }
        else if((char)tree->right->type == '='){
                t = (TOKEN *)tree->right->left->left;
            if(lookup_name(t,e) == NULL){declare_name(t,e);}
            else {printf("error: multiple declarations of variable '%s'\n",t->lexeme);exit(1);}
            return assign_to_name(t,e,interpret_tree(tree->right->right,e));
        }
    }
    interpret_tree(tree->left,e);
    return interpret_tree(tree->right,e);
}

VALUE* if_method(NODE* tree, FRAME* e){
    VALUE* condition = interpret_tree(tree->left,e);
    NODE* consequent = tree->right->left;
    NODE* alternative = tree->right->right;
    if(condition->type == BOOL){
        if(condition->boolean){
            return interpret_tree(consequent,e);
        }
        else{
            return interpret_tree(alternative,e);
        }
    }
    else{printf("error: condition is not boolean value\n");exit(1);}
}

VALUE* interpret(NODE* tree){
    FRAME* e = malloc(sizeof(FRAME));
    interpret_tree(tree,e);
    while(e != NULL){
        while (e->bindings != NULL){
            if(strcmp(e->bindings->name->lexeme,"main")==0){
                NODE* body = e->bindings->value->closure->code->right;
                return interpret_tree(body,e);
            }
            e->bindings = e->bindings->next;
        }
        e = e->next;
    }
    printf("No main function. exiting...\n");exit(1);

}
VALUE* interpret_tree(NODE *tree, FRAME* e){

    VALUE *left, *right;
    TOKEN *t;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
        t = (TOKEN *)tree->left;
        if (t->type == CONSTANT){
            return make_value_int(t->value);
        }
        else if (t->type == IDENTIFIER){
            VALUE *v = lookup_name(t,e);
            if (v==NULL){
                printf("error: undefined variable %s\n",t->lexeme);
            }
            else{return v;}
        }
    }
    char c = (char)tree->type;
    if (isgraph(c) || c==' ') {
        switch(c){
            default: printf("fatal: unknown token type '%c'\n",c); exit(1);
            
            case '~':
                return interpret_tilde(tree,e);
            case 'D':
            //case 'd':
                t = (TOKEN *)tree->left->right->left->left;
                return declare_func(t,new_closure(tree,e),e);
            case ';':
                interpret_tree(tree->left,e);
                return interpret_tree(tree->right,e);
            case '=':
                interpret_tree(tree->left,e);
                t = (TOKEN *)tree->left->left;
                return assign_to_name(t,e,interpret_tree(tree->right,e));   
            case '+':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(left->integer + right->integer);
            case '-':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(left->integer - right->integer);
            case '*':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(left->integer * right->integer);
            case '/':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(left->integer / right->integer);
            case '%':
                left = interpret_tree(tree->left,e);
                right = interpret_tree(tree->right,e);
                return make_value_int(left->integer % right->integer);
        }
    }
    switch(tree->type){
        default: printf("fatal: unknown token type '%i'\n", tree->type); exit(1);
        case RETURN:  
            return interpret_tree(tree->left,e);
        case IF:
            return if_method(tree,e);
        case LE_OP:
            if(interpret_tree(tree->left,e)->integer <= interpret_tree(tree->right,e)->integer){
                return make_value_bool(1);
            }
            else{return make_value_bool(0);}
        case GE_OP:
        if(interpret_tree(tree->left,e)->integer >= interpret_tree(tree->right,e)->integer){
                return make_value_bool(1);
            }
            else{return make_value_bool(0);}
        case EQ_OP:
            if(interpret_tree(tree->left,e)->integer == interpret_tree(tree->right,e)->integer){
                    return make_value_bool(1);
                }
                else{return make_value_bool(0);}
        case NE_OP:
            if(interpret_tree(tree->left,e)->integer != interpret_tree(tree->right,e)->integer){
                    return make_value_bool(1);
                }
                else{return make_value_bool(0);}
    } 
}