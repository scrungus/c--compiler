#include "genmc.h"
#include "gentac.h"
#include <stdlib.h>
#include <stdio.h>
#include "C.tab.h"
#include "regstack.h"

#define INSN_BUF 64
#define VAR_COUNT 64

extern int isempty() ;
extern int isfull() ;
extern TOKEN* pop();
extern TOKEN* peep();
extern int push(TOKEN*);


TOKEN * new_srdest(SR* sr){
    TOKEN* dst = (TOKEN*)malloc(sizeof(TOKEN));
    if(dst==NULL){printf("fatal: failed to generate destination\n");exit(1);}
    dst->type=IDENTIFIER;
    dst->lexeme = (char*)calloc(1,2);
    sprintf(dst->lexeme,"s%i",sr->srcnt);
    sr->srcnt++;
    int p = push(dst);
    if (p != 0){printf("fatal: failed to add new register \n");exit(1);}
    return dst;
}

int exists_var(VAR* vars, TOKEN* t){
  while(vars != NULL && t != NULL){
    if(vars->name == t){
      return 1;
    }
    vars = vars->next;
  }
  return 0;
}

VAR* change_var(VAR* vars, TOKEN* var, TOKEN* reg){
  while(vars != NULL && var != NULL && reg != NULL){
    if(vars->name == var){
      vars->reg = reg;
      return vars;
    }
    vars = vars->next;
  }
  return vars;
}

TOKEN *find_var(VAR* vars, TOKEN* t){
  while(vars != NULL && t != NULL){
    if(vars->name == t){
      return vars->reg;
    }
    vars = vars->next;
  }
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

/* void add_dat(MC* dat, char* name, int size, char vars[]){

    if(add_var(vars)){
      MC* next =  malloc(sizeof(MC));
      next->insn = malloc(sizeof(INSN_BUF));
      sprintf(next->insn,"%s: .word %d",name,size);
      dat->next = next;
    }
} */

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

MC* new_prc(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"%s:",tac->proc.name->lexeme);
  return mc;
}

MC* new_plus(TAC* tac){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  sprintf(mc->insn,"add $%s,$%s,$%s",tac->stac.dst->lexeme,tac->stac.src1->lexeme,tac->stac.src2->lexeme);
  return mc;
}
MC* new_minus(TAC* tac){
    
}
MC* new_div(TAC* tac){
    
}
MC* new_mod(TAC* tac){
    
}
MC* new_mult(TAC* tac){
    
}

MC* new_ld(VAR* vars, TAC* tac){
    MC *mc = malloc(sizeof(MC));
    mc->insn = malloc(sizeof(INSN_BUF));
    if(tac->ld.src1->type == CONSTANT){
        sprintf(mc->insn, "li $%s,%d",tac->ld.dst->lexeme,tac->ld.src1->value);
    }
    else if(tac->ld.src1->type == IDENTIFIER){
        TOKEN *loc = find_var(vars,tac->ld.src1);
        sprintf(mc->insn, "move $%s,$%s",tac->ld.dst->lexeme,loc->lexeme);
    } 
    return mc;
}

VAR* add_var(TAC* tac, VAR* vars){
  if(!exists_var(vars,tac->ld.dst)){
    VAR* v = malloc(sizeof(VAR));
    v->name = tac->ld.dst;
    v->reg = tac->ld.src1;
    v->next = vars;
    vars = v;
    return vars;
  }
  else{
    return change_var(vars,tac->ld.dst,tac->ld.src1);
  }
}

MC* new_str(TAC* tac, SR* sr, VAR* vars){
  MC *mc = malloc(sizeof(MC));
  mc->insn = malloc(sizeof(INSN_BUF));
  TOKEN* t = find_var(vars,tac->ld.dst);
  TOKEN* s = new_srdest(sr);
  sprintf(mc->insn,"move $%s,$%s",s->lexeme,t->lexeme);
  vars = change_var(vars,tac->ld.dst,s);
  return mc;
}

MC* gen_mc0(TAC* i, MC* dat, VAR* vars, SR* sr)
{
  MC* mc;
  if (i==NULL) return NULL;
  switch (i->op) {
  default:
    printf("unknown type code %d (%p) in mmc_mcg\n",i->op,i);

    case tac_plus:
      mc = new_plus(i);
      mc->next = gen_mc0(i->next,dat, vars,sr);
      return mc;
    case tac_minus:
    case tac_div:
    case tac_mod:
    case tac_mult:
    case tac_proc:
      mc = new_prc(i);
      mc->next = gen_mc0(i->next,dat, vars,sr);
      return mc;
    case tac_endproc:
      return NULL;
    case tac_load:
      mc = new_ld(vars,i);
      mc->next = gen_mc0(i->next,dat, vars,sr);
      return mc;
    case tac_store:
      vars = add_var(i,vars);
      mc = new_str(i,sr,vars);
      mc->next = gen_mc0(i->next,dat, vars,sr);
      return mc;
    case tac_if:
    case tac_lbl:
    case tac_goto:
    case tac_call:
    case tac_rtn:
      mc = new_rtn(i);
      mc->next = gen_mc0(i->next,dat, vars,sr);
      return mc;
  }
}

MC *gen_mc(TAC* i){
  MC* dat = init_dat();
  MC* mc = init_mc();
  SR *sr = malloc(sizeof(SR));
  sr->srcnt = 0;
  VAR* vars;
  mc->next = gen_mc0(i, dat, vars,sr);
  return mc;
}