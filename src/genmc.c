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
  }
  return n;
}

MC* find_lst(MC* mc){
  while(mc->next != NULL){
    mc = mc->next;
  }
  return mc;
}

MC* init_mc(){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    mc->insn = ".text";
    mc->next = malloc(sizeof(MC));
    mc->next->insn = malloc(sizeof(INSN_BUF));
    mc->next->insn = ".globl main";
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
    mc->insn = "li $v0,10";
    mc->next =malloc(sizeof(MC));
    mc->next->insn = malloc(sizeof(INSN_BUF));
    mc->next->insn = "syscall";
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
      arn = malloc(sizeof(AR));
      arn->fp = ar;
      arn->sl = ar->sl+1;
      mc = new_prc(i,arn);
      mc->next = gen_mc0(i->next,dat, ar,e);
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

MC *gen_mc(BB** i){
  MC* dat = init_dat();
  MC* mc = init_mc();
  AR* ar = malloc(sizeof(AR));
  FRME* e = malloc(sizeof(FRME));
  ar->sl = 0;
  VAR* vars;
  while((*i) != NULL){
     mc->next = gen_mc0((*i)->leader, dat, ar,e);
     *i++;
  }
  return mc;
}