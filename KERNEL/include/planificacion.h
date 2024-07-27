#ifndef KERNEL_PLANIFICACION_H_
#define KERNEL_PLANIFICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/temporal.h>

#include "../../SHARED/include/client_sockets.h"
#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "manejo_colas.h"
#include "desalojo.h"
#include "init.h"

//---------------------------- Variables ---------------------------- 
extern sem_t sem_corto_plazo;
extern int hubo_interrupcion; //Flag de interrupcion
extern t_list* cola;
extern t_list* cola_ready; // Ver que hacer con cola ready global
extern t_list* cola_new;
extern t_list* cola_aux_vrr;
extern int tiempo_ejecutado;
extern int IMPRIMIR;
extern int SINCRO;
extern int PID_EXEC;
//extern t_io_gen* interfaz;

extern bool existe_tipo, existencia_interfaz;

//---------------------------- Estructura ---------------------------- 
typedef struct {
    uint32_t PID;
    uint32_t q_restante;
} t_hilo_plani_quantum;
/*
typedef struct {
    t_pcb* pcb;
} t_hilo_plani_quantum;


*/

//---------------------------- Funciones ---------------------------- 
// Inicializar Proceso
t_pcb* crear_pcb();
t_registros_gral* init_registros();
int generarPID();
void iniciar_proceso(char*);

// Finalizar Proceso
void finalizar_proceso(char*);
t_pcb* buscar_pcb(uint32_t);
t_pcb* buscar_pcb_lista(t_list*, uint32_t);
t_pcb* buscar_pcb_rec_int(char*, uint32_t);

//Conexion Memoria
proceso_memoria* crear_proceso_memoria (uint32_t, char*);
void enviar_proceso_memoria (t_pcb*, char*);

//Planificacion
void planificador_largo_plazo();
void planificador_corto_plazo();
void manejar_ingreso_a_ready(t_pcb*);
t_pcb* obtener_sgte_pcb_exec();
void manejar_planificacion(t_pcb*);

void planificar_quantum(t_pcb*); 
void rafaga_cpu_interrupt(void*);
t_hilo_plani_quantum* init_args_hilo_plani_quantum(t_pcb*);
void manejar_vrr(t_pcb*, t_list*, pthread_mutex_t*);

void manejar_quantum(t_pcb*);
void actualizar_tiempo_exec(t_temporal*);

void iniciar_planificacion();
void detener_planificacion();
void accion_int_rec(int, bool);
void imprimir_rec_int(char*);

//LIberar estructuras
void liberar_parametros_io_gen(interfaz_a_enviar_gen*);
void liberar_parametros_io_std(interfaz_a_enviar_std*);

#endif