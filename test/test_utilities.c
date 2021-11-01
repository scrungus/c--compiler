#include "nodes.h"
#include "C.tab.h"
#include <stdlib.h>
#include "test_utilities.h"
#include "token.h"
#include "C.tab.h"

NODE* construct_basic_empty_function(TOKEN* name, int arity){
    TOKEN *v = new_token(VOID);
    NODE *main_node = make_leaf(name);
    NODE *f;
    if(arity == 0){
        f = make_node('F',main_node,v); 
    }
    NODE *d = make_node('d', NULL,f);
    NODE *tree = make_node('D',d,make_node(RETURN, NULL, NULL));
    return tree;
}
