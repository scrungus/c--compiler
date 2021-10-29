#include "nodes.h"
#include "C.tab.h"
#include <stdlib.h>
#include "test_utilities.h"
#include "token.h"
#include "C.tab.h"

NODE* construct_basic_empty_function(){
    TOKEN *main = new_token(STRING_LITERAL);
    main->lexeme = "main";
    NODE *main_node = make_leaf(main);
    NODE *f = make_node('F',main_node,NULL); 
    NODE *d = make_node('d', NULL,f);
    NODE *tree = make_node('D',d,make_node(RETURN, NULL, NULL));
    return tree;
}
