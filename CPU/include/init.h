#ifndef CPU_INIT_H
#define CPU_INIT_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"
#include <pthread.h>

// Variables globales

extern int conexion_memoria;
extern int conexion_kernel_dispatch;
extern int conexion_kernel_interrupt;
extern int cpu_dispatch_socket;
extern int cpu_interrupt_socket;
extern int hubo_interrupcion;
extern char* IP_CPU;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern int CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;
extern int result;
extern int INTERRUPT;
extern int NOT_INTERRUPT;
extern uint32_t tam_pagina;
extern size_t bytes;
extern t_log* logger_cpu;
extern t_log* logger_cpu_extra;
extern t_config* config_cpu;
extern pthread_mutex_t mutex_pid_exec;
extern pthread_mutex_t mutex_interrupcion;
extern int OK;
extern int ERROR;
extern int INTERRUPT_EXIT;
extern int NOT_INTERRUPT;

t_pcb* pedir_instrucciones(t_pcb*);

// logger_cpu
void iniciar_logger(void);

// Config
void iniciar_config(char*);
void obtener_config();
void loggear_config();

// Handshake
void handshake_cliente_cpu(size_t, handshake, int);

// Programa
void terminar_programa();

void string_split_free(char***);

// TLB
void init_tlb();
#endif