#ifndef MEMORIA_INIT_H
#define MEMORIA_INIT_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"

#include "cpu_server.h"
#include "io_server.h"
#include "kernel_server.h"
#include "memoria_real.h"
#include <pthread.h>

// Variables globales
extern char* PUERTO_ESCUCHA;
extern char* IP_MEMORIA;
extern int TAM_MEMORIA;
extern uint32_t TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;
extern t_log* logger_memoria;
extern t_log* logger_memoria_extra;
extern t_config* config_memoria;
extern t_list* procesos;
extern int mem_serv_socket;
extern int io_socket;
extern int kernel_socket;
extern int cpu_socket;

extern pthread_mutex_t mutex_conexion_io_mem;
extern pthread_mutex_t mutex_frames_status;
extern pthread_mutex_t mutex_lista_procesos;
// logger_memoria
void iniciar_logger();

// config_memoria
void iniciar_config(char*);
void obtener_config();
void loggear_config();

// Programa
void terminar_programa();
void liberar_procesos(t_proceso*);


#endif