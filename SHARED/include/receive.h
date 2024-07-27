#ifndef RECEIVE_H_
#define RECEIVE_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <commons/collections/list.h>
#include <commons/log.h>

#include "client_sockets.h"
#include "structs.h"
#include "math.h"

// Recepción
void* recibir_buffer(int*, int);
t_pedir_instruccion* recibir_instruccion(int);
interfaz_a_enviar_gen* recibir_parametros_io_gen(int, uint32_t);
interfaz_a_enviar_std* recibir_parametros_interfaz_std(int, uint32_t);
interfaz_a_enviar_fs* recibir_parametros_kernel_fs(int, uint32_t);
parametros_fs* recibir_parametros_io_fs(int);
proceso_memoria* recibir_proceso(int);
char* recibir_recurso(int, uint32_t);
pcb_motivo* recibir_pcb_motivo(int);
t_io_gen* recibir_interfaz_gen(int);
char* guardar_mensaje(int);
void recibir_mensaje(int, t_log*);
t_list* recibir_paquete(int);
int recibir_operacion(int);
t_pcb* recibir_pcb(int);
int recibir_nums(int);
void* recv_pcb(int*, int);
df_size* recibir_df_size(int);
t_io_std* recibir_io_std(int);
t_list* recibir_lista_parametros(void*, uint32_t);
t_tabla_r* recibir_peticion_lectura(int);
t_tabla_w* recibir_peticion_escritura(int);
t_io_df* recibir_interfaz_dialfs(int);
io_gen_pid* recibir_io_gen_pid (int);
char* guardar_mensaje_fin_io(int);
params_inst_fs* recibir_params_inst_fs(int);
parametros_fs_rw* recibir_params_fs_rw (int, uint32_t);

#endif