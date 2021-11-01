#include "nodes.h"
#include "value.h"
#ifndef ENVIRONMENT
#define ENVIRONMENT

typedef struct binding {
  TOKEN* name;
  VALUE* value;
  struct binding* next;
} BINDING;

typedef struct frame {
  BINDING* bindings;
  struct frame* next;
}FRAME;


extern VALUE *lookup_name(TOKEN*, FRAME*);
extern VALUE *assign_to_name(TOKEN*, FRAME*,VALUE*);
extern VALUE *declare_name(TOKEN*, FRAME*);
#endif //ENVIRONMENT