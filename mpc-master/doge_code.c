#include "mpc.h"

int main(int argc,char** argv){
      //先定义两个解析器
      mpc_parser_t* Ajective = mpc_or(
           4,mpc_sym("wow"),mpc_sym("many"),mpc_sym("so"),mpc_sym("such")
       );
     mpc_parser_t* Noun = mpc_or(
           5,mpc_sym("lisp"),mpc_sym("language"),mpc_sym("book"),mpc_sym("build"),mpc_sym("c")
      );

     mpc_parser_t* Phare =  mpc_and(2,mpcf_strfold,Ajective,Noun,free);

     mpc_parser_t* Doga = mpc_many(mpcf_strfold,Phare);

     mpc_delete(Doga);

     return 0;
}
