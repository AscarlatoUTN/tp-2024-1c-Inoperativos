#include "../include/consola_io.h"

// ------------------------------- STDIN --------------------------------

// Función principal que lee de la consola y trunca la entrada
char* consola_io_stdin(uint32_t tamanio){
    char* input = NULL;

    input = readline("> ");
    if (input == NULL){
        log_error(logger_io_extra, "Comando %p inválido \n", input);
        return NULL; // Salir de la función si la entrada es NULL
    }

    // Asignar memoria para la cadena truncada
    char* input_truncada = (char*)malloc(tamanio + 1);
    if (input_truncada == NULL){
        log_error(logger_io_extra, "No hay espacio suficiente en memoria \n");
        free(input); 
        return NULL; 
    }

    // Copiar la cantidad deseada de caracteres
    strncpy(input_truncada, input, tamanio);

    free(input); // Liberar la entrada original

    return input_truncada;
}

