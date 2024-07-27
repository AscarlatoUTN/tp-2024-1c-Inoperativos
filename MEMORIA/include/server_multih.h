#ifndef MEMORIA_SERVERMULTIH_H
#define MEMORIA_SERVERMULTIH_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <pthread.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"

// Definición de la estructura para los argumentos de procesar_conexion
typedef struct {
    int mem_serv_socket;
    char* mem_serv_name;
} t_procesar_conexion_args;

extern handshake HANDHSAKE;
extern int result;
extern size_t bytes;
extern int OK;
extern int ERROR;
extern int tamanio;
extern int RECIBIDO;
extern int RECHAZADO;
extern int codigo_instruccion;


int server_escuchar(int, char*);
void* procesar_conexion_cpu(void*);
void* procesar_conexion_kernel(void*);
void* procesar_conexion_io(void* void_args);

#endif