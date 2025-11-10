#include <iostream>
#include "shell.c"
#include "../db/db.h"
bool run_mini_ssh(){
    bool run_shell_val = false;
    while(1){
        std::cout<<"username:";
        std::string username;
        std::cin>>username;
        std::cout<<"key:";
        std::string key;
        std::cin>>key;
        if(check_key_username_pair(username,key)){
            run_shell_val = true; 
        }
        break;
    }
    if(run_shell_val){
        run_shell();
    }
}
// int main(){
//     run_mini_ssh();
//     return 0;
// }