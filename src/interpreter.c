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
extern VALUE *lookup_name_curr_frame(TOKEN*, FRAME*);
extern VALUE *assign_to_name(TOKEN*, FRAME*,VALUE*);
extern VALUE *declare_name(TOKEN*, FRAME*);
extern VALUE *declare_func(TOKEN*,VALUE*, FRAME*);

int r_early = 0, in_seq = 0;

//built-ins

void print_int(VALUE *v){
    if(v->type == CONSTANT){
         printf("%d\n", v->integer);
    }
    else{
        printf("fatal: print_int(): invalid int type\n"); exit(1);
    }
}

void print_string(VALUE *v){
    if(v->type == STRING_LITERAL){
        printf("%s\n",v->string);
    }
    else{
        printf("fatal: print_string(): invalid string type\n"); exit(1);
    }
}

VALUE* read_int(){
    int n;
    int c;
    char buf[2];
    printf("> ");
    clearerr(stdin);
    fgets(buf, 2,stdin);   
    n = strtol(buf, NULL,10);
    return make_value_int(n);
}

int is_builtin(TOKEN* name){
    char *ps = "print_string";
    char *p = "print_int";
    char *r = "read_int";

    if(!strcmp(ps,name->lexeme)||!strcmp(p,name->lexeme)||!strcmp(r,name->lexeme)){
        return 1;
    }
    return 0;
}

VALUE* call_builtin(TOKEN* name, VALUELIST* args){
    char *ps = "print_string";
    char *p = "print_int";
    char *r = "read_int";

    if(!strcmp(ps,name->lexeme)){
        print_string(args->value);
        return NULL;
    }
    if(!strcmp(p,name->lexeme)){
        print_int(args->value);
        return NULL;
    }
    if(!strcmp(r,name->lexeme)){
        return read_int();
    }
}

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

TOKENLIST* find_tokens(NODE* ids){
    TOKENLIST* tokens = malloc(sizeof(TOKENLIST));
    if((char)ids->type == '~'){
        tokens->name = (TOKEN*)ids->right->left;
        return tokens;
    }
    else{
        if((char)ids->type == ','){
            tokens->name = (TOKEN*)ids->right->right->left;
            tokens->next = find_tokens(ids->left);
            return tokens;
        }
    }
}

FRAME *extend_frame(FRAME* e, NODE *ids, VALUELIST *args){

    FRAME* new_frame = malloc(sizeof(FRAME));
    if(ids == NULL && args == NULL) {return new_frame;}
    BINDING *bindings = NULL;
    new_frame->bindings = bindings;
    //while (ids != NULL && args != NULL) {
       TOKENLIST* tokens = find_tokens(ids);
       while(tokens != NULL && args != NULL){
            declare_name(tokens->name,new_frame);
            assign_to_name(tokens->name,new_frame,args->value);
            tokens=tokens->next;
            args = args->next;
       }
       if(!(tokens == NULL && args == NULL)){
           printf("error: invalid number of arguments and/or tokens, exiting...\n");exit(1);
       }
    return new_frame;
}

VALUE* make_value_int(int val){
    VALUE *value = malloc(sizeof(VALUE));
    if (value == NULL) {perror("fatal: make_value_int failed\n"); exit(1);}

    value->type = CONSTANT;
    value->integer = val;
    return value;
}

VALUE* make_value_bool(int val){
    VALUE *value = malloc(sizeof(VALUE));
    if (value == NULL) {perror("fatal: make_value_bool failed\n"); exit(1);}

    value->type = BOOL;
    value->boolean = val;
    return value;
}

VALUE* make_value_string(char* str){
    VALUE *value = malloc(sizeof(VALUE));
    if (value == NULL) {perror("fatal: make_value_string failed\n"); exit(1);}

    value->type = STRING_LITERAL;
    value->string = malloc(strlen(str));
    strcpy(value->string, str);
    return value;
}

VALUE* interpret_tilde(NODE*tree, FRAME* e){
    TOKEN* t;
    if(tree->left->left->type==INT || tree->left->left->type==FUNCTION || tree->left->left->type==STRING_LITERAL){
        if(tree->right->type == LEAF){
            t = (TOKEN *)tree->right->left;
            if(lookup_name(t,e) == NULL){return declare_name(t,e);}
            else {printf("error: multiple declarations of %s",t->lexeme);exit(1);}
        }
        else if((char)tree->right->type == '='){
                t = (TOKEN *)tree->right->left->left;
            if(lookup_name_curr_frame(t,e) == NULL){declare_name(t,e);}
            else {printf("error: multiple declarations of variable '%s'\n",t->lexeme);exit(1);}
            return assign_to_name(t,e,interpret_tree(tree->right->right,e));
        }
    }
    interpret_tree(tree->left,e);
    return interpret_tree(tree->right,e);
}

VALUE* if_method(NODE* tree, FRAME* e){
    VALUE* condition = interpret_tree(tree->left,e);
    if(tree->right->type == ELSE){
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
    else{
        NODE* consequent = tree->right;
        if(condition->type == BOOL){
            if(condition->boolean){
                return interpret_tree(consequent,e);
            }
        }
        else{printf("error: condition is not boolean value\n");exit(1);}
    }
}

CLOSURE *find_func(TOKEN* name, FRAME* e){
    FRAME *ef = e;
    BINDING* bindings;
     while(ef != NULL){
        bindings = ef->bindings;
        while (bindings != NULL){
            if(bindings->name ==  name){
               return bindings->value->closure;
            }
            bindings = bindings->next;
        }
         ef = ef->next;
     }
     printf("No function %s in scope, exiting...\n",name->lexeme);exit(1);
}

NODE* formals(CLOSURE* f){
    return f->code->left->right->right;
}

VALUE *call(NODE* name, FRAME* e, VALUELIST* args){
    TOKEN* t = (TOKEN *)name;
    if(is_builtin(t)){
        return call_builtin(t,args);
    }
    CLOSURE *f = find_func(t,e);
    FRAME* ef = extend_frame(e,formals(f),args);
    ef->next = f->env;
    return interpret_tree(f->code->right,ef);
}

VALUELIST* find_curr_values(NODE *t, FRAME* e){
    VALUELIST *values = malloc(sizeof(VALUELIST));
    if(t == NULL) return NULL;
    char c = (char)t->type;
    if(t->type == LEAF || c == '*' || c == '+' || c == '-' || c == '%'|| c == '/' || t->type == APPLY){
        values->value = interpret_tree(t,e);
        values->next = NULL;
        return values;
    }
    else if((char)t->type == ','){
            values->value = interpret_tree(t->right,e);
            values->next = find_curr_values(t->left,e);
            return values;
    }
    else{
        printf("fatal: invalid parameter in call.\n");exit(1);
    }
}

VALUE* interpret(NODE* tree){
    FRAME* e = malloc(sizeof(FRAME));
    interpret_tree(tree,e);
    FRAME *ef = e;
    while(ef != NULL){
        BINDING* bindings = e->bindings;
        while (bindings != NULL){
            if(strcmp(bindings->name->lexeme,"main")==0){
                return call(bindings->name,e,NULL);
            }
            bindings = bindings->next;
        }
        ef = e->next;
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
        else if (t->type == STRING_LITERAL){
            return make_value_string(t->lexeme);
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
                in_seq = 1;
                if(tree->left != NULL){
                    left = interpret_tree(tree->left,e); //HOW DO YOU STOP EXECUTING BELOW IF THIS RETURNS ??
                    if(r_early){
                        return left;
                    }
                }
                in_seq = 0;
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
            case '>':
                if(interpret_tree(tree->left,e)->integer > interpret_tree(tree->right,e)->integer){
                    return make_value_bool(1);
                }
                else{return make_value_bool(0);}
            case '<':
                if(interpret_tree(tree->left,e)->integer < interpret_tree(tree->right,e)->integer){
                    return make_value_bool(1);
                }
                else{return make_value_bool(0);}
        }
    }
    switch(tree->type){
        default: printf("fatal: unknown token type '%i'\n", tree->type); exit(1);
        case RETURN:
            if(in_seq){
                r_early = 1;
            }  
            return interpret_tree(tree->left,e);
        case IF:
            return if_method(tree,e);
        case APPLY:
            return call(tree->left->left,e,find_curr_values(tree->right,e));
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