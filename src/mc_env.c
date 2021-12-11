#include <stdlib.h>
#include <stdio.h>
#include "mc_env.h"
#include "string.h"
#include "C.tab.h"
#include "genmc.h"

extern TOKEN * new_dst(FRME *);

TOKEN *lookup_loc(TOKEN * x, FRME * frame){
    while(frame != NULL){
        BNDING *bindings = frame->bindings;
        while(bindings != NULL){
            if(bindings->name == x){
                return bindings->loc;
            }
            bindings = bindings->next;
        }
        return NULL;
    }
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
        return NULL;
    }
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

TOKEN* use_temp_reg(FRME * frame){
    TOKEN* t= new_dst(frame);
    if(t == NULL ) {printf("error: all registers in use!");exit(1);}
    BNDING *bindings = frame->bindings;
    BNDING *new = malloc(sizeof(BNDING));
    if(new != NULL){
        new->type = IDENTIFIER;
        new->loc = t;
        new->next = bindings;
        frame->bindings=new;
        return t;
    }
    printf("fatal: binding creation failed!\n");
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
        new->type = IDENTIFIER;
        new->name = x;
        new->loc = NULL;
        new->next = bindings;
        frame->bindings=new;
        return;
    }
    printf("fatal: binding creation failed!\n");
}

TOKEN *declare_fnc(TOKEN * x, CLSURE* val, FRME * frame){
    BNDING *bindings = frame->bindings;
    BNDING *new = malloc(sizeof(BNDING));
    if(new != NULL){
        new->type = CLOS;
        new->name = x;
        new->clos = val;
        new->next = bindings;
        frame->bindings=new;
        return new->name;
    }
    printf("fatal: binding creation failed!\n");
}

CLSURE *find_fnc(TOKEN* name, FRME* e){
    FRME *ef = e;
    BNDING* bindings;
     while(ef != NULL){
        bindings = ef->bindings;
        while (bindings != NULL){
            if(bindings->name ==  name){
               return bindings->clos;
            }
            bindings = bindings->next;
        }
         ef = ef->next;
     }
     printf("No function %s in scope, exiting...\n",name->lexeme);exit(1);
}

