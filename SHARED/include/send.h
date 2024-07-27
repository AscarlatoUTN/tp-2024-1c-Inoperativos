#ifndef SEND_H_
#define SEND_H_

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

// Crear y Eliminar
t_paquete* crear_paquete(header);
void eliminar_paquete(t_paquete*);

// Serialización
void* serializar_paquete(t_paquete*, int);
void serializar_instruccion(t_paquete*, t_pedir_instruccion*);

// Agregar
void agregar_a_paquete(t_paquete*, void*, int);
void agregar_a_proceso(t_paquete*, proceso_memoria*);
void agregar_pcb(t_paquete*, t_pcb*);
void agregar_pcb_recurso(t_paquete*, enviar_pcb_rec*);
void agregar_df_size(t_paquete*, df_size*);
void agregar_interfaz_gen(t_paquete*, t_io_gen*);
void agregar_interfaz_gen_paquete(t_paquete*, interfaz_a_enviar_gen*);
void agregar_interfaz_std(t_paquete*, t_io_std*);
void agregar_interfaz_fs(t_paquete*, interfaz_a_enviar_fs*);
void agregar_interfaz_io_gen (t_paquete*, interfaz_a_enviar_gen*);
void agregar_interfaz_io_std(t_paquete*, interfaz_a_enviar_std*);
void agregar_lista(t_list*, t_paquete*);
void agregar_interfaz_dialfs(t_paquete*, t_io_df*);
void agregar_io_gen_pid (t_paquete*, io_gen_pid*);
void agregar_parametros_fs (t_paquete*, parametros_fs*);

void enviar_mensaje(char*, int, header);
void enviar_paquete(t_paquete*, int);
void enviar_parametros_interfaz_std(header, interfaz_a_enviar_std*, int);
void enviar_pcb(int, t_pcb*, header);
void enviar_interfaz_gen(header, int, t_io_gen*);
void enviar_nums(header, int, uint32_t);
void pedir_instruccion(t_pedir_instruccion*, int, header);
void enviar_pcb_recurso(header, enviar_pcb_rec*, int);
void enviar_df_size(header, int, df_size*);
void enviar_interfaz_std(header, int, t_io_std*);
void enviar_interfaz_io_gen(header, interfaz_a_enviar_gen*, int);
void enviar_interfaz_fs(header, interfaz_a_enviar_fs*, int);
void enviar_interfaz_dialfs(header, int , t_io_df*);
void enviar_gen_pid(int, io_gen_pid*);
void enviar_parametros_fs(header, parametros_fs*, int);
void enviar_param_fs_rw(header, parametros_fs_rw*, int);
void agregar_parametros_fs_rw(t_paquete*, parametros_fs_rw*);
void agregar_parametros_inst_mem(t_paquete*, params_inst_fs*);

#endif