/*
Alberto Cugat Martín
Jaume Julià Vallespir
Rubén Palmer Pérez
*/

#include "my_lib.c"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10
#define ITERATIONS 5

//ricos colorines Bacanos
#define blanco  "\033[0;37m"
#define rojo    "\033[0;31m"
#define azul    "\033[0;34m"
#define negro   "\033[0;30m"//Usar en caso de tener la terminal en blanco (te queremos Adelaida, lo hacemos por ti)


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct my_stack *pila; //apuntador al objeto pila

void *worker(void *ptr);
int checkAvailability(int argc, char *filename[]);


int main(int argc, char *argv[]){         //Parece ser que hay que hacere la funcion main como int main(int argc, char *argv[])
    
    char *filename; //nombre del archivo de la pila
    int len = 0; //usada para rellenar las pilas hasta el tamaño mínimo
    int lenOriginal = 0; //usada para almacenar la longitud original de la pila
    int minimumSize = NUM_THREADS; //tamaño minimo de la pila
    int aux = checkAvailability(argc, argv); 
    
    if(aux == -1){ //No se ha especificado el nombre del archivo, y por lo tanto se sale de la ejecución
        fprintf(stderr,"Syntax error: ./av3 file_name \n");
        return -1;

    }else if(aux == 0){ //la pila no existe
        printf("Creating new file to store the stack \n");
        pila = my_stack_init(sizeof(int)); //cogemos el puntero del objeto pila
        int fichero = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); //aqui queremos crear el archivo 
        if(fichero < 0){
            fprintf(stderr,"An error has ocurred during the creation of the file.\n");
            close(fichero);
            return -1;
        }
        close(fichero);
        len = minimumSize;
    }else{ //la pila si existe
        printf("Reading file\n");
        pila = my_stack_read(filename); //aqui leemos la pila del archivo y guardamos el puntero del objeto
        len = my_stack_len(pila); //cogemos el tamaño de la pila
        lenOriginal = len;
        len = minimumSize - len; //tenemos que asegurarnos que haya el mínimo de elementos, por lo que len contendrá los elementos que faltan
        if (len < 0){
            len = 0;
        }      
    }

    for(int i = 0; i < len; i++){ //llenamos las posiciones necesarias de la pila con 0s
        my_stack_push(pila, 0);
    }

    //Presentación del programa//
    printf("\nThreads: %d, Iterations: %d \n", NUM_THREADS, ITERATIONS);
    printf("Stack->size: %d \n", pila->size);
    printf("Original stack lenght: %d\n", lenOriginal);
    printf("New stack lenght: %d\n\n", my_stack_len(pila));

    //Creacion de los hilos//
    pthread_t *arrayThreads [NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++){
       pthread_create(arrayThreads[i],NULL,worker,NULL);
    }

    //espera del hilo principal//
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(*arrayThreads[i],NULL);
    }

    //guardado de la pila en el archivo//
    int elements = my_stack_write(pila,filename);
    if(elements == -1){
        fprintf(stderr,"An error has ocurred when trying to write into the file.\n");
        return -1;
    }
    printf("Written elements from stack to file: %d\n", elements);
    int releasedBytes = my_stack_purge(pila);
    if(releasedBytes == -1){
        fprintf(stderr,"An error has ocurred when trying to purge the stack. The stack is NULL already.\n");
        return -1;
    }
    printf("Released bytes: %d\n", releasedBytes);
    pthread_exit(NULL);
}

int checkAvailability(int argc, char *filename[]){
    //se coge por consola el nombre del archivo i se guarda en filename (PENDIENTE)
    //Por lo que se ve en el pdf, Adelaida arranca este archivo con un argumento extra
    //ejemplo: ./av3 s4 
    //supuestamente eso es el nombre del archivo donde esta la pila, y hay que guardarlo
    //voy a probar a ponerlo en los parametros de main
    printf("Argc vale %d", argc);
    if (argc == 2){
        int pila = open(filename[1], O_RDONLY , S_IRUSR); //intentamos abrir el archivo
    if(pila < 0){ //no existe la pila
        close(pila);
        return 0;
    }
        return 1;
    }
    return -1;
}

void *worker(void *ptr){
    printf("%d) Thread %ld created\n",*(int *)ptr,pthread_self());
    for (int i = 0 ; i < ITERATIONS; i++){
        //zona crítica//
        printf(rojo);
        pthread_mutex_lock(&mutex);
        //printf(rojo);
        printf("Soy el hilo %ld ejecutando un pop\n", pthread_self());
        //printf(blanco);
        int data = *((int *)my_stack_pop(pila));
        pthread_mutex_unlock(&mutex);
        //Fin zona crítica//
        data++;
        //zona crítica//
        printf(azul);
        pthread_mutex_lock(&mutex);
        //printf(azul);
        printf("Soy el hilo %ld ejecutando un push\n", pthread_self());
        //printf(blanco);               //Ponemos el color a blanco
        my_stack_push(pila, &data);
        pthread_mutex_unlock(&mutex);
        //Fin zona crítica//
    }
    pthread_exit(NULL);
}


