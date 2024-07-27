#ifndef MEMORIA_H_
#define MEMORIA_H_

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
#include "receive.h"
#include "send.h"

char* leer_memoria(t_log*, t_list*, t_tabla_r*, int, uint32_t, uint32_t, header);
int escribir_memoria(t_log*, t_list*, uint32_t, int, void*, size_t, uint32_t, uint32_t, header);
void peticion_escritura(header, t_tabla_w*, int);
void agregar_peticion_escritura(t_paquete*, t_tabla_w*);
void peticion_lectura (header, t_tabla_r*, int);
void agregar_peticion_lectura(t_paquete*, t_tabla_r*);
void enviar_pedido_marco(header, int, t_tabla*);
void agregar_pedido_marco(t_paquete*, t_tabla*);
t_tabla* recibir_pedido_marco(int);
void enviar_pedido_resize(header, int, t_resize*);
void agregar_resize(t_paquete*, t_resize*);
t_resize* recibir_resize(int);

#endif