#ifndef KERNEL_DESALOJO_H_
#define KERNEL_DESALOJO_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include <dirent.h>

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
#include "planificacion.h"
#include "init.h"
#include "manejo_recursos.h"
//#include "bitarray.h"

// ---------------------------- Funciones ---------------------------- 

void manejar_fin_quantum(t_pcb*);
void manejar_io_gen_sleep(t_pcb*, uint32_t);
void manejar_exit(t_pcb*);
bool manejar_motivo_desalojo(int, t_pcb*, uint32_t);
void motivo_desalojo();

// ------------------------------  SIGNAL 
void manejar_signal(t_pcb*, uint32_t);

// ------------------------------  WAIT 
void manejar_wait(t_pcb*, uint32_t);
t_list* get_cola_blocked(char*);

//------------------------------ MANEJAR IO STDIN
void manejar_io_std(t_pcb*, uint32_t, t_list*);

//------------------------------ MANEJAR IO GEN SLEEP
void manejar_io_gen_sleep(t_pcb*, uint32_t); 

// Función genérica para manejar la existencia de una interfaz
bool interfaz_valida(t_pcb*, char*, bool);
void liberar_parametros_fs_rw(parametros_fs_rw*);
void enviar_params_inst(header, params_inst_fs*, int);

//------------------------------ MANEJAR IO_FS
void manejar_fs(t_pcb*, uint32_t, header);
void manejar_fs_rw(t_pcb*, uint32_t, header);

#endif