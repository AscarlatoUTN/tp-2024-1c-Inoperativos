#ifndef KERNEL_SERVER_MULTIH_H_
#define KERNEL_SERVER_MULTIH_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>

#include "../../SHARED/include/client_sockets.h"
#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"
#include "planificacion.h"

extern uint8_t SEM_RECURSO;
extern uint8_t SEM_INTERFAZ;

// Funcion para escuchar del servidor 
int server_escuchar(void*);

//Funciones que init interfaces
void manejar_init_gen(int, char*);
void manejar_init_std(int, char*,int);
void* esperar_fin_io(void*);
int procesar_operacion(int, char*, header);
int manejar_codigo_instruccion(int, t_pcb*, char*);
void enviar_confirmacion(char*, int);


//Funciones auxiliares de inicializacion
void inicializar_interfaz(void*, char*, pthread_mutex_t*, t_list*, int);
void crear_hilo_interfaz(int, char*, header);
void cerrar_conexion(int, char*);

//Inicializar Elementos para Interfaces
void init_cola_blocked(char*);
void init_semaforo_block(char*, uint8_t);
void init_mutex(char*, int32_t);
pthread_mutex_t* inicializar_mutex();
char* generar_nombre_mutex(int32_t, char*);
char* construir_nombre_sem(char*, char*);
void* op_sem_block(char*, bool, int, int);
char* obtener_nombre_semaforo(char*, int);
sem_t* obtener_semaforo(char*);
void signal_semaforo(sem_t*, int); 


//Eliminar elementos
void eliminar_interfaz(char*, header, int);

int comparar_io_gen(void*, char*);
int comparar_io_std(void*, char*);
int comparar_io_dialfs(void*, char*);





#endif