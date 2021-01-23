/*
Alberto Cugat Martín
Jaume Julià Vallespir
Rubén Palmer Pérez
*/

#include "my_lib.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10
#define mutex = PTHREAD_MUTEX_INITIALIZER;
int pila;

char *checkAvailability(filename){
    //se coge por consola el nombre del archivo i se guarda en filename (PENDIENTE)
    pila = open(filename, O_RDONLY , S_IRUSR); //supongo que si el archivo no 
    
    if(pila < 0){
        close(pila);
        return NULL;
    }
    return filename;
}

int main(){         //Parece ser que hay que hacere la funcion main como int main(int argc, char *argv[])
    struct my_stack *pila;
    FILE *filepointer;
    char filename;
    int len = 0;
    
    if(fgets(filename, 200, filepointer) == NULL){  
        fprintf(stderr,"Failed to read the filename");                                                 // Non EOF command
        exit(EXIT_FAILURE);
    }
    if((filename = checkAvailability(filename)) == NULL){ //la pila no existe
        pila = my_stack_init(malloc(sizeof(int))); //cogemos el puntero del objeto pila
        filepointer = open(filename,  O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); //aqui queremos crear el archivo (PENDIENTE)
        len = 10;
        for(int i = 0; i < len; i++){ //llenamos las posiciones necesarias de la pila con 0s (si la pila existe no hay que machcarla sino usarla)
        my_stack_push(pila, 0);
    }
    }else{ //la pila si existe
        pila = my_stack_read(filename); //aqui leemos la pila del archivo y guardamos el puntero del objeto
        len = my_stack_len(pila); //cogemos el tamaño de la pila
        len = len - 10; //tenemos que asegurarnos que haya al menos 10 elementos
        if (len < 0){
            len = 0;
        }      
    }

    pthread_t arrayThreads [10];

    for(int i = 0; i < NUM_THREADS; i++){
       pthread_create(arrayThreads[i],NULL,worker,NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(arrayThreads[i],NULL);
    }
    
}

void *worker(void *ptr){

}