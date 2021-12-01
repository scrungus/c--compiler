#include <stdlib.h>
#include <stdio.h>
#include "mc_env.h"
#include "string.h"

TOKEN *lookup_loc(TOKEN * x, FRME * frame){
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            if(bindings->name == x){
                return bindings->loc;
            }
            bindings = bindings->next;
        }
        frame = frame->next;
    }
    return NULL;
}

int reg_in_use(TOKEN * x, FRME * frame){
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            if(bindings->loc != NULL && strcmp(bindings->loc->lexeme,x->lexeme)==0){
                return 1;
            }
            bindings = bindings->next;
        }
        frame = frame->next;
    }
    return 0;
}

TOKEN *assign_to_var(TOKEN * x, FRME * frame, TOKEN* loc){
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            if(bindings->name == x){
                bindings->loc = loc;
                return loc;
            }
            bindings = bindings->next;
        }
        frame = frame->next;
    }
    printf("fatal: unbound variable!\n");exit(1);
}

void declare_var(TOKEN * x, FRME * frame){
    BNDING *bindings = frame->bindings;
    BNDING *new = malloc(sizeof(BNDING));
    if(new != NULL){
        new->name = x;
        new->loc = NULL;
        new->next = bindings;
        frame->bindings=new;
        return;
    }
    printf("fatal: binding creation failed!\n");
}
/* TOKEN *declare_func(TOKEN * x, VALUE* val, FRAME * frame){
    BINDING *bindings = frame->bindings;
    BINDING *new = malloc(sizeof(BINDING));
    if(new != NULL){
        new->name = x;
        new->value = val;
        new->next = bindings;
        frame->bindings=new;
        return new->value;
    }
    printf("fatal: binding creation failed!\n");
} */

