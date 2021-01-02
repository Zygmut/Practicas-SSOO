#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROMPT "$"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64

#define VERDE "\x1b[32m"
#define AZUL "\x1b[34m"
#define BLANCO "\x1b[37m"

const char Separadores[5] = " \t\n\r";

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

int main(){
    
    char line[COMMAND_LINE_SIZE];
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
    return line;
}

/*
De momento sólo llama a parse_args() para obtener la linea fragmentada en tokens y le pasa 
los tokens a la función booleana check_internal() para determinar si se trata de un comando 
interno. 
*/

int execute_line(char *line){
    char *tokens[ARGS_SIZE];
    
    if(parse_args(tokens, line) != 0){ //Si tenemos argumentos en nuestro comando
        check_internal(tokens);
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
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_cd(char **args){
    printf("This is internal_cd\n Change the current working directory\n");
}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_export(char **args){
    printf("This is internal_export\n marks an environment variable to be exported with any newly forked child processes and thus it allows a child process to inherit all marked variables\n");
}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla)
*/
int internal_source(char **args){
    printf("This is internal_source\n  import functions into other bash scripts or to run scripts\n");
}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla)
*/
int internal_jobs(char **args){
    printf("This is internal_jobs\n The jobs command in Linux allows the user to directly interact with processes in the current shell.\n");
}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_fg(char **args){
    printf("This is internal_fg\n continues a stopped job by running it in the foreground\n");
}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_bg(char **args){
    printf("This is internal_bg\n t resumes suspended jobs in the background\n");
}
