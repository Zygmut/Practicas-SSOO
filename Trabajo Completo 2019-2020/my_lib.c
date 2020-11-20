#include <stdio.h>
#include <stdlib.h>
#include "my_lib.h"

//devuelve la longitud del string
size_t my_strlen(const char *str){
    int longitud = 0;
    for(int i = 0; str[i] != '\0'; i++){
        longitud = i+1;
    }
    return longitud;
}

//compara dos strings
int my_strcmp(const char *str1, const char *str2){
    int n = 0;
    int res = 0;
    while((str1[n] == str2[n]) && (str1[n] != '\0') && (str2[n] != '\0')){
        n++;
    }
    res = str1[n] - str2[n];
    return res;
}

//copia el string de origen en otra variable de destino
char *my_strcpy(char *dest, const char *src){
    int n = 0;
    while(src[n] != '\0'){
        dest[n] = src[n];
        n++;
    }
    dest[n] = '\0';
    return dest;
}

/*copia el número de carácteres pasados por parámetro del string de origen
al de destino*/
char *my_strncpy(char *dest, const char *src, size_t n){
    int i = 0;
    while((i < n) && (src[i] != '\0')){
        dest[i] = src[i];
        i++;
    }
    while(i < n){
        dest[i] = '\0';
        i++;
    }
    return dest;
}

//concatena dos strings
char *my_strcat(char *dest, const char *src){
    int i = 0;
    while(dest[i] != '\0'){
        i++;
    }
    int n = 0;
    while(src[n] != '\0'){
        dest[i] = src[n];
        i++;
        n++;
    }
    dest[i] = '\0';
    return dest;
}

//inicializa la  pila con el tamaño de datos pasados por parámetro
struct my_stack *my_stack_init (int size){
    struct my_stack *ptr;
    ptr = malloc(sizeof(struct my_stack));
    ptr->size = size;
    ptr->first = NULL;
    return ptr;
}

//mete un elemento en la pila
int my_stack_push (struct my_stack *stack, void *data){
    struct my_stack_node *ptr;
    ptr = malloc(sizeof(struct my_stack_node));
    ptr->data = data;
    if((stack != NULL) && (stack->size > 0)){
        if(stack->first == NULL){
            stack->first = ptr;
            ptr->next = NULL;
        } else{
            ptr->next = stack->first;
            stack->first = ptr;
        }
        return EXIT_SUCCESS;
    }else{
        return EXIT_FAILURE;
    }
}

//saca un elemento de la pila
void *my_stack_pop (struct my_stack *stack){
    struct my_stack_node *ptr = stack->first;
    void *data = NULL;
    if(ptr != NULL){
        data = ptr->data;
        stack->first = ptr->next;
        free(ptr);
    }
    return data;
}

//devuelve el número de nodos de la pila
int my_stack_len (struct my_stack *stack){
    int n = 0;
    struct my_stack_node *ptr = stack->first;
    while(ptr != NULL){
        n++;
        ptr = ptr->next;
    }
    return n;
}

/*escribe la pila en un fichero pasado por parámetro
para escribir la pila en orden, primero la volcamos en una pila auxiliar*/
int my_stack_write (struct my_stack *stack, char *filename){
    struct my_stack_node *ptr = stack->first;
    struct my_stack *aux;
    aux = my_stack_init(stack->size);
    //volcamos la pila en una auxiliar para escribirla al revés
    while(ptr != NULL){
        my_stack_push(aux, ptr->data);
        ptr = ptr->next;
    }
    //cogemos el tamaño de los datos a escribir
    int *size = &stack->size;
    void *data;
    int n = 0;
    //abrimos el fichero
    int fichero = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fichero < 0){
        return -1;
    }
    //escribimos el tamaño de los datos para luego saber cuanto hay que leer
    int bytes = write(fichero, size, sizeof(stack->size));
    if(bytes != sizeof(stack->size)){
        close(fichero);
        return -1;
    }
    //escribimos los datos en el fichero
    ptr = aux->first;
    while(ptr != NULL){
        data = ptr->data;
        bytes = write(fichero, data, stack->size);
        if(bytes != stack->size){
            close(fichero);
            return -1;
        }
        my_stack_pop(aux);
        ptr = aux->first;
        n++;
    }
    //liberamos nodo auxiliar y cerramos el fichero
    free(aux);
    close(fichero);
    return n;
}

/*lee una pila del fichero pasado por parámetro
el fichero tiene una estructura: primero hay un entero que determina el
tamaño de los datos con los que trabaja la pila y después vienen los datos*/
struct my_stack *my_stack_read (char *filename){
    struct my_stack *pila;
    int size;
    int bytes;
    void *datos;
    //abrimos el fichero
    int fichero = open(filename, O_RDONLY , S_IRUSR | S_IWUSR);
    if(fichero < 0){
        return NULL;
    }
    //leemos el tamaño de los datos a leer
    read(fichero, &size, sizeof(int));
    //inicializamos la pila
    pila = my_stack_init(size);
    datos = malloc(pila->size);
    //leemos los datos
    bytes = read(fichero, datos, pila->size);
    while(bytes == pila->size){
        my_stack_push(pila,datos);
        datos = malloc(pila->size);
        bytes = read(fichero, datos, pila->size);
    }
    //cerramos el fichero
    close(fichero);
    return pila;
}

/*libera la memoria utilizada por la pila y devuelve el número de nodos
eliminados*/
int my_stack_purge (struct my_stack *stack){
    int n = 0;
    struct my_stack_node *ptr = stack->first;
    struct my_stack_node *ptr1;
    while(ptr != NULL){
        ptr1 = ptr;
        ptr = ptr->next;
        n += sizeof(struct my_stack_node);
        n += stack->size;
        free(ptr1);
    }
    n += sizeof(struct my_stack);
    free(stack);
    return n;
}