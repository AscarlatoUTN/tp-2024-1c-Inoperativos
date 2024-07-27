#ifndef MEMORIA_REAL_H
#define MEMORIA_REAL_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <pthread.h>
#include <commons/bitarray.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"
#include "server_multih.h"

extern bool SET;
extern bool clean;


extern pthread_mutex_t mutex_user_space;
extern void* user_space;
extern t_bitarray* frames_status;
extern char* bitmap;

void init_memoria();
void inicializar_bitarray();
bool marco_ocupado(int);
uint32_t allocate_frame();
void deallocate_frame(uint32_t);
void status_frame(uint32_t, bool);
bool add_pages(t_proceso*, int, int);
void remove_pages (t_proceso*, int);
int modificar_tabla_paginas(uint32_t, uint32_t, uint32_t);
t_proceso* encontrar_proceso_por_pid(uint32_t);
void eliminar_process(uint32_t);
uint32_t acceder_tabla_paginas(uint32_t, int);
char* acceso_usuario(uint32_t, header, void*, uint32_t);

#endif