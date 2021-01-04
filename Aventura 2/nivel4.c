/******************************************************************/
/*                  Alberto Cugat Martín                          */
/*                  Jaume Julià Vallespir                         */
/*                  Rubén Palmer Pérez                            */
/******************************************************************/
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h> 
#include <signal.h>     
#include <sys/wait.h>   
#include <sys/types.h>  
#include <string.h>     
#include <unistd.h>     

#define PROMPT "$"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define minishell "./my_shell"

#define VERDE "\x1b[32m"
#define AZUL "\x1b[34m"
#define BLANCO "\x1b[37m"

#define N_JOBS 64 

const char Separadores[5] = " \t\n\r";
const char advanced_cd[4] = "\\\"\'";

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
void ctrlc(int signum);

int advanced_syntax(char* line);
                                                                            
struct info_process {                                                       // Objeto hilos/procesos
    pid_t pid;
    char status;
    char cmd[COMMAND_LINE_SIZE];
};

static struct info_process jobs_list[N_JOBS];                               // Array de procesos. El 0 es foreground

int main(){
    
    char line[COMMAND_LINE_SIZE];
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    struct info_process *jobs_list = malloc(N_JOBS*sizeof(struct info_process));
    for (int i = 0; i < N_JOBS ; i++){
        struct info_process newJob = {.pid = 0, .status = 'N'};

        jobs_list[i] = newJob;
        
    }

    memset(line, 0, sizeof(line));
    while(read_line(line)){
        execute_line(line);
        memset(line, 0, sizeof(line));
    }

    return -1;
}


/*
    Lo más simple es usar un símbolo como constante simbólica, por ejemplo: #define PROMPT ‘$’, 
    o un char const PROMPT =’$’. A la hora de imprimirlo será de tipo carácter, %c, y le podéis 
    añadir un espacio en blanco para separar la línea de comandos. 
    
    Opcionalmente se puede implementar una función auxiliar, imprimir_prompt(), para crear un 
    prompt personalizado tipo string, yuxtaponiendo variables de entorno como USER, HOME o PWD.
    El valor de estas variables se puede obtener con la función getenv() y también se pueden
    usar colores. El directorio actual también se puede obtener con la función getcwd().   
    
    Para forzar el vaciado del buffer de salida se puede utilizar la función fflush(stdout)).
    
    Imprime el prompt.
    Lee una linea de la consola (stdin) con la función fgets().
    Devuelve un puntero a la línea leída. 
*/

char *read_line(char *line){
    printf(VERDE"%s"BLANCO":"AZUL"%s"BLANCO"%s ", getenv("USERNAME"), getenv("PWD"), PROMPT);
    fgets(line, COMMAND_LINE_SIZE, stdin);
    if(feof(stdin) == 0){                                                   // Non EOF command
        return line;
    }else{
        printf("🤠 See you, space cowboy 🤠\n");
        exit(1);
    }
    
}

/*
    De momento sólo llama a parse_args() para obtener la linea fragmentada en tokens y le pasa 
    los tokens a la función booleana check_internal() para determinar si se trata de un comando 
    interno. 
*/

int execute_line(char *line){
    char *tokens[ARGS_SIZE];
    int len = strlen(line);
    if (len > 0 && line[len-1] == '\n'){
        line[len-1] = '\0';
    }

    strcpy(jobs_list[0].cmd, line);
    pid_t pid;
    if(parse_args(tokens, line) != 0){                                      // Si tenemos argumentos en nuestro comando
        printf("execute_line()-> PID padre: %d (%s)\n", getpid(), minishell);  
        if(check_internal(tokens) == 0){                                    // Identifica si es un comando interno o externo
            signal(SIGCHLD, reaper);           
            pid = fork();
            jobs_list[0].status = 'E';                                      // Proceso en ejecución
            if(pid == 0){ //proceso hijo
                jobs_list[0].pid = getpid();
                printf("execute_line()-> PID hijo: %d (%s)\n", jobs_list[0].pid, jobs_list[0].cmd);
                signal(SIGCHLD, SIG_DFL);                                   // El hijo no tiene que manejar el reaper, asi que delega la responsabilidad
                signal(SIGINT, SIG_IGN);                                    // El hijo ignora esta señal
                if(execvp(tokens[0], tokens) == -1){
                    fprintf(stderr, "Command %s not found", tokens[0]);
                    exit(0);
                }
                exit(0);
                
            }else if(pid > 0){                                              // Proceso padre
                jobs_list[0].pid = pid;
                while ((jobs_list[0].pid != 0)){
                    pause();
                }
                
            }                  
        }
    }

    return 0;
}

/*
    Manejador de señales que va a recuperar el estado del proceso hijo que se ha detenido y lo trata para
    evitar el estado zombie.
*/
void reaper(int signum){
    signal(SIGCHLD, reaper);                                                // Para refrescar la acción apropiada
    pid_t terminatedProcess;

    while((terminatedProcess = waitpid(-1, NULL, WNOHANG)) > 0){            // Busca todos los hijos que puedan haber terminado a la vez y los cierra | NUll == childsatus
        if (jobs_list[0].pid == terminatedProcess){
            printf("reaper() -> proceso hijo %d (%s) finalizado por la señal %d", terminatedProcess, jobs_list[0].cmd, signum);
            jobs_list[0].pid = 0;                                           // Desbloqueamos al minishell eliminando el pid del proceso en foreground
            jobs_list[0].status = 'F';                                      // Proceso finalizado
            memset(jobs_list[0].cmd, 0, COMMAND_LINE_SIZE);                 // Se borra el cmd asociado
        }
        printf("Ha terminado el proceso hijo con pid %d \n", terminatedProcess);
    }    
}

void ctrlc (int signum){
    pid_t pid = getpid();
    printf("\n");
    printf("ctrlc() -> Soy el proceso con PID %d, el proceso en foreground es %d (%s)\n", pid, jobs_list[0].pid , jobs_list[0].cmd);
    if(jobs_list[0].pid > 0){                                               // Hay proceso en foreground 
        if(strcmp(jobs_list[0].cmd, minishell)){                            // Proceso en foreground es un minishell
            printf("ctrlc() -> Señal %d no enviada debido a que el proceso en foreground es un minishell\n", signum);
        }else{
            printf("ctrlc() -> Señal %d enviada a %d (%s)\n", signum, jobs_list[0].pid, jobs_list[0].cmd);
            kill(jobs_list[0].pid, SIGTERM);                                // Esto aborta el proceso en foreground
        }
    }else{
        printf("ctrlc() -> Señal %d no enviada debido a que no hay proceso en foreground\n", signum );
    }
    
    signal(SIGINT, ctrlc);                                                  // Hay que refrescar
}
/*
    Trocea la línea obtenida en tokens, mediante la función strtok(), y obtiene el vector de 
    los diferentes tokens, args[]. No se han de tener en cuenta los comentarios (precedidos por #). 
    El último token ha de ser NULL. 
    En este nivel, muestra por pantalla el número de token y su valor para comprobar su correcto 
    funcionamiento  (en fases posteriores eliminarlo).
    Devuelve el número de tokens (sin contar NULL).
    
    Consideraremos los siguientes separadores: \t \n \r y espacio en blanco (todos en una misma cadena de
    delimitadores yuxtapuestos: “ \t\n\r”)
*/

int parse_args(char **args, char *line){
    int tokens = 0;
    
    args[tokens] = strtok(line, "#");                                       // Eliminamos los comentarios
    args[tokens] = strtok(args[tokens], Separadores);                       // Cogemos el primer argumento
    
    printf("Token %d: %s\n", tokens, args[tokens]);
    while (args[tokens] != NULL){ 
        tokens++;
        args[tokens] = strtok(NULL, Separadores);                           // Leer la siguiente palabra
        printf("Token %d: %s\n", tokens, args[tokens]); 
    }
        args[tokens]=NULL;
        return tokens;
}

/*
    Es una función booleana que averigua si args[0] se trata de un comando interno, mediante la 
    función strcmp(), y llama a la función correspondiente para tratarlo (internal_cd(), 
    internal_export(), internal_source(), internal_jobs(), internal_fg(), internal_bg()).
    En el caso del comando interno exit podéis ya llamar directamente a la función exit().
    La función devuelve 0 o FALSE si no se trata de un comando interno o la llamada a la 
    función correspondiente, cada una de las cuales a su vez devolverá un 1 o TRUE para 
    indicar que se ha ejecutado un comando interno.
*/

int check_internal(char **args){
    int internal = 1;
    if (strcmp(args[0], "cd") == 0){
        internal_cd(args);

    }else if (strcmp(args[0], "export") == 0){
        internal_export(args);

    }else if (strcmp(args[0], "source") == 0){
        internal_source(args);

    }else if (strcmp(args[0], "jobs") == 0){
        internal_jobs(args);

    }else if (strcmp(args[0], "fg") == 0){
        internal_fg(args);

    }else if (strcmp(args[0], "bg") == 0){
        internal_bg(args);

    }else if (strcmp(args[0], "exit") == 0){
        exit(0);        
    }else{                                                                  // No hemos encontrado ningun comando interno
        internal = 0;
    }

    return internal;
}

/*
    Utiliza la llamada al sistema ​chdir​() para cambiar de directorio.En este nivel,
    a modo de test, muestra por pantalla el directorio al que nos hemos trasladado. 
    Para ello usa la llamada al sistema ​getcwd​() (en niveles posteriores eliminarlo).

    Si queréis que se os actualice el prompt al cambiar de directorio, podéis cambiar 
    el valor de la variable de entorno PWD mediante ​setenv​() y utilizarla después en 
    la función imprimir_prompt(), aunque también podéis usar ​getcwd​() en vez de PWD 
    para imprimir el prompt. El comando "cd" sin argumentos ha de enviar al valor de 
    la variable HOME. Adicionalmente se puede implementar el ​cd avanzado.
    
    En ese caso en la sintaxis tendremos que admitir más de 2 elementos. 
    Podéis emplear la función strchr​() para determinar si el token guardado 
    en args[1] contiene comillas simples o dobles, o el caràcter \
*/
int internal_cd(char **args){
    char *pdir;
    pdir = malloc(COMMAND_LINE_SIZE);
    if(!pdir){
        fprintf(stderr, "Not enough space");
        return -1;
    }
    
    if(args[1] != NULL){                                                    // Tenemos un segundo argumento ?

        for(int i = 1; args[i]!=NULL; i++){                                 // Crear la linea con todos los argumentos
            strcat(pdir, args[i]);
            if(args[i+1]!=NULL){
                strcat(pdir, " ");
            }

        }

        advanced_syntax(pdir);                                              // Remove advanced_cd values
       
        if(chdir(pdir) < 0){
            perror("chdir() Error: ");
            strcpy(pdir, "\0");                                             // Preparacion para el siguiente pdir
            free(pdir);
            return -1;
        }
        strcpy(pdir, "\0");                                                 // Preparacion para el siguiente pdir
        free(pdir);
    
    }else if(chdir(getenv("HOME")) < 0 ){                                   // Directorio home
        perror("chdir() Error: ");
    } 

                                                                            // Actualiza PWD
    char cwd[COMMAND_LINE_SIZE];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("getcwd() Error: ");
    }else{
        setenv("PWD", cwd, 1);
    }

    return 0; 
}

/*
    Descompone en tokens el argumento NOMBRE=VALOR (almacenado en args[1]),
    por un lado el nombre y por otro el valor.Notifica de la sintaxis correcta
    si los argumentos no son los adecuados, utilizando la salida estándar de 
    errores ​stderr​.En este nivel, muestra por pantalla mediante la función ​getenv​()
    el ​valor inicial​ dela variable (en niveles posteriores eliminarlo).
    Utiliza la función ​setenv​() para asignar el nuevo valor.
    
    En este nivel, muestra por pantalla el ​nuevo valor​ mediante la función ​getenv​()
    para comprobar su funcionamiento (en niveles posteriores eliminarlo). 
*/
int internal_export(char **args){
    if(args[2] != NULL){                                                    // Revisa syntax
        fprintf(stderr, "Invalid syntax [NAME=VALUE]\n");
        return -1;
    }
    args[1] = strtok(args[1], "=");                                         // Variable de entorno
    
    if(!getenv(args[1])){ 
        fprintf(stderr, "Not a valid enviroment variable\n");
        return -1;
    }
    args[2] = strtok(NULL, "=");                                            // Valor de la variable de entorno
    setenv(args[1],args[2], 1);
    return 0;
}

/*
    Se comprueban los argumentos y se muestra la sintaxis en caso de no sercorrecta.Mediante la función ​fopen​() se abre en modo lectura el fichero de comandos3especificado por consola.Se indica error si el fichero no existe.Se va leyendo línea a línea el fichero mediante ​fgets​() y se pasa la línea leída anuestra función execute_line(). Hay que realizar un ​fflush​ del stream del ficherotras leer cada línea.Se cierra el fichero de comandos con ​fclose​()
*/
int internal_source(char **args){

    if(args[1] != NULL){
        FILE *file_p;  
        
        file_p = fopen(args[1], "r");                                       // Abrir archivos args[1] en solo lectura
        if(file_p != NULL){
                                                                            // Leer linea a linea el archivo

            char buffer[COMMAND_LINE_SIZE];
            while(fgets(buffer, COMMAND_LINE_SIZE,file_p)!= NULL){          // Existe una linea 
                execute_line(buffer);
                if(fflush(file_p)!= 0){
                    fprintf(stderr, "Error while flushing [%s]", args[1]);
                    return -1;
                }
            }
            if(fclose(file_p) != 0){
                fprintf(stderr, "Error while closing file [%s]", args[1]);
                return -1;
            }
        }else{
            fprintf(stderr, "%s: No such file or directory\n", args[1]); 
            return -1;
        }  
    }else{                                                                  // Syntax incorrecta
        printf("source requieres an additional parameter");
    }
    
    return 0;
}

/*
    En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla)
*/
int internal_jobs(char **args){
    printf("This is internal_jobs\n The jobs command in Linux allows the user to directly interact with processes in the current shell.\n");
    return 0;
}

/*
    En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_fg(char **args){
    printf("This is internal_fg\n continues a stopped job by running it in the foreground\n");
    return 0;
}

/*
    En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_bg(char **args){
    printf("This is internal_bg\n t resumes suspended jobs in the background\n");
    return 0;
}

/*
    USER METHODS
*/

/*
    Pasada una linea por argumento devuelve, en esa miesma linea, su valor sin los caracteres de advanced_cd. Returns 1 si ha hehco una conversion y 0 si no ha hehco nada
*/
int advanced_syntax(char *line){
    char return_line[COMMAND_LINE_SIZE];
    int return_line_index = 0;
    int found;
    int conversion = 0;
    
    for(int i = 0; line[i] != '\0'; i++){                                   // Recorrido de la linea
    found = 0;
        for(int j = 0; j < 3 &&(found == 0); j++){                          // Recorrido de advanced_cd
            if (line[i]==advanced_cd[j]){
                found = 1;                                                  // Encontrado un valor de advanced_cd
                conversion = 1;                                             // Se pondra varias veces, pero es mejor que hacer un if todo el rato
            }
        } 
        if(found == 0){                                                     // Si no hemos encontrado un valor de advanced_cd, copiamos la linea
            return_line[return_line_index] = line[i]; 
            return_line_index++;
        }

    }
    return_line[return_line_index] = '\0';                                  // Terminacion del return_line
    
    strcpy(line, return_line);                                              // Pasamos la linea modificada 
    return conversion;
}
