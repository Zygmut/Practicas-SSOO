//eliminar print parse args

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define AZUL "\x1b[34m"
#define BLANCO "\x1b[37m"
#define VERDE "\x1b[32m"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
const char s[5] = " \n\t\r";
#define NJOBS 64

struct info_process {
    pid_t pid;
    char status; // ’E’, ‘D’, ‘F’
    char command_line[COMMAND_LINE_SIZE]; // Comando
};
static struct info_process jobs_list[NJOBS];
static struct info_process *padre;

void imprimir_prompt();
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
void reaper(int signum);
void ctrlc();

//imprime el prompt
void imprimir_prompt(){
    printf(VERDE "%s" BLANCO ":" AZUL "%s" BLANCO "$ ", getenv("USERNAME"), getenv("PWD"));
}

//lee una línea de máximo 1024 carácteres
char *read_line(char *line){
    imprimir_prompt();
    char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin);
    if(!ptr){  //ptr==NULL
        printf("\r");
        if(feof(stdin)){
            exit(0);
        }
        else{
            ptr = line;
            ptr[0] = 0; 
        }
    }
    return ptr;
}

//ejecuta la línea pasada por parámetro
int execute_line(char *line){
    char *args[ARGS_SIZE];
    pid_t pid;
    //trocea la línea y si está vacia o solo hay comentarios no hace nada
    if(parse_args(args, line) != 0){
        printf("[execute_line() -> PID padre: %d(%s)]\n", padre->pid, padre->command_line);
        //si hay tokens comprueba si se trata de algún comando interno
        if(check_internal(args) == 0){
            //si no se trata de un comando interno hace un fork
            signal(SIGCHLD, reaper);
            pid = fork();
            strcpy(jobs_list[0].command_line, line);
            jobs_list[0].status = 'E';
            if(pid == 0){
                jobs_list[0].pid = getpid();
                printf("[execute_line() -> PID: %d(%s)]\n", jobs_list[0].pid, jobs_list[0].command_line);
                signal(SIGCHLD, SIG_DFL);
                signal(SIGINT, SIG_IGN);
                //si no encuentra el comando externo sale un mensaje de error
                if(execvp(args[0], args) == -1){
                    fprintf(stderr, "%s: no encontró la orden\n", args[0]);
                    exit(0);
                }
            }else if(pid > 0){ //padre
                jobs_list[0].pid = pid;
            }
            //mientras no acaba el hijo en primer plano espera
            while((jobs_list[0].pid  != 0)){
                pause();
            }
        }
    }
}

//trocea la línea en tokens
int parse_args(char **args, char *line){
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
            //quitamos el/los slash
            i = 0;
            while(i != strlen(c)){
                if(c[i] == '\\'){
                    c[i] = ' ';
                }
                i++;
            }
        }
        //si chdir da error porque no encentra el archivo sale un error
        if(chdir(c) == -1){
            fprintf(stderr, "cd: %s: No existe el archivo o el directorio\n", c);
        }
    } else{ //si no hay otro argumento va a home
        chdir(getenv("HOME"));
    }
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    //cambiar el directorio de la variable de entorno
    setenv("PWD", cwd, 1);
}

int internal_export(char **args){
    printf("Estoy en internal export\n");
    char *vec[2];
    vec[0] = strtok(args[1], "=");
    vec[1] = strtok(NULL, "=");
    if(vec[1] == NULL){
        fprintf(stderr, "export: Introducir argumento NOMBRE=VALOR\n");
    }else{
        setenv(vec[0], vec[1], 1);
    }
}

int internal_source(char **args){
    printf("Estoy en internal source\n");
    FILE *fp;
    char buff[COMMAND_LINE_SIZE];
    if(args[1] == NULL){
        fprintf(stderr, "source: se requiere un argumento de nombre de archivo\n");
        fprintf(stderr, "source: uso: source nombredearchivo [argumentos]\n");
    }else{
        fp = fopen(args[1], "r");
        if(fp == NULL){
            fprintf(stderr, "%s: No such file or directory\n", args[1]);
        }else{
            while(fgets(buff, COMMAND_LINE_SIZE, fp) != NULL){
                execute_line(buff);
                fflush(fp);
            }
            fclose(fp);
        }
    }
}

int internal_jobs(char **args){
    printf("Estoy en internal jobs\n");
}

//El reaper se encarga de tratar a los hijos que ya han acabado
void reaper(int signum){
    signal(SIGCHLD, reaper);
    pid_t ended;
            
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        if(ended == jobs_list[0].pid){
            printf("Proceso en foreground finalizado\n");
            jobs_list[0].pid = 0;
            memset(jobs_list[0].command_line, 0, COMMAND_LINE_SIZE);
            jobs_list[0].status = 'F';
        }
        printf("reaper()→ Enterrado proceso con PID %d\n", ended);
    }  
}

void ctrlc(){
    pid_t pid = getpid();
    printf("\n[ctrlc() -> Soy el proceso con PID %d, el proceso en foreground es %d(%s)]\n",
     pid, jobs_list[0].pid, jobs_list[0].command_line);

    if(jobs_list[0].pid > 0){
        if(strcmp(jobs_list[0].command_line, "./nivel4")){
            if (kill(jobs_list[0].pid, SIGTERM)==0) { // Enviamos la señal SIGINT al proceso
                printf("Señal enviada a %d\n", jobs_list[0].pid);
            }else{
                perror("kill\n");
                exit(-1);
            }
        }else{
            printf("[ctrlc() -> Señal 15 no enviada debido a que el proceso en foreground es el shell]\n");
        }
    }else{
        printf("[ctrlc() -> Señal 15 no enviada por %d debido a que no hay proceso en foreground]\n", pid);
    }
    signal(SIGINT, ctrlc);
}

int main(){
    padre = malloc(sizeof(struct info_process));
    strcpy(padre->command_line, "./nivel4");
    padre->status = 'E';
    padre->pid = getpid();

    signal(SIGCHLD, reaper); 
    signal(SIGINT, ctrlc);
    struct info_process *jobs_list = malloc(NJOBS*sizeof(struct info_process));
    char *line = malloc(COMMAND_LINE_SIZE);
    while(read_line(line)){
        execute_line(line);
        memset(line, 0, sizeof(line));
    }
    return 0;
}