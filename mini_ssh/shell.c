#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include <sys/wait.h>
#include <string.h>

static const char* name = "mysh>";
static const char* cd = "cd";
// const int mx = 1000;
#define mx 100
static char input[mx];
static char* parsed_arguments[mx];

typedef struct cli{
    char* path;
    char** args;
    int len;
    int pipe_detected;
} cli;

void set_argument_len(cli* c,int len){
    c->args = (char**) malloc((len+1)*sizeof(char*));
    c->args[len] = NULL;
    c->len = len;
}
cli* parse_arguments(){
    cli* c  = (cli*)malloc(sizeof(cli));
    char** cur = NULL;
    int last_len = 0;
    int cur_index = 0;
    int spaces = 0;
    for(int i=0;i<mx;i++){
        if(input[i]==' '){
            spaces+=1;
            free(parsed_arguments[cur_index]);
            parsed_arguments[cur_index] = (char*)malloc(last_len+1);
            parsed_arguments[cur_index][last_len] ='\0'; 
            for(int j=0;j<last_len;j++){
                parsed_arguments[cur_index][j] = input[i-last_len+j];
            }
            last_len = 0 ;
            cur_index+=1;
        }
        else if(input[i]=='\n'){
            free(parsed_arguments[cur_index]);
            parsed_arguments[cur_index] = (char*)malloc(last_len+1);
            parsed_arguments[cur_index][last_len] ='\0'; 
            for(int j=0;j<last_len;j++){
                parsed_arguments[cur_index][j] = input[i-last_len+j];
            }
            printf("%d\n",i);
            break;
        }
        else{
            if(input[i]=='|'){
                c->pipe_detected = 1;
            }
            last_len+=1;
        }
    }
    set_argument_len(c,spaces+1);
    c->path = parsed_arguments[0];
    for(int i=0;i<spaces+1;i++){
        c->args[i] = parsed_arguments[i]; 
    }
    return c;
}
void run_shell(){
    while(1){
        size_t line = 0;
        int read;
        printf("%s",name);
        fgets(input,sizeof(input),stdin);
        printf("%s-output>%s",name,input);
        cli* c = parse_arguments();
        int t = fork();
        if (strcmp(c->args[0], "cd") == 0) {
            if (c->args[1] != NULL) {
                if (chdir(c->args[1]) != 0) perror("chdir failed");
            } 
            else {
                fprintf(stderr, "cd: missing argument\n");
            }
            continue;  
        }
        if (strcmp(c->args[0], "exit") == 0) {
            exit(0);  
        }
        if(c->pipe_detected){
            int num_pipes = 0;
            for(int i=0;i<c->len;i++){
                if(strcmp(c->args[i],"|")==0){
                    num_pipes++;
                }
            }
            
        }
        else{
            if(t>0){
                wait(NULL);
            }
            else if(t==0){
                execvp(c->path,c->args);
            }
            else{
                exit(-1);
            }
        }
    }
}