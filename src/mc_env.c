#include <stdlib.h>
#include <stdio.h>
#include "mc_env.h"
#include "string.h"
#include "C.tab.h"

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

TOKENLIST* list_vars(FRME * frame){
    TOKENLIST * tks = malloc(sizeof(TOKENLIST));
    TOKENLIST* head = tks;
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            tks->name = bindings->loc;
            bindings = bindings->next;
            tks->next = malloc(sizeof(TOKENLIST));
            tks = tks->next;
        }
        frame = frame->next;
    }
    return tks;
}

TOKEN* lookup_reg(int x, FRME * frame){
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            if(bindings->loc != NULL && bindings->loc->value == x){
                return bindings->name;
            }
            bindings = bindings->next;
        }
        frame = frame->next;
    }
    return 0;
}

void delete_loc(TOKEN * x, FRME * frame){
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        BNDING *head = bindings;
        BNDING *prev = NULL;
        while(bindings != NULL){
            if(bindings->name == x){
                if(prev == NULL){
                    frame->bindings = bindings->next;
                }
                else{
                    prev->next = bindings->next;
                    frame->bindings = head;
                }
                return;
            }
            prev = bindings;
            bindings = bindings->next;
        }
        frame = frame->next;
    }
}

void delete_constants(FRME* frame){
     while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            if(bindings->name->type == CONSTANT){
                delete_loc(bindings->name,frame);
            }
            bindings = bindings->next;
        }
        frame = frame->next;
    }
}

int reg_in_use(int x, FRME * frame){
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            if(bindings->loc != NULL && bindings->loc->value == x){
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
                if(reg_in_use(loc->value,frame)){
                    delete_loc(lookup_reg(loc->value,frame),frame);
                }
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

