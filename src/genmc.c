#include "genmc.h"
#include "gentac.h"
#include <stdlib.h>
#include <stdio.h>
#include "C.tab.h"
#include "regstack.h"
#include "mc_env.h"

#define INSN_BUF 64
#define VAR_COUNT 64

extern int isempty() ;
extern int isfull() ;
extern TOKEN* pop();
extern TOKEN* peep();
extern int push(TOKEN*);
extern TOKEN *lookup_loc(TOKEN*, FRME*);
extern TOKEN *assign_to_var(TOKEN*, FRME*,TOKEN*);
extern void declare_var(TOKEN*, FRME*);

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

MC* find_lst(MC* mc){
  while(mc->next != NULL){
    mc = mc->next;
  }
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

MC* init_mc(){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    mc->insn = ".globl main";
    mc->next = malloc(sizeof(MC));
    mc->next->insn = malloc(sizeof(INSN_BUF));
    mc->next->insn = ".text";
    return mc;
}

MC* init_dat(){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  mc->insn = ".data";
  return mc;
}

MC* new_rtn(TAC* tac){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    if(tac->rtn.type == CONSTANT){
      sprintf(mc->insn,"li $v1 %d",tac->rtn.v->value);
    }
    else if(tac->rtn.type == IDENTIFIER){
      sprintf(mc->insn,"move $v1 $%s",tac->rtn.v->lexeme);
    }
    return mc;
}

MC* new_prc(TAC* tac, AR* ar){
  MC *mc = malloc(sizeof(MC));
  MC *first =mc;
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"%s:",tac->proc.name->lexeme);
  /* mc = mc->next;
  for(int i = tac->proc.arity; i>0;i--){
    mc->insn = malloc(sizeof(INSN_BUF));
    sprintf(mc->insn,"%s:",tac->proc.name->lexeme);
  } */
  return mc;
}

MC* new_plus(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"add $%s,$%s,$%s",tac->stac.dst->lexeme,tac->stac.src1->lexeme,tac->stac.src2->lexeme);
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

MC* new_ld(FRME *e, TAC* tac){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    if(tac->ld.src1->type == CONSTANT){
        sprintf(mc->insn, "li $%s,%d",tac->ld.dst->lexeme,tac->ld.src1->value);
    }
    else if(tac->ld.src1->type == IDENTIFIER){
        TOKEN *loc = lookup_loc(tac->ld.src1,e);
        sprintf(mc->insn, "move $%s,$%s",tac->ld.dst->lexeme,loc->lexeme);
    } 
    return mc;
}

/* MC* allocate_mem(){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  mc->insn = "li $a0 4";
  mc->next = malloc(sizeof(MC));
  mc->next->insn = malloc(sizeof(INSN_BUF));
  mc->next->insn = "li $v0 9";
  mc->next->next = malloc(sizeof(MC));
  mc->next->next->insn = malloc(sizeof(INSN_BUF));
  mc->next->next->insn = "syscall";
  return mc;
} */

 MC* new_str(TAC* tac, FRME* e){
  MC *mc = malloc(sizeof(MC));
  MC *last;
  mc->insn = malloc(sizeof(INSN_BUF));
  TOKEN* t = lookup_loc(tac->ld.dst,e);
  if(t == NULL){
    declare_var(tac->ld.dst,e);
  }
  assign_to_var(tac->ld.dst,e,tac->ld.src1);
 /* mc->next = allocate_mem();
   last = find_lst(mc);
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  sprintf(last->next->insn,"sw $%s, $v0",tac->ld.src1->lexeme); */
  return mc; 
}

AR* gen_frame(AR* old, TAC* tac){
  AR* new = malloc(sizeof(AR));
  int locals = count_locals(tac);
  new->size = (locals*4) + (tac->proc.arity*4)+4;

  for(int i = 0; i < tac->proc.arity; i++){
      new->local[i] = 4+(i*4);
    }

  for(int i = 0; i < locals; i++){
    new->local[i] = 4+(tac->proc.arity*4)+(i*4);
  }
  return new;
}

MC* print_frame(AR* ar){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"li $a0 %d",ar->size);

  MC* last = find_lst(mc);
  last->next = make_syscall(SBRK);

  last = find_lst(mc);
  last->next = malloc(sizeof(MC));
  last->next->insn = malloc(sizeof(INSN_BUF));
  last->next->insn = "move $fp $v0";

  return mc;
}



MC* gen_mc0(TAC* i, MC* dat, AR* ar, FRME* e)
{
  MC* mc, *last;
  AR* arn;
  if (i==NULL) return NULL;
  switch (i->op) {
  default:
    printf("unknown type code %d (%p) in mmc_mcg\n",i->op,i);

    case tac_plus:
      mc = new_plus(i);
      mc->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_minus:
      mc = new_minus(i);
      mc->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_div:
      mc = new_div(i);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_mod:
      mc = new_mod(i);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_mult:
      mc = new_mult(i);
      mc->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_proc:
      arn = gen_frame(ar,i);
      mc = new_prc(i,arn);
      mc->next = print_frame(arn);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_endproc:
      mc = new_func_rtn(i);
      if(i->next != NULL){mc->next = gen_mc0(i->next,dat, ar,e);}
      return mc;
    case tac_load:
      mc = new_ld(e,i);
      mc->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_store:
      mc = new_str(i,e);
      last = find_lst(mc);
      last->next = gen_mc0(i->next,dat, ar,e);
      return mc;
    case tac_if:
    case tac_lbl:
    case tac_goto:
    case tac_call:
    case tac_rtn:
      mc = new_rtn(i);
      mc->next = gen_mc0(i->next,dat, ar,e);
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

MC *gen_mc(BB** i){
  MC* dat = init_dat();
  MC* mc = init_mc();
  MC* last = find_lst(mc);
  AR* ar = malloc(sizeof(AR));
  FRME* e = malloc(sizeof(FRME));
  ar->sl = 0;
  VAR* vars;
  while((*i) != NULL){
     last->next = gen_mc0((*i)->leader, dat, ar,e);
     *i++;
  }
  last = find_lst(mc);
  last->next = print_result();
  return mc;
}