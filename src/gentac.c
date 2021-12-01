#include "gentac.h"
#include <stdlib.h>
#include "C.tab.h"
#include "nodes.h"
#include <stdio.h>
#include <ctype.h>
#include "value.h"
#include "mc_env.h"
#include "token.h"
#include "string.h"
#include "regstack.h"
#include "hashtable.h"

extern TOKEN* new_token(int);
extern int isempty() ;
extern int isfull() ;
extern TOKEN* pop();
extern TOKEN* pop_arg();
extern TOKEN* peep();
extern int push(TOKEN*);
extern int push_arg(TOKEN*);
extern BB* insert(TOKEN*,TAC*); 

extern TOKEN *lookup_loc(TOKEN*, FRME*);
extern TOKEN *assign_to_var(TOKEN*, FRME*,TOKEN*);
extern void declare_var(TOKEN*, FRME*);
extern int reg_in_use(TOKEN*, FRME*);


TOKEN* new_lbl(ENV *env){
    TOKEN* lbl = (TOKEN*)malloc(sizeof(TOKEN));
    if(lbl==NULL){printf("fatal: failed to generate destination\n");exit(1);}
    lbl->type=IDENTIFIER;
    lbl->lexeme = (char*)calloc(1,2);
    sprintf(lbl->lexeme,"L%i",env->lblcounter);
    lbl->value = env->lblcounter;
    env->lblcounter++;
    env->currlbl = lbl;
    return lbl;
}

TOKEN * new_dest(ENV *env){
    TOKEN* dst = (TOKEN*)malloc(sizeof(TOKEN));
    if(dst==NULL){printf("fatal: failed to generate destination\n");exit(1);}
    dst->type=IDENTIFIER;
    dst->lexeme = (char*)calloc(1,2);
    sprintf(dst->lexeme,"t%i",env->dstcounter);
    env->dstcounter++;
    int p = push(dst);
    if (p != 0){printf("fatal: exceeded max registers \n");exit(1);}
    return dst;
}

TOKEN * new_arg(ENV *env){
    TOKEN* dst = (TOKEN*)malloc(sizeof(TOKEN));
    if(dst==NULL){printf("fatal: failed to generate destination\n");exit(1);}
    dst->type=IDENTIFIER;
    dst->lexeme = (char*)calloc(1,2);
    sprintf(dst->lexeme,"a%i",env->argcounter);
    env->argcounter++;
    int p = push_arg(dst);
    if (p != 0){printf("fatal: exceeded max parameters in function \n");exit(1);}
    return dst;
}

TOKEN* get_arg(ENV *env){
    TOKEN* popped = pop_arg();
    if(popped != NULL){env->argcounter--;}
    return popped;
}

void reset_args(ENV* env){
    TOKEN* popped = get_arg(env);
    while (popped != NULL){
        popped = get_arg(env);
    }
}

TOKEN* get_dest(ENV *env){
    TOKEN* popped = pop();
    if(popped==NULL){printf("fatal: failed to get destination\n");exit(1);}
    env->dstcounter--;
    return popped;
}

TOKEN* peep_dest(){
    TOKEN* peeped = peep();
    return peeped;
}


TAC* find_last(TAC* tac){
    while(tac->next!=NULL){
        tac = tac->next;
    }
    return tac;
}

TOKEN* find_last_dest(TAC* tac){
    tac = find_last(tac);
    switch (tac->op){
        case tac_plus:
        case tac_minus:
        case tac_div:
        case tac_mod:
        case tac_mult:
            return tac->stac.dst;
        
        case tac_load: 
            return tac->ld.dst;
        
        case tac_store: 
            return tac->ld.src1;
    }
}

ENV *init_env(){
    ENV *env = malloc(sizeof(ENV));
    if (env==NULL) {
        printf("Error! memory not allocated.");
        exit(0);
    }
    env->lblcounter=0;
    env->dstcounter=0;
    env->argcounter=0;
    new_lbl(env);
    new_dest(env);
    return env;
}

TAC* empty_tac() {
    TAC* ans = (TAC*)malloc(sizeof(TAC));
    if (ans==NULL) {
        printf("Error! memory not allocated.");
        exit(0);
    }
    return ans;
}


TAC* new_stac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst){
  TAC* ans = empty_tac();
  ans->op = op;
  ans->stac.src1 = src1;
  ans->stac.src2 = src2;
  ans->stac.dst = dst;
  return ans;
}

TAC* new_proc (TOKEN* name, int arity){
    TAC* ans = empty_tac();
    ans->op = tac_proc;
    ans->proc.name = name;
    ans->proc.arity = arity;
    return ans;
}

TAC* new_load(TOKEN* name, TOKEN* dst){
    TAC* ans = empty_tac();
    ans->op = tac_load;
    ans->ld.src1 = name;
    ans->ld.dst = dst;
    return ans;
}

TAC* new_store(TOKEN* name, TOKEN* dst, FRME *e,ENV* env){
    TAC* ans = empty_tac();
    ans->op = tac_store;
    ans->ld.dst = dst;
    if(lookup_loc(dst,e) == NULL){
        declare_var(dst,e);
    }
    while (reg_in_use(name,e)){
        name = new_dest(env);
    }
    assign_to_var(dst,e,name);
    ans->ld.src1 = name;
    return ans;
}


int count_params(NODE * tree){
    int count = 0;
    if (tree == NULL || tree->type == INT) {return 0;}
    if( tree->type == LEAF && tree->left->type == IDENTIFIER){
        return 1;
    }
    else{
        count += count_params(tree->left);
        count += count_params(tree->right);
        return count;
    }
}

TOKENLIST* get_params(NODE* ids){
    TOKENLIST* tokens = malloc(sizeof(TOKENLIST));
    if((char)ids->type == '~'){
        tokens->name = (TOKEN*)ids->right->left;
        return tokens;
    }
    else{
        if((char)ids->type == ','){
            tokens->name = (TOKEN*)ids->right->right->left;
            tokens->next = get_params(ids->left);
            return tokens;
        }
    }
}

TAC* new_endproc(){
    TAC* ans = empty_tac();
    ans->op = tac_endproc;
    return ans;
}

TAC* new_if(TOKEN* op1, TOKEN* op2, int code, TOKEN* lbl){
    TAC* ans = empty_tac();
    ans->op= tac_if;
    ans->ift.code = code;
    ans->ift.op1 = op1;
    ans->ift.op2 = op2;
    ans->ift.lbl = lbl;
    return ans;
}

TAC* new_goto(TOKEN* lbl){
    TAC* ans = empty_tac();
    ans->op= tac_goto;
    ans->gtl.lbl = lbl;
    return ans;
}

TAC* new_label(TOKEN* lbl){
    TAC* ans = empty_tac();
    ans->op= tac_lbl;
    ans->lbl.name = lbl;
    return ans;
}

TAC* parse_tilde(NODE* tree, FRME* e, ENV* env){
    TAC *tac,*last;
    TOKEN* t;
     if(tree->left->left->type==INT){
        if(tree->right->type == LEAF){
            t = (TOKEN *)tree->right->left;
            TOKEN* new = new_token(CONSTANT);
            TOKEN* reg = get_dest(env);
            new->value = 0;
            tac = new_load(new,reg);
            tac->next = new_store(reg,t,e,env);
            return tac;
        }
        else if((char)tree->right->type == '='){
            t = (TOKEN *)tree->right->left->left;
            tac = gen_tac0(tree->right->right,env,e);
            last = find_last(tac);
            if(last->stac.dst != NULL){
                last->next = new_store(last->stac.dst,t,e,env);
            }
            else{  last->next = new_store(last->ld.dst,t,e,env); }
            return tac;
        }
    }
    tac = gen_tac0(tree->left,env,e);
    last = find_last(tac);
    last->next = gen_tac0(tree->right,env,e);
    return tac;
}

TAC* parse_if(NODE* tree, ENV* env, FRME *e){
    int code = tree->left->type;
    TOKEN* op1 = (TOKEN*)tree->left->left->left;
    TOKEN* op2 = (TOKEN*)tree->left->right->left;
    TAC* last1,*last2;
    new_lbl(env);
    TAC* tacif = new_if(op1,op2,code,env->currlbl);
    if(tree->right->type == ELSE){
        TAC* consequent = gen_tac0(tree->right->left,env,e);
        if(peep_dest(env)==NULL){
            new_dest(env);
        }
        TAC* alternative = gen_tac0(tree->right->right,env,e);
        TAC* altlbl = new_label(env->currlbl);
        new_lbl(env);
        TAC* gtl = new_goto(env->currlbl);

        last1 = find_last(alternative);
        last1->next = new_label(env->currlbl);
        altlbl->next = alternative;
        gtl->next = altlbl;
        last2 = find_last(consequent);
        last2->next = gtl;
        tacif->next = consequent;
        return tacif;
    }
    else{
        TAC* consequent = gen_tac0(tree->right,env,e);
        consequent->next = new_label(env->currlbl);
        tacif->next = consequent;
        return tacif;
    }
}

int count_args(NODE * tree){
    int count = 0;
    if (tree == NULL) {return 0;}
    if( tree->type == LEAF){
        return 1;
    }
    else{
        count += count_args(tree->left);
        count += count_args(tree->right);
        return count;
    }
}

TOKENLIST* get_args(NODE *tree){
    TOKENLIST* tokens = malloc(sizeof(TOKENLIST));

    if(tree->type == LEAF){
        tokens->name = (TOKEN*)tree->left;
        return tokens;
    }
    else{
        if((char)tree->type == ','){
            tokens->name = (TOKEN*)tree->right->left;
            tokens->next = get_args(tree->left);
            return tokens;
        }
    }
}

TAC* load_args(ENV* env, int arity,TOKENLIST* args){
    TAC* ans = malloc(sizeof(TAC));
    if(arity == 0 || args == NULL){
        return NULL;
    }
    else{
        ans = new_load(args->name,new_arg(env));
        arity--;
        ans->next = load_args(env,arity,args->next);
        return ans;
    }
}

TAC* new_call(NODE* tree, ENV* env){
    TAC* ans = empty_tac();
    ans->op = tac_call;
    ans->call.name = (TOKEN*)tree->left->left;
    ans->call.arity = count_args(tree->right);
    return ans;
}

TAC* load_and_call(NODE* tree,ENV* env){
    TAC* ans = empty_tac();
    int arity = count_args(tree->right);
    TOKENLIST* args = get_args(tree->right);
    ans = load_args(env,arity,args);
    TAC* last = find_last(ans);
    last->next = new_call(tree,env);
    return ans;
}

TAC* new_return(NODE* tree, ENV* env, FRME* e){
    TAC* ans = empty_tac();
    ans->op = tac_rtn;
    if (tree->type==LEAF){
        TOKEN *t = (TOKEN *)tree->left;
        ans->rtn.type = t->type;
        ans->rtn.v = t;
    }
    else if (tree->type==APPLY){
       ans = load_and_call(tree,env);
    }
    else{
        TAC* tac = gen_tac0(tree,env,e);
        TOKEN* t = find_last_dest(tac);
        TAC* last = find_last(tac);
        ans->rtn.type = t->type;
        ans->rtn.v = t;
        last->next = ans;
        return tac;
    }
    return ans;
}

TAC *gen_tac0(NODE *tree, ENV* env, FRME* e){

    TOKEN *left = malloc(sizeof(TOKEN)), *right = malloc(sizeof(TOKEN));
    TAC *tac, *last;
    TOKEN *t;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
            t = (TOKEN *)tree->left;
            tac = new_load(t,new_dest(env));
            return tac;
        }
    char c = (char)tree->type;
    if (isgraph(c) || c==' ') {
        switch(c){
            default: printf("fatal: unknown token type '%d'\n",c); exit(1);
            
            case '~':
               return parse_tilde(tree,e,env);
            case 'D':
                tac = gen_tac0(tree->left,env,e);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,env,e);
                last = find_last(tac);
                last->next = new_endproc();
                reset_args(env);
                return tac;
            case 'd':
                return gen_tac0(tree->right,env,e);
            case 'F':
                left = (TOKEN*)tree->left->left;
                return new_proc(left,count_params(tree->right));
            case ';':
                tac = gen_tac0(tree->left,env,e);
                last = find_last(tac);
                if(peep_dest() == NULL){
                    new_dest(env);
                }
                last->next = gen_tac0(tree->right,env,e);
                return tac;
            case '=':
                tac = gen_tac0(tree->right,env,e);
                last = find_last(tac);
                t = (TOKEN *)tree->left->left;
                if(last->stac.dst != NULL){
                    last->next = new_store(last->stac.dst,t,e,env);
                }
                else{  last->next = new_store(last->ld.dst,t,e,env); }
                return tac;
            case '+':
                new_dest(env);
                tac = gen_tac0(tree->left,env,e);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e);
                right = find_last_dest(last->next);
                last = find_last(last);
                new_dest(env);
                last->next = new_stac(tac_plus,left,right,get_dest(env));
                return tac;
            case '-':
                new_dest(env);
                tac = gen_tac0(tree->left,env,e);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e);
                right = find_last_dest(last->next);
                last = find_last(last);
                new_dest(env);
                last->next = new_stac(tac_minus,left,right,get_dest(env));
                return tac;
            case '*':
                new_dest(env);
                tac = gen_tac0(tree->left,env,e);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e);
                right = find_last_dest(last->next);
                last = find_last(last);
                new_dest(env);
                last->next = new_stac(tac_mult,left,right,get_dest(env));
                return tac;
            case '/':
                new_dest(env);
                tac = gen_tac0(tree->left,env,e);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e);
                right = find_last_dest(last->next);
                last = find_last(last);
                new_dest(env);
                last->next = new_stac(tac_div,left,right,get_dest(env));
                return tac;
            case '%':
                new_dest(env);
                tac = gen_tac0(tree->left,env,e);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e);
                right = find_last_dest(last->next);
                last = find_last(last);
                new_dest(env);
                last->next = new_stac(tac_mod,left,right,get_dest(env));
                return tac;
        }
    }
    switch(tree->type){
    default: printf("fatal: unknown token type '%c'\n", tree->type); exit(1);
    case RETURN:  
        return new_return(tree->left,env,e);
    case IF:
        return parse_if(tree,env,e);
    case APPLY: 
        return new_call(tree,env);
    }
}

TAC* find_in_seq(TAC* seq, TAC* target){
    while(seq!=target){
        seq = seq->next;
    }
    return seq;
}

BB* find_bb(BB** bbs, TOKEN* id, int size){

    for(int i=0; i<size; i++){
        if(bbs[i] != NULL && bbs[i]->id == id){
            return bbs[i];
        }
    }
    return NULL;
}

BB* find_next_bb(BB** bbs, TOKEN* id, int size){
    for(int i=0; i<size; i++){
        if(bbs[i] != NULL && bbs[i]->id->value == (id->value+1)){
            return bbs[i];
        }
    }
    return NULL;
}

BB** gen_bbs(TAC* tac){
    static BB* bbs[10];
    //bb->nexts = malloc(sizeof(BB)*2);
    TAC *curr;
    int i =0;
    int id = 0;
    while(tac != NULL){
        BB* bb = malloc(sizeof(BB));
        bb->leader = tac;
        curr = tac->next;
        while(curr->op != tac_if && curr->op != tac_goto && curr->next != NULL && curr->next->op != tac_lbl){
            curr = curr->next;
        }
        tac = curr->next;
        curr = find_in_seq(bb->leader,curr);
        /* switch(curr->op){
            case tac_if:
                bb->nexts[0] = gen_bbs(tac);
                bb->nexts[1] = insert(curr->ift.lbl,NULL);
                break;
            case tac_goto:
                bb->nexts[0] = insert(curr->gtl.lbl,NULL);
                break;              
        }  */
        curr->next = NULL;
        bbs[i] = bb;
        i++;

        if(bb->leader->op == tac_lbl){
            bb->id = bb->leader->lbl.name;
        }
        else{
            TOKEN* c = new_token(CONSTANT); c->value = id;
            bb->id = c;
            id++;
        }
    }
    TAC* transfer;
    i = 0;
    while(bbs[i] != NULL){
        transfer = find_last(bbs[i]->leader);
        if(transfer->op == tac_goto){
            bbs[i]->nexts[0] = find_bb(bbs,transfer->gtl.lbl,10);
        }
        else{
            bbs[i]->nexts[0] = find_next_bb(bbs,bbs[i]->id,10);
            if(transfer->op == tac_if){
                bbs[i]->nexts[1] = find_bb(bbs,transfer->ift.lbl,10);
            }
        }
        i++;
    }
    return bbs;
}

BB **gen_tac(NODE* tree){
    ENV *env = init_env();
    FRME* e = malloc(sizeof(FRME));
    TAC* tac = gen_tac0(tree,env,e);
    BB** bbs = gen_bbs(tac);
    return bbs;
}