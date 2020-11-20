//eliminar print parse args

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
    printf("en execute\n");
    char *args[ARGS_SIZE];
    printf("ir a parse\n");
    //trocea la línea y si está vacia o solo hay comentarios no hace nada
    if(parse_args(args, line) != 0){
        printf("ir a check internal\n");
        //si hay tokens comprueba si se trata de algún comando interno
        check_internal(args);
    }
}

//trocea la línea en tokens
int parse_args(char **args, char *line){
    printf("en parse\n");
    int ntokens = 0;
    args[ntokens] = strtok(line, s);
    //coge los tokens hasta NULL o el inicio de un comentario
    while((args[ntokens] != NULL) && (args[ntokens][0] != '#')){
        ntokens++;

        //eliminar
        printf("token%d: %s\n",ntokens, args[ntokens-1]); 
        ///////////

        args[ntokens] = strtok(NULL, s);
    }
    //último token ha de ser NULL
    args[ntokens] = NULL;
    printf("fin parse\n");
    return ntokens;
}

//compara el primer elemento del array para saber si se trata de un comando interno
int check_internal(char **args){
    printf("en check internal\n");
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

//quita las comillas iniciales y finales del string pasado por parámetro
char *quitar_comillas(char *str){
    char *aux = malloc(strlen(str)-2);
    int n = 0;
    for(int i = 1; i < strlen(str)-1; i++){
        aux[n] = str[i];
        n++;
    }
    return aux;
}

//tratamiento del comando interno cd
int internal_cd(char **args){
    printf("Estoy en internal cd\n");
    int i = 0;
    //comprueba si existe otro argumento
    if(args[1] != NULL){
        char *c = malloc(COMMAND_LINE_SIZE);
        strcat(c, args[1]);
        //comprueba si hay comillas dobles
        if(strchr(args[1], 34)){
            printf("comillas dobles\n");
            //enlazamos todos los tokens que pertenecen al nuevo directorio
            i = 2;
            while((args[i] != NULL) && !strchr(args[i], 34)){
                strcat(c, " ");
                strcat(c, args[i]);
                i++;
            }
            strcat(c, " ");
            strcat(c, args[i]);
            //quitamos las comillas del nuevo string
            c = quitar_comillas(c);
        }else if(strchr(args[1], 39)){//comillas simples
            printf("comillas simples\n");
            //enlazamos todos los tokens que pertenecen al nuevo directorio
            i = 2;
            while((args[i] != NULL) && !strchr(args[i], 39)){
                strcat(c, " ");
                strcat(c, args[i]);
                i++;
            }
            strcat(c, " ");
            strcat(c, args[i]);
            //quitamos las comillas del nuevo string
            c = quitar_comillas(c);
        }else if(strchr(args[1], 92)){//slash
            printf("slash\n");
            //quitamos el/los slash
            i = 0;
            while(i != strlen(c)){
                if(c[i] == '\\'){
                    c[i] = ' ';
                }
                i++;
            }
        }
        printf("go to %s\n", c);
        //si chdir da error porque no encentra el archivo sale un error
        if(chdir(c) == -1){
            fprintf(stderr, "cd: %s: No existe el archivo o el directorio\n", c);
        }
    } else{ //si no hay otro argumento va a home
        printf("go to home\n");
        chdir(getenv("HOME"));
    }
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("nuevo dir: %s\n", cwd);
    //cambiar el directorio de la variable de entorno
    setenv("PWD", cwd, 1);
}

int internal_export(char **args){
    printf("Estoy en internal export\n");
    char *vec[2];
    vec[0] = strtok(args[1], "=");
    printf("vec[0]=%s\n", vec[0]);
    vec[1] = strtok(NULL, "=");
    printf("vec[1]=%s\n", vec[1]);
    if(vec[1] == NULL){
        fprintf(stderr, "export: Introducir argumento NOMBRE=VALOR\n");
    }else{
        printf("var: %s\n", getenv(vec[0]));
        setenv(vec[0], vec[1], 1);
        printf("new var: %s\n", getenv(vec[0]));
    }
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
        printf("a execute\n");
        execute_line(line);
    }
    return 0;
}