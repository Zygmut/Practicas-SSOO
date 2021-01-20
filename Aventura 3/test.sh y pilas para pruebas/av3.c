/*
Alberto Cugat Martín
Jaume Julià Vallespir
Rubén Palmer Pérez
*/

#include "my_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

char *checkAvailability(){
    //se coge por consola el nombre del archivo i se guarda en filename (PENDIENTE)
    int pila = open(filename, O_RDONLY , S_IRUSR); //supongo que si el archivo no 
    
    if(pila < 0){
        close(pila);
        return NULL;
    }
    return filename 
}

int main(){
    struct my_stack *pila;
    char filename;
    int len = 0;
    if((filename = checkAvailability()) == NULL){ //la pila no existe
        pila = my_stack_init(malloc(sizeof(int))); //cogemos el puntero del objeto pila
        int fichero = open(filename, ); //aqui queremos crear el archivo (PENDIENTE)
        len = 10;
    }else{ //la pila si existe
        pila = my_stack_read(filename); //aqui leemos la pila del archivo y guardamos el puntero del objeto
        len = my_stack_len(pila); //cogemos el tamaño de la pila
        len = len - 10; //tenemos que asegurarnos que haya al menos 10 elementos
        if (len < 0){
            len = 0;
        }      
    }
    for(int i = 0; i < len; i++){ //llenamos las posiciones necesarias de la pila con 0s
        my_stack_push(pila, 0);
    }
}