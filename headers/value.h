#ifndef VALUE_
#define VALUE_

typedef struct frame FRAME;

typedef struct closure {
FRAME* env;
NODE* code;
} CLOSURE;

typedef struct value {
    int type;
    union 
    {
        int integer;
        int boolean;
        char* string;
        CLOSURE* closure;

    } ;
    
}VALUE;
#endif