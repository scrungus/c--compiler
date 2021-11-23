#include "nodes.h"
#include "C.tab.h"
#include <stdlib.h>
#include "test_utilities.h"
#include "token.h"
#include "C.tab.h"

NODE* construct_basic_empty_function(TOKEN* name, int arity){
    //void parameters
    TOKEN *v = new_token(VOID);
    NODE *vn = make_leaf(v);
    //func name
    NODE *main_node = make_leaf(name);

    //function formals
    NODE *f;
    if(arity == 0){
        f = make_node('F',main_node,vn); 
    }
    //function formals and function formals type (null)
    NODE *d = make_node('d', NULL,f);
    //Function node
    NODE *tree = make_node('D',d,make_node(RETURN, NULL, NULL));
    return tree;
}
