#include<stdio.h>
#include<stdlib.h>
#include "mpc.h"
#ifdef _WIN32
#include<string.h>

char buffer[2048];
char *readline(char *s){
       fputs(s,stdout);
       fgets(buffer,2048,stdin);

      char *cpy = malloc(strlen(buffer)+1);

      strcpy(cpy,buffer);
      cpy[strlen(buffer)-1] = '\0';
      return cpy;
}

//如果没有定义，就定义一个假的的历史记录的函数
void add_history(char *history){};

#else
#include<readline/readline.h>
#include<readline/history.h>
#endif

typedef struct lval{
     //这是当前的结构体的类型
    int type;
    long number;
    
    //当前的结构体的错误
    char* err;
    //这是当前的结构体的操作符
    char* sym;
    //这是记录有多少个lval*
    int count;
    //这是声明的子结构体
    struct lval** child;
} lval;

void lvalprint(lval* x);
void lval_expr_print(lval* x,char open,char close);
lval* lval_eval_sexp(lval* v);
lval* lval_eval(lval* v);

//这是分别是声明的子结构体的数目、当前结构体的错误、当前结构体的操作符和表达式
enum{LVAL_NUM,LVAL_ERR,LVAL_SYM,LVAL_SEXP,LVAL_QEXP};
     
lval* lval_num(long num){
     lval* x = malloc(sizeof(lval));
     x->type = LVAL_NUM;
     x->number = num;
     return x;
}

lval* lval_err(char* err){
     lval* x = malloc(sizeof(lval));
     x->type = LVAL_ERR;
     x->err = malloc(strlen(err)+1);
     strcpy(x->err,err);
     return x;
}

lval* lval_sym(char* sym){
     lval* x = malloc(sizeof(lval));
     x->type = LVAL_SYM;
     x->sym = malloc(strlen(sym)+1);
     strcpy(x->sym,sym);
     return x;
}

lval* lval_child(void){
     lval* x = malloc(sizeof(lval));
     x->type = LVAL_SEXP;
     x->count = 0;
     x->child = NULL;
     return x;
}

lval* lval_qexp(void){
     lval* x = malloc(sizeof(lval));
     x->type = LVAL_QEXP; 
     x->count = 0;
     x->child = NULL;
     return x;
}

void lval_free(lval* x){
     switch(x->type){
       case LVAL_NUM: break;
       case LVAL_ERR: free(x->err); break;

       case LVAL_SYM: free(x->sym); break;

       case LVAL_QEXP:
       case LVAL_SEXP: 
           for(int i=0;i<x->count;i++){
               lval_free(x->child[i]);
           }
           free(x->child);
           break;
     }
     free(x);
}


lval* lval_add(lval* x,lval* y){
       x->count++;
       x->child = realloc(x->child,sizeof(lval*)*x->count);
       x->child[x->count-1] = y;
       return x;
}

lval* lval_read_num(mpc_ast_t* t){
    errno =0;
    long x = strtol(t->contents,NULL,10);
    return errno != ERANGE ? lval_num(x) : lval_err("输入参数超出数学函数定义的范围");
}


//阅读s_expression
lval* lval_read(mpc_ast_t* t){
     if(strstr(t->tag,"number")){return lval_read_num(t);}
     if(strstr(t->tag,"symbol")){return lval_sym(t->contents);}

     lval* x;
     //这里是一个问题,这里是我们的开始的那个导向符
     if(strcmp(t->tag,">") == 0){x = lval_child();}
     if(strstr(t->tag,"sexpresion")){x = lval_child();}
     if(strstr(t->tag,"qexpresion")){x = lval_qexp();}

    for(int i=0;i<t->children_num;i++){
        if(strcmp(t->children[i]->contents,"(") == 0){continue;}
        if(strcmp(t->children[i]->contents,")") == 0){continue;}
        if(strcmp(t->children[i]->contents,"{") == 0){continue;}
        if(strcmp(t->children[i]->contents,"}") == 0){continue;}
        if(strcmp(t->children[i]->tag,"regex") == 0){continue;}
        x = lval_add(x,lval_read(t->children[i]));
    }
   
    return x;
}


void lval_expr_print(lval* x,char open,char close){
     putchar(open);

     for(int i=0;i<x->count;i++){
         lvalprint(x->child[i]);
         if(i != (x->count-1)){
              putchar(' ');
         }
     }

     putchar(close);
}

void lvalprint(lval* x){
    switch(x->type){
        case LVAL_NUM: printf("%li",x->number);break;
        case LVAL_SYM: printf("%s",x->sym);break;
        case LVAL_ERR: printf("Error:%s",x->err);break;
        case LVAL_QEXP:
            lval_expr_print(x,'{','}');
            break;
        case LVAL_SEXP:
            lval_expr_print(x,'(',')');
            break;
    }
}

void lval_println(lval *x){lvalprint(x);putchar('\n');}

lval* lval_pop(lval* v,int i){
     lval* tmp = v->child[i];
     
     memmove(&v->child[i],&v->child[i+1],sizeof(lval*)*(v->count-i-1));

     v->count--;

     v->child = realloc(v->child,sizeof(lval*)*v->count);

     return tmp;
}

lval* lval_take(lval* v,int i){
     lval* tmp = lval_pop(v,i);
     lval_free(v);
     return tmp;
}

lval* builtin_op(lval* v,char* op){
     for(int i=0;i<v->count;i++){
          if(v->child[i]->type != LVAL_NUM){
               lval_free(v);
               return lval_err("当前的操作数有问题!");
          }
      }

      lval* x = lval_pop(v,0);
      
     if( (strcmp(op,"-") == 0) && v->count == 0){
         x->number = -x->number;
     }

   //  lval* y = lval_pop(v,0);

    while(v->count > 0){
       lval* y = lval_pop(v,0);
 
       if(strcmp(op,"+") == 0){x->number += y->number;}
       if(strcmp(op,"-") == 0){x->number -= y->number;}
       if(strcmp(op,"*") == 0){x->number *= y->number;}
       if(strcmp(op,"/") == 0){
           if(y->number == 0){
              lval_free(x);
              lval_free(y);
             // return lval_err("除数不能出现0!");   这样也是可以的
             x = lval_err("除数不能出现0!");
             break;
           }
          x->number /= y->number;
       }
     lval_free(y);
    }

    lval_free(v);
    return x;
}

lval* lval_eval_sexp(lval* v){
     for(int i=0;i<v->count;i++){
         v->child[i] = lval_eval(v->child[i]);
     }

     for(int i=0;i<v->count;i++){
        if(v->child[i]->type == LVAL_ERR){
           return lval_take(v,i);
        }
     }

     if(v->count == 0){return v;}

     if(v->count == 1){return lval_pop(v,0);}

     lval* f = lval_pop(v,0);

     if(f->type != LVAL_SYM){
          lval_free(f);
          lval_free(v);
          lval_err("第一个操作不是操作符");
     }

     lval* result = builtin_op(v,f->sym);
     lval_free(f);
     return result;
}

lval* lval_eval(lval* v){
     if(v->type == LVAL_SEXP){return lval_eval_sexp(v);}
   
     return v;
}

int main(int argc,char** argv){
       mpc_parser_t* Number = mpc_new("number");
       mpc_parser_t* Symbol = mpc_new("symbol");
       mpc_parser_t* Sexpresion = mpc_new("sexpresion");
       mpc_parser_t* Qexpresion = mpc_new("qexpresion"); 
       mpc_parser_t* Expresion = mpc_new("expresion");
       mpc_parser_t* Lispy = mpc_new("lispy");

       mpca_lang(MPCA_LANG_DEFAULT,
           "number: /-?[0-9]+/;       \
            symbol: '+' | '-' | '*' | '/'; \
           qexpresion: '{' <expresion>* '}'; \
            sexpresion: '('<expresion>*')' ;    \
            expresion: <number> | <symbol> | <sexpresion> | <qexpresion>; \
            lispy: /^/ <expresion>* /$/; \
           ",
           Number,Symbol,Qexpresion,Sexpresion,Expresion,Lispy
       );

       puts("Welcome to my lisp 1.5");
       puts("if you get wrong,please press ctrl+c to stop");

        while(1){
             char *input = readline("Lisp >");
             add_history(input);

             mpc_result_t result;

             if(mpc_parse("<stdin>",input,Lispy,&result)){
                 lval* r = lval_eval(lval_read(result.output));
                 lval_println(r);
                 lval_free(r);
                 mpc_ast_delete(result.output);
               }else{
                   mpc_err_print(result.error);
                   mpc_err_delete(result.error);
               }
          free(input);
      }

      mpc_cleanup(5,Number,Symbol,Sexpresion,Expresion,Lispy);
      return 0;
}
