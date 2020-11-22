#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>  // strtok
#include <unistd.h>  // chdir

#define PROMPT "$"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64

#define VERDE "\x1b[32m"
#define AZUL "\x1b[34m"
#define BLANCO "\x1b[37m"

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
    char pdir[COMMAND_LINE_SIZE];
    if(args[1] != NULL){
        //Check the directory
        
        strcpy(pdir, args[1]); 
        for (int m = 0; m < 3; m++){
            if(strchr(pdir, advanced_cd[m]) != NULL){ // Using advanced cd 
                //En todos los casos, tenemos que eliminar \, " o ' . Por lo tanto, puede seguir un procedimiento genérico
                int l = -1; //pdir counter
                for(int i = 1; args[i]!= NULL; i++){
                    l++; //Sumamos aqui ya que, en el caso que solo tengamos un argumento, queremos reemplazar ' ' por '\0'
                    for(int j = 0; args[i][j] != '\0'; j++){
                        
                        if(args[i][j] != advanced_cd[m]){
                            pdir[l]=args[i][j];
                            l++;
                        }
                    }
                    pdir[l] = ' ' ; //Añadimos el espacio
                }
                pdir[l] = '\0';
                break;
            }
        }
        
        if(chdir(pdir) < 0){
            perror("chdir() Error: ");
        }
    }else{
        if(chdir(getenv("HOME")) < 0 ){
            perror("chdir() Error: ");
        } //Go to home
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
    
    if(args[2] != NULL){
        perror("Invalid syntax");
        return -1;
    }
    args[1] = strtok(args[1], "=");
    
    printf("args[1]: %s\n", args[1]);
    char *env = getenv(args[1]);
    if(!env){
        perror("Not a valid variable");
        return -1;
    }
    args[2] = strtok(NULL, "=");
    printf("args[2]: %s\n", args[2]);
    setenv(env,args[2], 1);
    printf("%s = %s",args[1],args[2]);
}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla)
*/
int internal_source(char **args){

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
