#ifndef MANEJO_COLAS_H_
#define MANEJO_COLAS_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/temporal.h>

#include "../../SHARED/include/client_sockets.h"
#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"

void ingresar_a_new(t_pcb*);
void ingresar_a_ready(t_list*, t_pcb*);
t_pcb* obtener_sgte_ready();
bool agregar_pcb_a_cola(char*, t_pcb*);
t_pcb* sacar_pcb_cola_block(char*);
void ingresar_a_block(t_list*, t_pcb*, pthread_mutex_t*);
void agregar_a_cola_aux_vrr(t_pcb*, t_list*, pthread_mutex_t*);
pthread_mutex_t* obtener_mutex_diccionario(char*);
t_pcb* cola_block_get(char*);
t_pcb* sacar_pcb_lista(t_list*, uint32_t, pthread_mutex_t*);
void ingresar_fcb_lista(t_list*, t_fcb*, pthread_mutex_t*);
bool agregar_fcb_a_cola(char*, t_fcb*);
void add_fcb_lista(t_list*, t_fcb*);
t_pcb* finish_pcb(t_list*, uint32_t);

#endif