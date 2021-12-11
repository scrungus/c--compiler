#include "genmc.h"
#include "gentac.h"
#include <stdlib.h>
#include <stdio.h>
#include "C.tab.h"
#include "regstack.h"
#include "mc_env.h"
#include "string.h"

#define INSN_BUF 64

extern TOKEN *lookup_loc(TOKEN*, FRME*);
extern TOKEN *assign_to_var(TOKEN*, FRME*,TOKEN*);
extern void declare_var(TOKEN*, FRME*);
extern void declare_fnc(TOKEN*, CLSURE*, FRME*);
extern CLSURE *find_fnc(TOKEN* , FRME* );
extern TOKEN* use_temp_reg(FRME *);

int call_stack;

//PROBLEM : HOW DO YOU TRACK THE CALL STACK, AND MAKE SURE THAT THE SCOPE YOU USE IS LEXICAL?
// I.E. IF YOU HAVE A FUNCTION CALL ITSELF X TIMES, IT WILL BE X*SIZE HIGH ON THE STACK, BUT ITS LEXICAL SCOPE IS ONLY ONE DEEP. 

TAC* find_endproc(TAC* i){
  while(i != NULL && i->op != tac_endproc){
    i = i->next;
  }
  return i;
}

MC* find_lst(MC* mc){
  while(mc->next != NULL){
    mc = mc->next;
  }
  return mc;
}

int count_locals(TAC* i){
  int n = 0;
  while(i != NULL && i->op != tac_endproc){
    if(i->op == tac_store){
      n++;
    }
    i = i->next;
  }
  return n;
}

TOKEN * new_dst(FRME *e){
    for(int i=0; i<MAXREGS; i++){
        if(!reg_in_use(i,e)){
            TOKEN* dst = (TOKEN*)malloc(sizeof(TOKEN));
            if(dst==NULL){printf("fatal: failed to generate destination\n");exit(1);}
            dst->type=IDENTIFIER;
            dst->lexeme = (char*)calloc(1,2);
            sprintf(dst->lexeme,"t%i",i);
            dst->value = i;
            return dst;
        }
    }
}

MC* new_minus(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"sub $%s,$%s,$%s",tac->stac.dst->lexeme,tac->stac.src1->lexeme,tac->stac.src2->lexeme);
  return mc;
}
MC* new_div(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"div $%s,$%s",tac->stac.src1->lexeme,tac->stac.src2->lexeme);
  mc->next = malloc(sizeof(MC));
  mc->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->next->insn,"move $%s,$lo",tac->stac.dst->lexeme);
  return mc;
}
MC* new_mod(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"div $%s,$%s",tac->stac.src1->lexeme,tac->stac.src2->lexeme);
  mc->next = malloc(sizeof(MC));
  mc->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->next->insn,"move $%s,$hi",tac->stac.dst->lexeme);
  return mc;
}
MC* new_mult(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"mul $%s,$%s,$%s",tac->stac.dst->lexeme,tac->stac.src1->lexeme,tac->stac.src2->lexeme);
  return mc;
}
MC* new_plus(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"add $%s,$%s,$%s",tac->stac.dst->lexeme,tac->stac.src1->lexeme,tac->stac.src2->lexeme);
  return mc;
}

MC* init_mc(){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    mc->insn = ".globl premain";
    mc->next = malloc(sizeof(MC));
    mc->next->insn = malloc(sizeof(INSN_BUF));
    mc->next->insn = ".text";
    return mc;
}

MC* make_syscall(int code){
  MC* mc;
  mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"li $v0 %d",code);

  MC* last = find_lst(mc);
  last->next =  malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "syscall";
  return mc;
}

MC* new_smpl_ld(FRME* e, TOKEN* src, TOKEN* dst){
  MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    if(src->type == CONSTANT){
        sprintf(mc->insn, "li $%s,%d",dst->lexeme,src->value);
    }
    else if(src->type == IDENTIFIER){
        TOKEN *loc = lookup_loc(src,e);
        sprintf(mc->insn, "move $%s,$%s",dst->lexeme,src->lexeme);
    } 
    return mc;
}

TOKEN* lookup_from_memory(TOKEN* name, FRME* e, AR* ar){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    mc->insn = "# Looking up token from memory";
    MC* last = find_lst(mc);
    int j = 0;
    TOKEN* t;
   while(e != NULL){
    BNDING *bindings = e->bindings;
    int i = 1;
    while(bindings != NULL){
        if(bindings->name == name){
          t = new_token(IDENTIFIER);
          t->lexeme = malloc(sizeof(INSN_BUF));
          sprintf(t->lexeme,"%d($sp)",call_stack+ar->size-e->stack_pos-8-4*i);
        }
        if(bindings->type == IDENTIFIER){
          i++;
        }
        bindings = bindings->next;
    }
    j+= e->size;
    e = e->next;
  }
  return t;
}

MC* new_ld(FRME *e, TAC* tac, AR* curr){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    if(tac->ld.src1->type == CONSTANT){
        sprintf(mc->insn, "li $%s,%d",tac->ld.dst->lexeme,tac->ld.src1->value);
    }
    else if(tac->ld.src1->type == IDENTIFIER){
        TOKEN *loc = lookup_loc(tac->ld.src1,e);
        if(loc == NULL){
          loc = lookup_from_memory(tac->ld.src1,e,curr);
          sprintf(mc->insn, "lw $%s,$%s",tac->ld.dst->lexeme,loc->lexeme);
        }
        else{
          sprintf(mc->insn, "move $%s,$%s",tac->ld.dst->lexeme,loc->lexeme);
        }   
    } 
    return mc;
}

 MC* new_str(TAC* tac, FRME* e){
  MC *mc = malloc(sizeof(MC));
  MC *last;
  mc->insn = malloc(sizeof(INSN_BUF));
  TOKEN* t = lookup_loc(tac->ld.dst,e);
  if(t == NULL){
    declare_var(tac->ld.dst,e);
    assign_to_var(tac->ld.dst,e,tac->ld.src1);
  }
  else if(t->value != tac->ld.src1->value) {
    sprintf(mc->insn,"move $%s $%s",t->lexeme,tac->ld.src1->lexeme);
  }
  return mc; 
}

MC* new_ift(TAC* tac, FRME* e){
  TOKEN* dst1 = lookup_loc(tac->ift.op1,e);
  MC* mc;
  if(dst1 == NULL){
    dst1 = new_dst(e);
    declare_var(tac->ift.op1,e);
    assign_to_var(tac->ift.op1,e,dst1);
    mc = new_smpl_ld(e,tac->ift.op1,dst1);
  }
  TOKEN* dst2 = lookup_loc(tac->ift.op2,e);
  if(dst2 == NULL){
    dst2 = new_dst(e);
    declare_var(tac->ift.op2,e);
    assign_to_var(tac->ift.op2,e,dst2);
    if(mc != NULL){
      mc->next = new_smpl_ld(e,tac->ift.op2,dst2);
    }
    else {mc = new_smpl_ld(e,tac->ift.op2,dst2);}
  }
  
  MC* last = find_lst(mc);
  if(last != NULL){
    last->next = malloc(sizeof(MC));
    last->next->insn = malloc(sizeof(INSN_BUF));
    last = last->next;
  }
  else{
    last = malloc(sizeof(MC));
    last->insn = malloc(sizeof(INSN_BUF));
  }

  switch(tac->ift.code){
    case '>':
      sprintf(last->insn,"ble $%s $%s %s",dst1->lexeme,dst2->lexeme,tac->ift.lbl->lexeme);
      break;
    case '<':
      sprintf(last->insn,"bge $%s $%s %s",dst1->lexeme,dst2->lexeme,tac->ift.lbl->lexeme);
      break;
    case EQ_OP:
      sprintf(last->insn,"bne $%s $%s %s",dst1->lexeme,dst2->lexeme,tac->ift.lbl->lexeme);
      break;
    case NE_OP:
      sprintf(last->insn,"beq $%s $%s %s",dst1->lexeme,dst2->lexeme,tac->ift.lbl->lexeme);
      break;
    case LE_OP:
      sprintf(last->insn,"bgt $%s $%s %s",dst1->lexeme,dst2->lexeme,tac->ift.lbl->lexeme);
      break;
    case GE_OP:
      sprintf(last->insn,"blt $%s $%s %s",dst1->lexeme,dst2->lexeme,tac->ift.lbl->lexeme);
  }
  delete_constants(e);
  return mc;
}

MC* new_gtl(TAC* i){
  MC* mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"j %s",i->gtl.lbl->lexeme);
  return mc;
}

MC* new_lbli(TAC* i){
  MC* mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"%s:",i->lbl.name->lexeme);
  return mc;
}

MC* save_frame(AR* ar, FRME *e){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  mc->insn = "# Saving frame";
  MC* last = find_lst(mc);
  int i = 0;
  while(e != NULL && ar->arity != 0){
    BNDING *bindings = e->bindings;
    int i = 1;
    while(bindings != NULL){
        last->next = malloc(sizeof(MC));
        last->next->insn = malloc(sizeof(INSN_BUF));
        if(bindings->type == IDENTIFIER){
          sprintf(last->next->insn,"sw $%s %d($sp)",bindings->loc->lexeme,8+4*i);
           i++;
        }
       
        bindings = bindings->next;
        last = find_lst(last);
    }
    break;
  }
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "# End of saving frame";
  return mc;
}

MC* gen_frame(AR* ar){


  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  mc->insn = "# Creating new frame";
  MC* last = find_lst(mc);

  //allocate stack space for new frame
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(last->next->insn,"addiu $sp, $sp -%d",ar->size);
  last = find_lst(mc);

  //load new size into reg
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(last->next->insn,"li $t1, %d",ar->size);
  last = find_lst(mc);

   //store size on stack
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "sw $t1, 0($sp)";
  last = find_lst(mc);

  //load new arity into reg
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(last->next->insn,"li $t2, %d",ar->arity);
  last = find_lst(mc);

  //store arity on stack
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "sw $t2, 8($sp)";
  last = find_lst(mc);

  //store sl on stack
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(last->next->insn,"li $t3, %d",ar->sl);
  last = find_lst(mc);

  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "sw $t3, 4($sp)";
  last = find_lst(mc);

  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "# End of creating frame";
  return mc;
}

MC* gen_globframe(TAC* tac, FRME* e, AR* global){
  global->sl = 0;
  global->size = 12;
  global->arity = 0;
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  mc->insn = "#saving global frame";
  MC* last = find_lst(mc);
  CLSURE* f;
  while(tac != NULL){
      switch(tac->op){
        case(tac_load):
           last->next = new_ld(e,tac,global);
           last = find_lst(last);
           last->next = new_str(tac->next,e);
           global->size+=4;
           global->arity++;
           break;
        case(tac_proc):
          f = malloc(sizeof(CLSURE));
          f->env = e;
          f->code = tac; 
          declare_fnc(tac->proc.name,f,e);
          tac = find_endproc(tac);
      }
      tac = tac->next;
  }
  e->size = global->size;
  MC* first =malloc(sizeof(MC));
  first->insn = malloc(sizeof(INSN_BUF));
  first->insn = "premain: ";
  MC* r = find_lst(first);
  r->next = gen_frame(global);
  r = find_lst(r);
  r->next = mc;
  r = find_lst(r);
  r->next = save_frame(global,e);
  return first;
}

AR* calculate_frame(AR* old, TAC* tac){
  AR* new = malloc(sizeof(AR));
  int locals = count_locals(tac);
  new->arity = locals + tac->proc.arity;
  new->size = (locals*4) + (tac->proc.arity*4)+12;
  new->sl = old->sl+1;
  return new;
}

MC* restore_frame(AR* ar, FRME *e){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  mc->insn = "# Restoring frame";
  MC* last = find_lst(mc);
  int i = 0;
  while(e != NULL && ar->arity != 0){
    BNDING *bindings = e->bindings;
    int i = 1;
    while(bindings != NULL){
        last->next = malloc(sizeof(MC));
        last->next->insn = malloc(sizeof(INSN_BUF));
        if(bindings->type == IDENTIFIER){
          sprintf(last->next->insn,"lw $%s  %d($sp)",bindings->loc->lexeme,8+4*i);
        }
        i++;
        bindings = bindings->next;
        last = find_lst(last);
    }
    break;
  }
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "# End of saving frame";
  return mc;
}


MC *call_func(TOKEN* name, FRME* e, AR* curr){
  TOKEN* t = (TOKEN *)name;
  CLSURE *f = find_fnc(t,e);
  FRME* ef = malloc(sizeof(FRME));
  ef->next = f->env;
  call_stack += curr->size;
  ef->stack_pos = call_stack;
  MC* mc = gen_mc0(f->code,ef,curr);
  return mc;
}


MC* new_func_rtn(TAC* i){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  if(i->next == NULL){
  }
  else{
    mc->insn = "jr $ra";
  }
  return mc;
}

MC* new_rtn(TAC* tac, FRME* e, AR* ar){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    if(tac->rtn.type == CONSTANT){
      sprintf(mc->insn,"li $v1 %d",tac->rtn.v->value);
    }
    else if(tac->rtn.type == IDENTIFIER){
      TOKEN *t = lookup_loc(tac->rtn.v,e);
      if(t == NULL){
        t = lookup_from_memory(tac->rtn.v,e,ar);
      }
      if(t == NULL){
        sprintf(mc->insn,"move $v1 $%s",tac->rtn.v->lexeme);
      }
      else{
        sprintf(mc->insn,"lw $v1 %s",t->lexeme);
      }
    }
    call_stack -= e->size;
    MC* last = find_lst(mc);
    last->next =  malloc(sizeof(MC));
    last->next->insn = malloc(sizeof(INSN_BUF));
    sprintf(last->next->insn,"addiu $sp, $sp %d",ar->size);
    return mc;
}

MC* new_prc(TAC* tac, FRME* e){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"%s:",tac->proc.name->lexeme);
  MC* last = find_lst(mc);
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  TOKENLIST* vars = tac->proc.args;
  TOKEN* t;
  int i = 0;
  while(i < tac->proc.arity && vars != NULL){
    t = new_dst(e);
    sprintf(last->next->insn,"move $%s $a%d",t->lexeme,i);
    last = find_lst(last);
    declare_var(vars->name,e);
    assign_to_var(vars->name,e,t);
    vars = vars->next;
    i++;
  }
  return mc;
}

MC* new_cll(TAC* tac, FRME* e){
  MC* mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  MC* last = find_lst(mc);
  int i=0;
  while(i< tac->call.arity && tac->call.args != NULL){
    TOKEN* t = new_token(IDENTIFIER); 
    t->lexeme = (char*)calloc(1,2);
    sprintf(t->lexeme,"a%d",i);
    last->next = new_smpl_ld(e,tac->call.args->name,t);
    last = find_lst(last);
    i++;
  }
  last = find_lst(last);
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(last->next->insn,"jal %s",tac->call.name->lexeme);
  return mc;
}

MC* gen_mc0(TAC* i, FRME* e, AR* curr){
  MC* mc, *last;
  if (i==NULL || i->op == tac_endproc) 
  {delete_constants(e); mc = new_func_rtn(i); return mc;}

  switch (i->op) {
  default:
    printf("unknown type code %d (%p) in mmc_mcg\n",i->op,i);

    case tac_plus:
      mc = new_plus(i);
      mc->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_minus:
      mc = new_minus(i);
      mc->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_div:
      mc = new_div(i);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_mod:
      mc = new_mod(i);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_mult:
      mc = new_mult(i);
      mc->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_proc:
      curr = calculate_frame(curr,i);
      e->size = curr->size;
      mc = new_prc(i,e);
      last = find_lst(mc);
      last->next = gen_frame(curr);
      last = find_lst(last);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_load:
      mc = new_ld(e,i,curr);
      mc->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_store:
      mc = new_str(i,e);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_if:
      mc = new_ift(i,e);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_lbl:
      mc = new_lbli(i);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_goto:
      mc = new_gtl(i);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
    case tac_call:
      mc = save_frame(curr,e);
      last = find_lst(mc);
      last->next = new_cll(i,e);
      last = find_lst(last);
      last->next = restore_frame(curr,e);
      last = find_lst(last);
      last->next = gen_mc0(i->next,e,curr);
      last = find_lst(last);
      last->next = call_func(i->call.name,e,curr);
      return mc;
    case tac_rtn:
      mc = new_rtn(i,e,curr);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,e,curr);
      return mc;
  }
}

MC* print_result() {

    //print integer result
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    mc->insn = "move $a0 $v1";

    MC* last = find_lst(mc);
    last->next = make_syscall(PRINT_INT);

    //print newline
    last = find_lst(last);
    last->next = malloc(sizeof(MC));
    last->next->insn = malloc(sizeof(INSN_BUF));
    last->next->insn = "li $a0 10";

    last = find_lst(last);
    last->next = make_syscall(PRINT_CHAR);

    //exit
    last = find_lst(last);
    last->next = make_syscall(EXIT);
    return mc;
}


MC* gen_mc(TAC* tac){
  FRME* e = malloc(sizeof(FRME));
  AR* global = malloc(sizeof(AR));
  MC* mc = init_mc();
  MC* last = find_lst(mc);
  last->next = gen_globframe(tac,e,global);
  last = find_lst(last);
  FRME *ef = e;
    while(ef != NULL){
        BNDING* bindings = e->bindings;
        while (bindings != NULL){
            if(strcmp(bindings->name->lexeme,"main")==0){
                last->next = malloc(sizeof(MC));
                last->next->insn = malloc(sizeof(INSN_BUF));
                last->next->insn = "jal main";
                last = find_lst(last);
                last->next = call_func(bindings->name,e,global);
                last = find_lst(last);
                last->next = print_result();
                return mc;
            }
            bindings = bindings->next;
        }
        ef = e->next;
    }
}