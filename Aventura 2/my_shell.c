#include <stdio.h>
#include <stdlib.h>

#define YEET 0;
#define PROMPT '$';
#define COMMAND_LINE_SIZE 1024;
#define ARGS_SIZE 64;

int main(){
    
    char line[COMMAND_LINE_SIZE];
    while(1){

    }
    
    return YEET;
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
    char string[] = fgets(line, 20, nuestra terminak ostiabrooooo);
}
/*
De momento sólo llama a parse_args() para obtener la linea fragmentada en tokens y le pasa 
los tokens a la función booleana check_internal() para determinar si se trata de un comando 
interno. 
*/
int execute_line(char *line){

}

/*
Trocea la línea obtenida en tokens, mediante la función strtok(), y obtiene el vector de 
los diferentes tokens, args[]. No se han de tener en cuenta los comentarios (precedidos por #). 
El último token ha de ser NULL. 
En este nivel, muestra por pantalla el número de token y su valor para comprobar su correcto 
funcionamiento  (en fases posteriores eliminarlo).
Devuelve el número de tokens (sin contar NULL).
*/

int parse_args(char **args, char *line){

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

}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_cd(char **args){

}

/*
En este nivel, imprime una explicación de que hará esta función (en fases posteriores eliminarla).
*/
int internal_export(char **args){

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