#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define AZUL "\x1b[34m"
#define BLANCO "\x1b[37m"
#define VERDE "\x1b[32m"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
const char s[5] = " \n\t\r";

void imprimir_prompt();
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);

//imprime el prompt
void imprimir_prompt(){
    printf(VERDE "%s" BLANCO ":" AZUL "%s" BLANCO "$ ", getenv("USERNAME"), getenv("PWD"));
}

//lee una línea de máximo 1024 carácteres
char *read_line(char *line){
    imprimir_prompt();
    fgets(line, COMMAND_LINE_SIZE, stdin);
    return line;
}

//ejecuta la línea pasada por parámetro
int execute_line(char *line){
    char *args[ARGS_SIZE];
    //trocea la línea y si está vacia o solo hay comentarios no hace nada
    if(parse_args(args, line) != 0){
        //si hay tokens comprueba si se trata de algún comando interno
        check_internal(args);
    }
}

//trocea la línea en tokens
int parse_args(char **args, char *line){
    int ntokens = 0;
    args[ntokens] = strtok(line, s);
    //coge los tokens hasta NULL o el inicio de un comentario
    while((args[ntokens] != NULL) && (args[ntokens][0] != '#')){
        ntokens++;
        printf("token%d: %s\n",ntokens, args[ntokens-1]);
        args[ntokens] = strtok(NULL, s);
    }
    //último token ha de ser NULL
    args[ntokens] = NULL;
    return ntokens;
}

//compara el primer elemento del array para saber si se trata de un comando interno
int check_internal(char **args){
    int check = 0;
    //si el primer token coincide con algún comando se ejecuta el correspondiente
    if(strcmp(args[0], "cd") == 0){
        internal_cd(args);
        check++;
    }else if(strcmp(args[0], "export") == 0){
        internal_export(args);
        check++;       
    }else if(strcmp(args[0], "source") == 0){
        internal_source(args); 
        check++;      
    }else if(strcmp(args[0], "jobs") == 0){
        internal_jobs(args);
        check++;      
    }else if(strcmp(args[0], "exit") == 0){
        exit(0);  
    }
    return check;
}

int internal_cd(char **args){
    printf("Estoy en internal cd\n");
}

int internal_export(char **args){
    printf("Estoy en internal export\n");
}

int internal_source(char **args){
    printf("Estoy en internal source\n");
}

int internal_jobs(char **args){
    printf("Estoy en internal jobs\n");
}

int main(){
    char *line = malloc(COMMAND_LINE_SIZE);
    while(read_line(line)){
        execute_line(line);
    }
    return 0;
}