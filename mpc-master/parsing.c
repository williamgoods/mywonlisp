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

void add_history(char *history){};

#else
#include<readline/readline.h>
#include<readline/history.h>
#endif

typedef struct{
    int type;
    long num;
    int err;
} lval;

enum{LVAL_NUM,LVAL_ERR};

enum{LERR_DIV_ZERO,LERR_BSA_OP,LERR_BSA_NUM};

lval lval_num(long num){
     lval x;
     x.type = LVAL_NUM;
     x.num = num;
     return x;
}

lval lval_err(int err){
     lval x;
     x.type = LVAL_ERR;
     x.err = err;
     return x;
}

void lval_print(lval x){
     switch(x.type){
         case LVAL_NUM:
             printf("最后的数据为:%li",x.num);
             break;
         case LVAL_ERR:
            if(x.err == LERR_DIV_ZERO){
                   printf("当前的错误为除数为零!");
             }
            if(x.err == LERR_BSA_OP){
                  printf("当前的错误为操作符的错误!");
            }
            if(x.err == LERR_BSA_NUM){
                  printf("当前的错误为操作数有误!");
            }
            break;
         default:
             break;
     }
}

void lval_println(lval x){lval_print(x);putchar('\n');}

lval eval_op(lval x,char* option,lval y){
      if(x.type == LVAL_ERR){return x;}
      if(x.type == LVAL_ERR){return y;}       

      if(strcmp("+",option) == 0){return lval_num(x.num + y.num);}
      if(strcmp("-",option) == 0){return lval_num(x.num - y.num);}
      if(strcmp("*",option) == 0){return lval_num(x.num * y.num);}
      if(strcmp("/",option) == 0){
           return y.num == 0 ? lval_err(LERR_DIV_ZERO):lval_num(x.num/y.num);
      }        
      return lval_err(LERR_BSA_OP);  
}

lval eval(mpc_ast_t* t){
      if(strstr(t->tag,"number")){
         errno = 0;
         long num = strtol(t->contents,NULL,10);
         return errno != ERANGE ? lval_num(num):lval_err(LERR_BSA_NUM);
      }
         char* option = t->children[1]->contents;
         printf("children[1]为%s\n",t->children[1]->contents);
         printf("children[2]为%s\n",t->children[2]->contents);
         printf("children[3]为%s\n",t->children[3]->contents);
         lval x = eval(t->children[2]);
         
        // int i = 3;
         if(strstr(t->children[3]->tag,"expression")){
              x = eval_op(x,option,eval(t->children[3]));
             // printf("123123424返回的数值为%li\n",eval(t->children[i]));
             // printf("1234214234变化的数值i为%d\n",i);
             // printf("1234214234变化的数值x为%li\n",x);
             // i++;
         }
       //  printf("最后的数值i为%d\n",i);
      return x;
}

int main(int argc,char** argv){
       mpc_parser_t* Number = mpc_new("number");
       mpc_parser_t* Operator = mpc_new("operator"); 
       mpc_parser_t* Expression = mpc_new("expression"); 
       mpc_parser_t* Lispy = mpc_new("lispy");

       mpca_lang(MPCA_LANG_DEFAULT,
           "number: /-?[0-9]+/;       \
            operator: '+' | '-' | '*' | '/'; \
            expression: <number> | '(' <operator><expression> +')'; \
            lispy: /^/ <operator><expression> + /$/; \
           ",
           Number,Operator,Expression,Lispy
       );
      
       puts("Welcome to my lisp 0.1");
       puts("if you get wrong,please press ctrl+c to stop");

        while(1){
             char *input = readline("Lisp >");
             add_history(input);

             mpc_result_t result;

             if(mpc_parse("<stdin>",input,Lispy,&result)){
                  // mpc_ast_print(result.output);
                  // mpc_ast_delete(result.output);
                  lval lastresult = eval(result.output);
                  lval_println(lastresult);
                  mpc_ast_delete(result.output);
               }else{
                   mpc_err_print(result.error);
                   mpc_err_delete(result.error);
               }
          free(input);
      }

      mpc_cleanup(4,Number,Operator,Expression,Lispy);
      return 0;
}






