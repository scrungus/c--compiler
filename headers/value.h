
#ifndef VALUE_
#define VALUE_
typedef struct value {
    int type;
    union 
    {
        int integer;
        int boolean;
        char* string;

    } ;
    
}VALUE;
#endif