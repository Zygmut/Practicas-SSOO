/*  
    Pau Capellà Ballester
    Alberto Cugat Martín
    Joaquín González Alcover
*/

#define _POSIX_C_SOURCE 200112L
#define USE_READLINE

#include <stdio.h>

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#define AZUL "\x1b[34m"
#define BLANCO "\x1b[37m"
#define VERDE "\x1b[32m"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
const char delim[5] = " \n\t\r";
#define NJOBS 64

struct info_process {
    pid_t pid;
    char status; // ’E’, ‘D’, ‘F’
    char command_line[COMMAND_LINE_SIZE]; // Comando
};
static struct info_process jobs_list[NJOBS];
static struct info_process *padre;

static int n_pids;
static char *current_command_line;
static char *cmdline;

char *imprimir_prompt();
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
void reaper(int signum);
void ctrlc();
int is_background(char *line);
void ctrlz();
int jobs_list_add(pid_t pid, char status, char *command_line);
int jobs_list_find(pid_t pid);
int  jobs_list_remove(int pos);
int is_output_redirection(char **args);

//imprime el prompt
char *imprimir_prompt(){
    char *prompt = malloc(COMMAND_LINE_SIZE);
    strcat(prompt, VERDE);
    strcat(prompt, getenv("USERNAME"));
    strcat(prompt, BLANCO);
    strcat(prompt, ":");
    strcat(prompt, AZUL);
    strcat(prompt, getenv("PWD")); 
    strcat(prompt, BLANCO);
    strcat(prompt, "$ ");
    return prompt;
}

//lee una línea de máximo 1024 carácteres
char *read_line(char *line){
    char *ptr;
    #ifdef USE_READLINE
    char *aux = readline(imprimir_prompt());
    if(aux == NULL){
        exit(0);
    }
    add_history(aux);
    ptr = malloc(strlen(aux)+1);
    int i = 0;
    while(i < strlen(aux)){
        ptr[i] = aux[i];
        i++;
    }
    ptr[i] = '\n';
    strcpy(line, ptr);
    free(ptr);
    #else
    printf("%s", imprimir_prompt());
    ptr = fgets(line, COMMAND_LINE_SIZE, stdin);
    #endif
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
    int background = is_background(line);
    //trocea la línea y si está vacia o solo hay comentarios no hace nada
    if(parse_args(args, line) != 0){
        //si hay tokens comprueba si se trata de algún comando interno
        if(check_internal(args) == 0){
            //si no se trata de un comando interno hace un fork
            signal(SIGCHLD, reaper);
            pid = fork();
            if(pid == 0){
                signal(SIGCHLD, SIG_DFL);
                signal(SIGINT, SIG_IGN); //ctrl+c
                signal(SIGTSTP, SIG_IGN); //ctrl+z
                //mira si hay redireccionamiento a algun fichero
                is_output_redirection(args);
                //si no encuentra el comando externo sale un mensaje de error
                if(execvp(args[0], args) == -1){
                    fprintf(stderr, "%s: no encontró la orden\n", args[0]);
                    exit(0);
                }
            }else if(pid > 0){ //padre
                signal(SIGINT, ctrlc);
                signal(SIGTSTP, ctrlz);
                if(!background){ //foreground
                    jobs_list[0].pid = pid;
                    strcpy(jobs_list[0].command_line, current_command_line);
                    jobs_list[0].status = 'E';
                    //mientras no acaba el hijo en primer plano espera
                    while((jobs_list[0].pid  != 0)){
                        pause();
                    }
                }else{ //background
                    jobs_list_add(pid, 'E', current_command_line);
                    printf("%d. PID: %d\t Line: %s\t Status: %c\n",
                     n_pids, jobs_list[n_pids].pid, jobs_list[n_pids].command_line,
                      jobs_list[n_pids].status);
                }
            }
        }
    }
    return 1;
}

//trocea la línea en tokens
int parse_args(char **args, char *line){
    int ntokens = 0;
    args[ntokens] = strtok(line, delim);
    //coge los tokens hasta NULL o el inicio de un comentario
    while((args[ntokens] != NULL) && (args[ntokens][0] != '#')){
        ntokens++;
        args[ntokens] = strtok(NULL, delim);
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
    }else if(strcmp(args[0], "fg") == 0){
        internal_fg(args);
        check++;
    }else if(strcmp(args[0], "bg") == 0){
        internal_bg(args);
        check++;
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

//quita el & de la linea
char *quita_up_and(char *str){
    char *aux =malloc(strlen(str));
    for(int i = 0; i < strlen(str); i++){
        if((i == strlen(str)-1) && (str[i] == '&')){
            aux[i] = '\0';
        }else{
            aux[i] = str[i];
        }
    }
    return aux;
}

char *poner_up_and(char *str){
    char *aux =malloc(strlen(str)+1);
    for(int i = 0; i < strlen(str); i ++){
        aux[i] = str[i];
    }
    aux[strlen(str)] = '&';
    return aux;
}

//tratamiento del comando interno cd
int internal_cd(char **args){
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
    return 1;
}

int internal_export(char **args){
    char *vec[2];
    vec[0] = strtok(args[1], "=");
    vec[1] = strtok(NULL, "=");
    if(vec[1] == NULL){
        fprintf(stderr, "export: Introducir argumento NOMBRE=VALOR\n");
    }else{
        setenv(vec[0], vec[1], 1);
    }
    return 1;
}

int internal_source(char **args){
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
            //ejecutamos todo el fichero de comandos
            while(fgets(buff, COMMAND_LINE_SIZE, fp) != NULL){
                execute_line(buff);
                fflush(fp);
            }
            fclose(fp);
        }
    }
    return 1;
}

int internal_jobs(char **args){
    for(int i = 1; i <= n_pids; i++){
        printf("%d. PID: %d\t Line: %s\t Status: %c\n",
         i, jobs_list[i].pid, jobs_list[i].command_line,
          jobs_list[i].status);
    }
    return 1;
}

int internal_fg(char **args){
    signal(SIGTSTP, ctrlz);
    int pos;
    //cogemos el string y lo pasamos a un integer
    if(args[1] != NULL){
        sscanf(args[1], "%d", &pos);
        if((pos > n_pids) || (pos == 0)){
            fprintf(stderr, "No existe ese trabajo\n");
        }else{
            if(jobs_list[pos].status == 'D'){
                kill(jobs_list[pos].pid, SIGCONT);
                fprintf(stderr, "Continuacion del trabajo con PID: %d(%s)\n",
                 jobs_list[pos].pid, jobs_list[pos].command_line);
            }
            //ponemos el trabajo que pasa a foreground en la posicion 0
            jobs_list[0].pid = jobs_list[pos].pid;
            jobs_list[0].status = jobs_list[pos].status;
            strcpy(jobs_list[0].command_line, quita_up_and(jobs_list[pos].command_line));
            //eliminar trabajo de la lista de trabajos
            jobs_list_remove(pos);
            fprintf(stderr, "Command_line: %s\n", jobs_list[0].command_line);
            signal(SIGTSTP, ctrlz);
            //mientras haya un trabajo en foreground espera
            while((jobs_list[0].pid  != 0)){
                pause();
            }
        }
    }else{
        fprintf(stderr, "fg: argumentos invalidos\n");
        fprintf(stderr, "fg: uso: fg PIDdetrabajo\n");
    }
    return 1;
}

int internal_bg(char **args){
    int pos;
    if(args[1] != NULL){
        //cogemos el string y lo pasamos a un integer
        sscanf(args[1], "%d", &pos);
        if((pos > n_pids) || (pos == 0)){
            fprintf(stderr, "bg: %d: no existe ese trabajo\n", pos);
        }else{
            if(jobs_list[pos].status == 'E'){
                fprintf(stderr, "bg: el trabajo %d ya esta ejecutandose en background\n", pos);
            }else{
                kill(jobs_list[pos].pid, SIGCONT);
                jobs_list[pos].status = 'E';
                strcpy(jobs_list[pos].command_line, poner_up_and(jobs_list[pos].command_line));
                printf("%d. PID: %d\t Line: %s\t Status: %c\n",
                 pos, jobs_list[pos].pid, jobs_list[pos].command_line, 
                 jobs_list[pos].status);
            }
        }
    }else{
        fprintf(stderr, "bg: argumentos invalidos\n");
        fprintf(stderr, "bg: uso: bg PIDdetrabajo\n");
    }
    return 1;
}

//El reaper se encarga de tratar a los hijos que ya han acabado
void reaper(int signum){
    signal(SIGCHLD, reaper);
    pid_t ended;
    int endedPos;
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        if(ended == jobs_list[0].pid){
            fprintf(stderr, "\n[reaper()→ Proceso en foreground finalizado]\n");
            jobs_list[0].pid = 0;
            strcpy(cmdline, jobs_list[0].command_line);
            jobs_list[0].status = 'F';
            fprintf(stderr, "[reaper()→ Enterrado proceso con PID %d(%s)]\n", ended, jobs_list[0].command_line);
            memset(jobs_list[0].command_line, 0, COMMAND_LINE_SIZE);
        }else{
            fprintf(stderr, "\n[reaper()→ Proceso en background finalizado]\n");
            endedPos = jobs_list_find(ended);
            //strcpy(cmdline, jobs_list[endedPos].command_line);
            fprintf(stderr, "[reaper()→ Enterrado proceso con PID %d(%s)]\n", ended, jobs_list[endedPos].command_line);
            jobs_list_remove(endedPos);
        }
        
    }  
}

void ctrlc(){
    signal(SIGINT, ctrlc);
    pid_t pid = getpid();
    printf("\n[ctrlc()→ Soy el proceso con PID %d, el proceso en foreground es %d(%s)]\n",
     pid, jobs_list[0].pid, jobs_list[0].command_line);
    if(jobs_list[0].pid > 0){
        printf("\n");
        if(strcmp(jobs_list[0].command_line, padre->command_line)){
            if (kill(jobs_list[0].pid, SIGTERM)==0) { // Enviamos la señal SIGINT al proceso
                printf("[ctrlc()→ Señal enviada a %d(%s)\n",
                 jobs_list[0].pid, jobs_list[0].command_line);
            }else{
                perror("kill\n");
                exit(-1);
            }
        }else{
            fprintf(stderr, "[ctrlc()→ Señal 15 no enviada debido a que el proceso en foreground es el shell]\n");
        }
    }else{
        fprintf(stderr, "[ctrlc()→ Señal 15 no enviada por %d debido a que no hay proceso en foreground]\n", pid);
        #ifdef USE_READLINE
        printf("%s", imprimir_prompt());
        #else
        printf("\n");
        #endif
    }
    fflush(stdout);
}

//devuelve 1 si el ultimo caracter es '&' indicando que se trata de un 
//comando en background
int is_background(char *line){
    int n = 0;
    while(line[n] != '\n'){
        n++;
    }
    line[n] = '\0';
    strcpy(current_command_line, line);
    if(line[n-1] == '&'){
        line[n-1] = '\n';
        return 1;
    }
    return 0;
}

//controlador de la señal SIGSTSP
void ctrlz(){
    signal(SIGTSTP, ctrlz);
    if(jobs_list[0].pid > 0){
        printf("\n");
        if(strcmp(jobs_list[0].command_line, padre->command_line)){
            kill(jobs_list[0].pid, SIGSTOP);
            fprintf(stderr, "\n[ctrlz()→ Señal SIGSTOP enviada a PID: %d(%s)\n",
             jobs_list[0].pid, jobs_list[0].command_line);
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].command_line);
            jobs_list[0].pid = 0;
            memset(jobs_list[0].command_line, 0, COMMAND_LINE_SIZE);
            jobs_list[0].status = 'F';
        }else{
            fprintf(stderr, "\n[ctrlz()→ Señal SIGSTOP no enviada debido a que el proceso en foreground es el shell]\n");
        }
    }else{
        fprintf(stderr, "\n[ctrlz()→ Señal SIGSTOP no enviada debido a que no hay proceso en foreground]\n");
        #ifdef USE_READLINE
        printf("%s", imprimir_prompt());
        #else
        printf("\n");
        #endif
    }
    signal(SIGTSTP, ctrlz);
    fflush(stdout);
}

//añade un trabajo a la lista de trabajos
int jobs_list_add(pid_t pid, char status, char *command_line){
    if(n_pids < NJOBS){
        n_pids++;
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        strcpy(jobs_list[n_pids].command_line, command_line);
    }else{
        fprintf(stderr, "Numero maximo de trabajos en background\n");
    }
    return 1;
}

//encuentra y devuelve la posicion de un trabajo de la lista de trabajos
int jobs_list_find(pid_t pid){
    int encontrado = 0;
    int pos = 0;
    for(int i = 1; (i <= n_pids) && !encontrado; i++){
        if(jobs_list[i].pid == pid){
            encontrado = 1;
            pos = i;
        }
    }
    return pos;
}

//elimina un trabajo de la lista de trabajos
int  jobs_list_remove(int pos){
    if(pos != n_pids){
        jobs_list[pos] = jobs_list[n_pids];
    }else{
        jobs_list[pos].pid = 0;
        jobs_list[pos].status = 'F';
        memset(jobs_list[pos].command_line, 0, COMMAND_LINE_SIZE);
    }
    n_pids--;
    return 1;
}

//devuelve 1 si ha de redireccionar a un fichero
int is_output_redirection(char **args){
    for(int i = 0; args[i] != NULL; i++){
        if(strcmp(args[i], ">") == 0){
            args[i] = NULL;
            int fd = open(args[i+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            dup2(fd, 1);
            close(fd);
            return 1;
        }
    }
    return 0;
}

int main(){
    n_pids = 0;
    current_command_line = malloc(COMMAND_LINE_SIZE);
    cmdline = malloc(COMMAND_LINE_SIZE);
    padre = malloc(sizeof(struct info_process));
    strcpy(padre->command_line, "./my_shell");
    padre->status = 'E';
    padre->pid = getpid();
    signal(SIGCHLD, reaper); 
    signal(SIGINT, ctrlc);
    signal(SIGTSTP, ctrlz);
    char *line = malloc(COMMAND_LINE_SIZE);
    while(read_line(line)){
        if(strlen(line) > 0){
            execute_line(line);
        }
        int sizeline = sizeof(line);
        memset(line, 0, sizeline);
    }
    return 0;
}