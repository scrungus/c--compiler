#include "nodes.h"
#include "C.tab.h"
#include <stdlib.h>
#include "test_utilities.h"

NODE* construct_basic_empty_function(){
    NODE *tree = make_node('D',NULL,make_node(RETURN, NULL, NULL));
    return tree;
}
