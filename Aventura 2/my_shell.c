#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h> 
#include <sys/wait.h>   //para la funcion wait
#include <sys/types.h>  //por si acaso
#include <string.h>     // strtok
#include <unistd.h>     // chdir
#include <signal.h>     // Libreria de mierda que maneja señales  

#define PROMPT "$"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64

#define VERDE "\x1b[32m"
#define AZUL "\x1b[34m"
#define BLANCO "\x1b[37m"

#define N_JOBS 1

const char Separadores[5] = " \t\n\r";
const char advanced_cd[4] = "\\\"\'";

// Estructura para el estado de un proceso
struct info_process {
    pid_t pid;
    char status;    // Ninguno, Ejecutandose, Detenido, Finalizado [N,E,D,F]
    char cmd[COMMAND_LINE_SIZE]     // Comando que esta siendo ejecutado
};

static struct info_process *father;
static struct info_process jobs_list[N_JOBS];  // Donde SON_COUNT es la cantidad de procesos hijos que permitimos tener

char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int reaper(int signal);

int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);

int advanced_syntax(char* line);

int main(){
    
    char line[COMMAND_LINE_SIZE];
    int status;
    while(read_line(line)){
        execute_line(line);
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
    if(feof(stdin) == 0){ // Non EOF command
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
    int status; // No se para que sirve, pero asi el wait funciona 
    
    if(parse_args(tokens, line) != 0){ //Si tenemos argumentos en nuestro comando
        
        if(check_internal(tokens) == 0){ //identifica si es un comando interno o externo
            signal(SIGCHLD, reaper);     // Llevo como 20 minutos y me acabo de dar cuenta que esto es un action listener. fuck off 
            pid = fork();
            strcpy(sons[0].cmd, line);  // Copiar el comando a la cola de procesos
            sons[0].status = "E";   // Proceso hijo esta siendo ejecutado
            
            if(pid == 0){ //proceso hijo
                jobs_list[0].pid = getpid();    // Obtenemos el pid y lo guardamos en la cola de procesos 
                printf("son\n");
                printf("EQUISDE\n");
                if(execvp(tokens[0], tokens) == -1){
                    fprintf(stderr, "Command %s not found", tokens[0]);
                }
                exit(0);
            }else if(pid > 0){  //proceso padre
                jobs_list[0].pid = pid;     // Coger el pid del hijo, en el caso que exista, claro
                //Wait to dodge the zombie apocalipse (zombie child)
                wait(&status); // Ponga ocmo lo ponga esta mierda peta. Me duele el cuerpo, too bad!
                printf("father\n");
            }else{ //proceso aborto capoeira da morte radioactiva full petao
                perror("fork");                
            }

            
        }
    }
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
    
    args[tokens] = strtok(line, "#"); //Eliminamos los comentarios
    args[tokens] = strtok(args[tokens], Separadores); // Cogemos el primer argumento
    
    printf("Token %d: %s\n", tokens, args[tokens]);
    while (args[tokens] != NULL){ 
        tokens++;
        args[tokens] = strtok(NULL, Separadores); //leer la siguiente palabra
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
    }else{ //No hemos encontrado ningun comando interno
        internal = 0;
    }

    return internal;
}

/*
    Desecha los cadaveres de los hijos 
*/
void reaper(int signal){
    signal(SIGCHLD, reaper);   // Lo dice adelaida. Wtf
    pid_t ended;    // Necesitamos esta variable para el while extraño 

    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {  // Pero que 
        if(ended == jobs_list[0].pid){  // Ha acabado el proceso en fore
            printf("Foreground process finished\n");
            jobs_list[0].pid = 0;       // PID a 0
            memset(jobs_list[0].cmd, 0, 1);    // Pone un 0 en la primera posicion del string, "eliminandolo"
            jobs_list[0].status = "F";
        }
        printf("Reaper just killed %d!!!\n", ended);
    }  
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
        fprintf(stderr,"Not enough space");
        return -1;
    }
    
    if(args[1] != NULL){ //Do we have a second argument?

        for(int i = 1; args[i]!=NULL; i++){ //create the line of all the arguments
            strcat(pdir, args[i]);
            if(args[i+1]!=NULL){
                strcat(pdir, " ");
            }

        }

        advanced_syntax(pdir); //Remove advanced_cd values
       
        if(chdir(pdir) < 0){
            perror("chdir() Error: ");
           //SI DA ERROR PDIR PETA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
             
            strcpy(pdir, "\0"); //Preparacion para el siguiente pdir
            free(pdir);
            return -1;
        }
        strcpy(pdir, "\0"); //Preparacion para el siguiente pdir
        free(pdir);
    
    }else if(chdir(getenv("HOME")) < 0 ){ //Go homo
        perror("chdir() Error: ");
    } 

    //Update PWD
    char cwd[COMMAND_LINE_SIZE];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("getcwd() Error: ");
    }else{
        setenv("PWD", cwd, 1);
    }
    
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
    if(args[2] != NULL){ //only export
        fprintf(stderr, "Invalid syntax [NAME=VALUE]\n");
        return -1;
    }
    args[1] = strtok(args[1], "="); //Variable de entorno
    
    if(!getenv(args[1])){ 
        fprintf(stderr, "Not a valid enviroment variable\n");
        return -1;
    }
    args[2] = strtok(NULL, "="); //Valor de la variable de entorno
    setenv(args[1],args[2], 1);
}

/*
Se comprueban los argumentos y se muestra la sintaxis en caso de no sercorrecta.Mediante la función ​fopen​() se abre en modo lectura el fichero de comandos3especificado por consola.Se indica error si el fichero no existe.Se va leyendo línea a línea el fichero mediante ​fgets​() y se pasa la línea leída anuestra función execute_line(). Hay que realizar un ​fflush​ del stream del ficherotras leer cada línea.Se cierra el fichero de comandos con ​fclose​()
*/
int internal_source(char **args){
    

    if(args[1] != NULL){
        FILE *file_p;  //File pointer

        file_p = fopen(args[1], "r"); //Open file with name args[1] on only read
        if(file_p != NULL){
            // Leer linea a linea el archivo

            char buffer[COMMAND_LINE_SIZE];
            while(fgets(buffer, COMMAND_LINE_SIZE,file_p)!= NULL){ //existe una linea 
                execute_line(buffer);
                if(fflush(file_p)!= 0){
                    fprintf(stderr, "Error while flushing [%s]", args[1]);
                }
            }
            if(fclose(file_p) != 0){
                fprintf(stderr, "Error while closing file [%s]", args[1]);
            }
        }else{
            fprintf(stderr, "%s: No such file or directory\n", args[1]); 
        }  
    }else{ //syntax incorrecta
        printf("source requieres an additional parameter");
    }
}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla)
*/
int internal_jobs(char **args){

}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_fg(char **args){

}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_bg(char **args){
    
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
    
    for(int i = 0; line[i] != '\0'; i++){ //recorrido de la linea
    found = 0;
        for(int j = 0; j < 3 &&(found == 0); j++){ //recorrido de advanced_cd
            if (line[i]==advanced_cd[j]){
                found = 1; //encontrado un valor de advanced_cd
                conversion = 1; //Se pondra varias veces, pero es mejor que hacer un if todo el rato
            }
        } 
        if(found == 0){ //Si no hemos encontrado un valor de advanced_cd, copiamos la linea
            return_line[return_line_index] = line[i]; 
            return_line_index++;
        }

    }
    return_line[return_line_index] = '\0'; //Terminacion del return_line
    
    strcpy(line, return_line); //Pasamos la linea modificada 
    return conversion;
}
