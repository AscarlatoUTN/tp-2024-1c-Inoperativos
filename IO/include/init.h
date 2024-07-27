#ifndef MEMORIA_INIT_H
#define MEMORIA_INIT_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <stdlib.h> // para atoi 
#include <unistd.h> // para sleep y usleep
#include <pthread.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include "../../SHARED/include/client_sockets.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/structs.h"

#include "consola_io.h"
#include "client_monoh.h"

// Variables globales
extern int conexion_kernel;
extern int conexion_memoria;
extern char* TIPO_INTERFAZ;
extern uint32_t TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;
extern size_t bytes;
extern int32_t result;
extern int config_creada;
extern t_log* logger_io;
extern t_log* logger_io_extra;
extern t_config* config_io;

extern sem_t consola;

typedef struct {
    header tipo;
    char* tipo_interfaz;
} t_interfaz_MAP;

typedef struct {
    char* nombre;
    char* config;
    header tipo;
} t_init_interfaz;


// logger_io
void iniciar_logger(void);

// Programa
void terminar_programa();

// Interfaz
void crear_conexion(t_init_interfaz*, t_config*);
void interfaz_config(t_init_interfaz*);
void init_interfaz (char*, char*);
t_io_gen* crear_estructura_io_gen(t_config*, t_init_interfaz*);
t_io_std* crear_estructura_io_std(t_config*,t_init_interfaz*);
t_io_df* crear_estructura_io_dialfs(t_config*, t_init_interfaz*);

#endif