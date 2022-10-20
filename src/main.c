#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include "interpreter.h"
#include "gentac.h"
#include "genmc.h"

char *named(int t)
{
    static char b[100];
    if (isgraph(t) || t==' ') {
      sprintf(b, "%c", t);
      return b;
    }
    switch (t) {
      default: return "???";
    case IDENTIFIER:
      return "id";
    case CONSTANT:
      return "constant";
    case STRING_LITERAL:
      return "string";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case EXTERN:
      return "extern";
    case AUTO:
      return "auto";
    case INT:
      return "int";
    case VOID:
      return "void";
    case APPLY:
      return "apply";
    case LEAF:
      return "leaf";
    case IF:
      return "if";
    case ELSE:
      return "else";
    case WHILE:
      return "while";
    case CONTINUE:
      return "continue";
    case BREAK:
      return "break";
    case RETURN:
      return "return";
    }
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;
    for (i=0; i<level; i++) putchar(' ');
    if (t->type == CONSTANT) printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL) printf("\"%s\"\n", t->lexeme);
    else if (t) puts(t->lexeme);
}

void print_tree0(NODE *tree, int level)
{
    int i;
    if (tree==NULL) return;
    if (tree->type==LEAF) {
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
      printf("%s\n",named(tree->type));
/*       if (tree->type=='~') { */
/*         for(i=0; i<level+2; i++) putchar(' '); */
/*         printf("%p\n", tree->left); */
/*       } */
/*       else */
        print_tree0(tree->left, level+2);
      print_tree0(tree->right, level+2);
    }
}

void print_tree(NODE *tree)
{
    print_tree0(tree, 0);
}

char* tac_ops[] = {"","ADD","SUB","DIV","MOD","MULT","PROC","ENDPROC","LOAD","STORE","IF","LABEL","GOTO","CALL","RETURN","INNER_PROC"};

void print_if(TAC* tac){
  if(tac->ift.op1->type == IDENTIFIER && tac->ift.op2->type == IDENTIFIER){
    printf("%s (%s%s%s) %s\n",
    tac_ops[tac->op],
    tac->ift.op1->lexeme, 
    named(tac->ift.code),
    tac->ift.op2->lexeme,
    tac->ift.lbl->lexeme);
  }
  else if(tac->ift.op1->type == IDENTIFIER){
    printf("%s (%s%s%d) %s\n",
    tac_ops[tac->op],
    tac->ift.op1->lexeme, 
    named(tac->ift.code),
    tac->ift.op2->value,
    tac->ift.lbl->lexeme);
  }
  else if(tac->ift.op2->type == IDENTIFIER){
    printf("%s (%d%s%s) %s\n",
    tac_ops[tac->op],
    tac->ift.op1->value, 
    named(tac->ift.code),
    tac->ift.op2->lexeme,
    tac->ift.lbl->lexeme);
  }
  else{
    printf("%s (%d%s%d) %s\n",
    tac_ops[tac->op],
    tac->ift.op1->value, 
    named(tac->ift.code),
    tac->ift.op2->value,
    tac->ift.lbl->lexeme);
  }
  
}

void print_rtn(TAC* tac){
  if(tac->rtn.type == tac_call){
    printf("%s\n",
    tac_ops[tac->op]);
  }
  else if (tac->rtn.type == CONSTANT){
    printf("%s %i\n",
    tac_ops[tac->op],
    tac->rtn.v->value);
  }
  else{
    printf("%s %s\n",
    tac_ops[tac->op],
    tac->rtn.v->lexeme);
  }
}

void print_ic(TAC* tac){

  while(tac!=NULL){
    switch(tac->op){
      default: 
        printf("%s %s %s %s\n",
        tac_ops[tac->op],
        tac->stac.src1->lexeme,
        tac->stac.src2->lexeme,
        tac->stac.dst->lexeme);
        break;
      case tac_load:
        if(tac->ld.src1->type == CONSTANT){
          printf("%s %i %s\n",
          tac_ops[tac->op],
          tac->ld.src1->value,
          tac->ld.dst->lexeme);
        }
        else{
          printf("%s %s %s\n",
          tac_ops[tac->op],
          tac->ld.src1->lexeme,
          tac->ld.dst->lexeme);
        }
        break;
      case tac_store:
        printf("%s %s %s\n",
        tac_ops[tac->op],
        tac->ld.src1->lexeme,
        tac->ld.dst->lexeme);
        break;
      case tac_proc:
        printf("%s %s %i\n",
        tac_ops[tac->op],
        tac->proc.name->lexeme,
        tac->proc.arity);
        break;
      case tac_innerproc:
        printf("%s %s %i\n",
        tac_ops[tac->op],
        tac->proc.name->lexeme,
        tac->proc.arity);
        break;
      case tac_endproc:
        printf("%s\n",
        tac_ops[tac->op]);
        break;
      case tac_if:
        print_if(tac);
        break;
      case tac_lbl:
        printf("%s %s\n",
        tac_ops[tac->op],
        tac->lbl.name->lexeme);
        break;
      case tac_goto:
        printf("%s %s\n",
        tac_ops[tac->op],
        tac->gtl.lbl->lexeme);
        break;
      case tac_call:
        printf("%s %s %i\n",
        tac_ops[tac->op],
        tac->call.name->lexeme,
        tac->call.arity);
        break;
      case tac_rtn:
        print_rtn(tac);
        break;
    }
    tac = tac->next;
  }
    
}

void print_token(TOKEN *i){
  if (i->type == CONSTANT){
    printf("%d",i->value);
  }
  else {
    printf("%s",i->lexeme);
  }
}
void print_mc(MC* i)
{
  for(;i!=NULL;i=i->next) printf("%s\n",i->insn);
}

void print_bbs(BB** bbs){
  int i = 0;
  while(bbs[i] != NULL){
    printf("\033[0;31m");
    printf("BLOCK #");print_token(bbs[i]->id);
    printf("\n\033[0m");
    print_ic(bbs[i]->leader);
    if(bbs[i]->nexts[0] != NULL){
      printf("\033[0;31m");
      printf("LINKS TO : ");
      print_token(bbs[i]->nexts[0]->id);
    }
    if(bbs[i]->nexts[1] != NULL){
      printf(" ");
      print_token(bbs[i]->nexts[1]->id);
    }
    i++;
    printf("\n\n");
  }
  printf("\033[0m");
}

extern int yydebug;
extern NODE* yyparse(void);
extern NODE* ans;
extern void init_symbtable(void);
extern VALUE* interpret(NODE*);
extern TAC* gen_tac(NODE*);
extern MC* gen_mc(TAC*);

int main(int argc, char** argv)
{
    NODE* tree;
    FRAME* e = malloc(sizeof(FRAME));
    if (argc>1 && strcmp(argv[1],"-d")==0) yydebug = 1;
    init_symbtable();
    printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);
    printf("\n");
    printf("Calling interpreter...\n");
    VALUE* result = interpret(tree);
    if(result != NULL){
      printf("RESULT : %i\n",result->integer);
    }
    else{
      printf("RESULT: NULL\n");
    }

    printf("----------------------------------------------------------------\n");
    printf("Generating TAC...\n");
    TAC* tac = gen_tac(tree);
    print_ic(tac);
    //print_bbs(tac);
    printf("Generating machine code...\n");
    print_mc(gen_mc(tac));
    return 0;
}
