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
#define ITERATIONS 1000000

// Ricos colorines Bacanos //
#define BLANCO  "\033[0;37m"
#define ROJO    "\033[0;31m"
#define CIAN    "\033[0;36m"
#define NEGRO   "\033[0;30m"                                        // Usar en caso de tener la terminal en blanco (te queremos Adelaida, lo hacemos por ti)
                                                                    // edit: Lo hicieron sin mi.

pthread_t arrayThreads[NUM_THREADS];                                // Array para contener el ID de los threads
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;                  // Semaforo
struct my_stack *pila;                                              // Apuntador al objeto pila

// Funciones //
void *worker(void *ptr);
int checkAvailability(int argc, char *filename[]);                  // Funcion para confirmar si el fichero ya existe

int main(int argc, char *argv[]){
    
    char *filename = argv[1];                                       // Nombre del archivo de la pila
    int len = 0;                                                    // Usada para rellenar las pilas hasta el tamaño mínimo
    int lenOriginal = 0;                                            // Usada para almacenar la longitud original de la pila
    int minimumSize = NUM_THREADS;                                  // Tamaño minimo de la pila
    int aux = checkAvailability(argc, argv);                        // Aux almacena el resultado de checkAvailability
    
    if(aux == -1){                                                  // No se ha especificado el nombre del archivo, y por lo tanto se sale de la ejecución
        fprintf(stderr,"Syntax error: ./av3 file_name \n");
        return -1;
    }else if(aux == 0){                                             // La pila no existe
        printf("Creating new file to store the stack \n");
        pila = my_stack_init(sizeof(int));                          // Cogemos el puntero del objeto pila
        int fichero = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if(fichero < 0){
            fprintf(stderr,"An error has ocurred during the creation of the file.\n");
            close(fichero);
            return -1;
        }
        close(fichero);
        len = minimumSize;
    }else{                                                          // La pila si existe
        printf("Reading file\n");
        pila = my_stack_read(filename);                             // Aqui leemos la pila del archivo y guardamos el puntero del objeto
        len = my_stack_len(pila);                                   // Cogemos el tamaño de la pila
        lenOriginal = len;
        len = minimumSize - len;                                    // Tenemos que asegurarnos que haya el mínimo de elementos, por lo que len contendrá los elementos que faltan
        if (len < 0){
            len = 0;
        }      
    }

    for(int i = 0; i < len; i++){                                   // Llenamos las posiciones necesarias de la pila con 0s
        void *rellenuto = malloc(sizeof(int));
        *((int *)rellenuto) = 0;
        my_stack_push(pila, rellenuto);
    }

    // Presentación del programa //
    printf("\nThreads: %d, Iterations: %d \n", NUM_THREADS, ITERATIONS);
    printf("Stack->size: %d \n", pila->size);
    printf("Original stack lenght: %d\n", lenOriginal);
    printf("New stack lenght: %d\n\n", my_stack_len(pila));
    
    // Creacion de los hilos //
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_create(&arrayThreads[i],NULL,worker,NULL);
    }

    // Espera del hilo principal //
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(arrayThreads[i],NULL);
    }
    printf(BLANCO);
    // Guardado de la pila en el archivo //
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
    
    if (argc == 2){
        int pila = open(filename[1], O_RDONLY , S_IRUSR);           // Intentamos abrir el archivo
        if(pila < 0){                                               // No existe la pila
            close(pila);
            return 0;
        }
        close(pila);
        return 1;
    }
    return -1;
}

void *worker(void *ptr){

    for (int i = 0 ; i < NUM_THREADS ; i++){                        // Impresion de los hilos creados
        if(arrayThreads[i] == pthread_self()){
            printf(BLANCO"%d)Thread %ld created\n",i,arrayThreads[i]);
            break;
        }
    }

    for (int i = 0 ; i < ITERATIONS; i++){
        // Zona crítica //
        pthread_mutex_lock(&mutex);
        //printf(ROJO"Soy el hilo %ld ejecutando un pop\n", pthread_self());
        void *data = my_stack_pop(pila);
        pthread_mutex_unlock(&mutex);
        // Fin zona crítica //

        *((int *)data) = *((int *)data) + 1;

        // Zona crítica //
        pthread_mutex_lock(&mutex);
        //printf(CIAN"Soy el hilo %ld ejecutando un push\n", pthread_self());
        my_stack_push(pila, data);
        pthread_mutex_unlock(&mutex);
        // Fin zona crítica //
    }
    pthread_exit(NULL);
}