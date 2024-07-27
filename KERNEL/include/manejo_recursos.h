#ifndef MANEJO_RECURSOS_H
#define MANEJO_RECURSOS_H

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


void init_recursos_sist();
recurso_t* init_recurso(char*, char*);
void init_semaforo_contador(recurso_t*);

// ------------------------------  WAIT
void manejador_recursos(char*);

// -------------------------- Funciones Auxiliares ---------------------------------------------
int calcular_tamanio_array(char**);
int instancias_disp(t_pcb*, char*);
recurso_t* encontrar_recurso(char*);
int agregar_recurso_asoc(t_pcb*, char*);
pid_rec* encontrar_proceso_por_PID(int);
int buscar_y_eliminar_recurso(pid_rec*, char*);
int eliminar_recursos_asociados(t_pcb*);
void op_mutex(char*, bool);
int aumentar_instancias_recurso(t_pcb*, char*);
pid_rec* crear_nuevo_proceso(uint32_t);
int eliminar_proceso_por_PID(int);
void init_sem_contador_recursos(recurso_t*);
int actualizar_recurso_asoc(t_pcb*, char*);

#endif