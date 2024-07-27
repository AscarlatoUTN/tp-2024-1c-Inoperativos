#ifndef KERNEL_INIT_H_
#define KERNEL_INIT_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <string.h>
#include <pthread.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../KERNEL/include/consola.h"
#include "../../SHARED/include/structs.h"

#include "client_multih.h"
#include "server_multih.h"
#include "planificacion.h"
#include "desalojo.h"

// Variables globales
extern int io_socket;
extern int conexion_memoria;
extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern char* IP_MEMORIA;
extern char* IP_CPU;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern char* ALGORITMO_PLANIFICACION;
extern int QUANTUM;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;
extern int GRADO_MULTIPROGRAMACION;
extern char *PATH_INSTRUCCIONES;
extern int INTERRUPT;
extern int NOT_INTERRUPT;
extern handshake HANDSHAKE;
extern int32_t result;
extern size_t bytes;
extern int32_t OK;
extern int32_t ERROR;
extern sem_t rr_exit;
extern uint32_t BLOCKED;
extern uint32_t INSTANCES;
extern int32_t SEM_CONTADOR;
extern int32_t SEM_BINARIO;
extern bool WAIT;
extern bool SIGNAL;
extern bool LOCK;
extern bool UNLOCK;

extern t_log* logger_kernel;
extern t_log* logger_kernel_extra;
extern t_config* config_kernel;

extern t_dictionary* diccionario_colas;
extern t_dictionary* diccionario_sem;
extern t_dictionary* diccionario_mutex;


//Semaforos
extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_lista_gen;
extern pthread_mutex_t mutex_dict_colas;
extern pthread_mutex_t mutex_dict_sem;
extern pthread_mutex_t mutex_dict_mutex;
extern pthread_mutex_t mutex_interrupcion;
//extern pthread_mutex_t mutex_tiempo_ejecutado;
extern pthread_mutex_t mutex_cola_aux_vrr;
extern pthread_mutex_t mutex_lista_recursos;
extern pthread_mutex_t mutex_lista_pid_recs;
extern pthread_mutex_t mutex_conexion_io_kernel;
extern pthread_mutex_t mutex_lista_stdin;
extern pthread_mutex_t mutex_lista_stdout;
extern pthread_mutex_t mutex_lista_dialfs;
extern pthread_mutex_t mutex_pid;
extern pthread_mutex_t mutex_plani;

extern sem_t planificador_corto;
extern sem_t planificador_largo;

extern sem_t sem_multiprogramacion;
extern sem_t sem_pcb_espera_ready;
extern sem_t sem_tiempo_ejecutado;

extern sem_t sem_exec;
extern sem_t sem_Q;

extern t_list* cola;
extern t_list* cola_ready; // Ver que hacer con cola ready global
extern t_list* cola_new;
extern t_list* cola_aux_vrr;
extern t_list* interfaces_gen;
extern t_list* recursos;
extern t_list* pid_recs;
extern t_list* interfaces_stdin;
extern t_list* interfaces_stdout;
extern t_list* interfaces_dialfs;

// logger_kernel
void iniciar_logger();

// Config
void iniciar_config(char*);
void obtener_config();
void loggear_config();

// Hilos conexion I/O - Dispatch
typedef struct {
    char* kernel_serv_name;
    int conexion_interfaz;
} t_procesar_conexion_args;

// Hilos conexion I/O - Dispatch
typedef struct {
    int conexion_io_kernel;
    char* io_kernel_serv_name;
    char* nombre_interfaz; 
    header tipo_interfaz;
} t_hilo_interfaz;

void init_hilo_motivos_desalojo(pthread_t*);
void init_hilo_largo_plazo(pthread_t*);
void init_hilo_server(pthread_t*);
void init_hilo_consola(pthread_t*);

int server_escuchar(void*);

void init_colas();
void init_diccionario_colas();
void init_dic_sem();

//Inicializacion de semaforos
void init_sem_planificacion();

// Programa
void terminar_programa_kernel();

void liberar_io_gen(t_io_gen*);

void liberar_io_std(t_io_std*);

void liberar_io_dialfs(t_io_df*);

void liberar_recurso(recurso_t*);

void liberar_pid_recs(pid_rec*);

void liberar_diccionario_colas(t_list*);

void liberar_int_a_enviar_fs(interfaz_a_enviar_fs*);

void destruir_mutex(void*);

void destruir_lista_de_colas(void*);

void destruir_sem(void*);

#endif
