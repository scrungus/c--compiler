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
extern int reg_in_use(int, FRME*);
extern void delete_constants(FRME*);


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

TOKEN * new_dest(FRME *e){
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
    new_lbl(env);
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

TAC* new_proc (TOKEN* name, int arity, TOKENLIST* args){
    TAC* ans = empty_tac();
    ans->op = tac_proc;
    ans->proc.name = name;
    ans->proc.arity = arity;
    ans->proc.args = args;
    return ans;
}

TAC* new_innerproc (TOKEN* name, int arity, TOKENLIST* args){
    TAC* ans = empty_tac();
    ans->op = tac_innerproc;
    ans->proc.name = name;
    ans->proc.arity = arity;
    ans->proc.args = args;
    return ans;
}

TAC* new_load(TOKEN* name, FRME* e){
    TAC* ans = empty_tac();
    ans->op = tac_load;
    ans->ld.src1 = name;
    TOKEN* t = lookup_loc(name,e);
    if(t == NULL){
        t = new_dest(e);
        declare_var(name,e);
        assign_to_var(name,e,t);
    }
    ans->ld.dst = t;
    return ans;
}

TAC* new_store(TOKEN* name, TOKEN* dst, FRME *e,ENV* env){
    TAC* ans = empty_tac();
    ans->op = tac_store;
    ans->ld.dst = dst;
    TOKEN* t = lookup_loc(dst,e);
    if(t == NULL){
        declare_var(dst,e);
    }
    assign_to_var(dst,e,name);
    ans->ld.src1 = name;
    return ans;
}


int count_params(NODE * tree){
    int count = 0;
    if (tree == NULL || tree->type == INT || tree->type == FUNCTION || tree->type == STRING_LITERAL) {return 0;}
    if( tree->type == LEAF && tree->left->type==IDENTIFIER){
        return 1;
    }
    else{
        count += count_params(tree->left);
        count += count_params(tree->right);
        return count;
    }
}

TOKENLIST* get_params(NODE* ids){
    if(ids == NULL){return NULL;}
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

TAC* parse_tilde(NODE* tree, FRME* e, ENV* env, int depth){
    TAC *tac,*last;
    TOKEN* t;
     if(tree->left->left->type==INT){
        if(tree->right->type == LEAF){
            t = (TOKEN *)tree->right->left;
            TOKEN* new = new_token(CONSTANT);
            TOKEN* reg = new_dest(e);
            new->value = 0;
            tac = new_load(new,e);
            tac->next = new_store(reg,t,e,env);
            return tac;
        }
        else if((char)tree->right->type == '='){
            t = (TOKEN *)tree->right->left->left;
            tac = gen_tac0(tree->right->right,env,e,depth);
            last = find_last(tac);
            if(last->stac.dst != NULL){
                last->next = new_store(last->stac.dst,t,e,env);
            }
            else{  last->next = new_store(last->ld.dst,t,e,env); }
            return tac;
        }
    }
    tac = gen_tac0(tree->left,env,e,depth);
    last = find_last(tac);
    last->next = gen_tac0(tree->right,env,e,depth);
    return tac;
}

TAC* parse_if(NODE* tree, ENV* env, FRME *e, int depth){
    int code = tree->left->type;
    TOKEN* op1 = (TOKEN*)tree->left->left->left;
    TOKEN* op2 = (TOKEN*)tree->left->right->left;
    TAC* last1,*last2;
    new_lbl(env);
    TAC* tacif = new_if(op1,op2,code,env->currlbl);
    if(tree->right->type == ELSE){
        TAC* consequent = gen_tac0(tree->right->left,env,e,depth);
        TAC* alternative = gen_tac0(tree->right->right,env,e,depth);
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
        TAC* consequent = gen_tac0(tree->right,env,e,depth);
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

TOKENLIST* get_args(NODE *tree, ENV* env, FRME* e){
    TOKENLIST* tokens = malloc(sizeof(TOKENLIST));
    if(tree == NULL){return NULL;}
    char c = (char)tree->type;
    if(tree->type == LEAF){
        tokens->name = (TOKEN*)tree->left;
        return tokens;
    }
    else{
        if((char)tree->type == ','){
            tokens->name = (TOKEN*)tree->right->left;
            tokens->next = get_args(tree->left,env,e);
            return tokens;
        }
    }
}

TAC* new_call(NODE* tree, ENV* env, FRME* e){
    TAC* ans = empty_tac();
    ans->op = tac_call;
    ans->call.name = (TOKEN*)tree->left->left;
    ans->call.arity = count_args(tree->right);
    ans->call.args = get_args(tree->right,env,e);
    return ans;
}

TAC* new_return(NODE* tree, ENV* env, FRME* e, int depth){
    TAC* ans = empty_tac();
    TAC* last; 
    ans->op = tac_rtn;
    if (tree->type==LEAF){
        TOKEN *t = (TOKEN *)tree->left;
        ans->rtn.type = t->type;
        ans->rtn.v = t;
    }
    else if (tree->type==APPLY){
       ans = new_call(tree,env,e);
       last = find_last(ans);
       last->next = empty_tac();
       last->next->op = tac_rtn;
       last->next->rtn.type = tac_call;
    }
    else{
        TAC* tac = gen_tac0(tree,env,e,depth);
        TOKEN* t = find_last_dest(tac);
        TAC* last = find_last(tac);
        ans->rtn.type = t->type;
        ans->rtn.v = t;
        last->next = ans;
        return tac;
    }
    delete_constants(e);
    return ans;
}

TAC *gen_tac0(NODE *tree, ENV* env, FRME* e, int depth){

    TOKEN *left = malloc(sizeof(TOKEN)), *right = malloc(sizeof(TOKEN));
    TAC *tac, *last;
    TOKEN *t;

    if (tree==NULL) {printf("fatal: no tree received\n") ; exit(1);}
    if (tree->type==LEAF){
            t = (TOKEN *)tree->left;
            tac = new_load(t,e);
            return tac;
        }
    char c = (char)tree->type;
    if (isgraph(c) || c==' ') {
        switch(c){
            default: printf("fatal: unknown token type '%d'\n",c); exit(1);
            
            case '~':
               return parse_tilde(tree,e,env,depth);
            case 'D':
                tac = gen_tac0(tree->left,env,e,++depth);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,env,e,++depth);
                last = find_last(tac);
                last->next = new_endproc();
                return tac;
            case 'd':
                return gen_tac0(tree->right,env,e,depth);
            case 'F':
                left = (TOKEN*)tree->left->left;
                if(depth > 1){
                    return new_innerproc(left,count_params(tree->right),get_params(tree->right));
                }
                else{
                    return new_proc(left,count_params(tree->right),get_params(tree->right));
                }
                
            case ';':
                tac = gen_tac0(tree->left,env,e,depth);
                last = find_last(tac);
                last->next = gen_tac0(tree->right,env,e,depth);
                return tac;
            case '=':
                tac = gen_tac0(tree->right,env,e,depth);
                last = find_last(tac);
                t = (TOKEN *)tree->left->left;
                if(last->stac.dst != NULL){
                    last->next = new_store(last->stac.dst,t,e,env);
                }
                else if(last->op = tac_call){
                    last->next = new_store(new_dest(e),t,e,env);
                }
                else{  last->next = new_store(last->ld.dst,t,e,env); }
                delete_constants(e);
                return tac;
            case '+':
                tac = gen_tac0(tree->left,env,e,depth);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e,depth);
                right = find_last_dest(last->next);
                last = find_last(last);
                t = new_token(CONSTANT);
                declare_var(t,e);
                assign_to_var(t,e,new_dest(e));
                last->next = new_stac(tac_plus,left,right,lookup_loc(t,e));
                return tac;
            case '-':
                tac = gen_tac0(tree->left,env,e,depth);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e,depth);
                right = find_last_dest(last->next);
                last = find_last(last);
                t = new_token(CONSTANT);
                declare_var(t,e);
                assign_to_var(t,e,new_dest(e));
                last->next = new_stac(tac_minus,left,right,lookup_loc(t,e));
                return tac;
            case '*':
                tac = gen_tac0(tree->left,env,e,depth);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e,depth);
                right = find_last_dest(last->next);
                last = find_last(last);
                t = new_token(CONSTANT);
                declare_var(t,e);
                assign_to_var(t,e,new_dest(e));
                last->next = new_stac(tac_mult,left,right,lookup_loc(t,e));
                return tac;
            case '/':
                tac = gen_tac0(tree->left,env,e,depth);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e,depth);
                right = find_last_dest(last->next);
                last = find_last(last);
                t = new_token(CONSTANT);
                declare_var(t,e);
                assign_to_var(t,e,new_dest(e));
                last->next = new_stac(tac_div,left,right,lookup_loc(t,e));
                return tac;
            case '%':
                tac = gen_tac0(tree->left,env,e,depth);
                left = find_last_dest(tac);
                last = find_last(tac);   
                last->next = gen_tac0(tree->right,env,e,depth);
                right = find_last_dest(last->next);
                last = find_last(last);
                t = new_token(CONSTANT);
                declare_var(t,e);
                assign_to_var(t,e,new_dest(e));
                last->next = new_stac(tac_mod,left,right,lookup_loc(t,e));
                return tac;
        }
    }
    switch(tree->type){
    default: printf("fatal: unknown token type '%c'\n", tree->type); exit(1);
    case RETURN:  
        return new_return(tree->left,env,e,depth);
    case IF:
        return parse_if(tree,env,e,depth);
    case APPLY: 
        return new_call(tree,env,e);
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

TAC *gen_tac(NODE* tree){
    ENV *env = init_env();
    FRME* e = malloc(sizeof(FRME));
    TAC* tac = gen_tac0(tree,env,e,0);
    //BB** bbs = gen_bbs(tac);
    return tac;
}