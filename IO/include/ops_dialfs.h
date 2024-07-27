#ifndef OPS_DIALFS_H
#define OPS_DIALFS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include "../../SHARED/include/client_sockets.h"
#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"
#include "../include/consola_io.h"

#include "init.h"

// Variables Globales
extern int FREE_BLOCK_COUNT;
extern size_t bitmap_size;
extern t_config* arch_metadata;
extern pthread_mutex_t mutex_bitmap;
extern pthread_mutex_t mutex_block_count;
extern pthread_mutex_t mutex_lista_fcbs;
extern pthread_mutex_t mutex_archivo_bloques;

//Creacion de archivos
void* crear_archivo_bitmap();
void* crear_archivo_bloques();

char* construir_ruta_archivo(char*);
int abrir_archivo(char*, size_t);
void* mapear_archivo(int, size_t, char*);
void* crear_y_mapear_archivo(char*, size_t);
bool archivo_duplicado(char*);

//Disco - Bloques

uint32_t actualizar_espacio_disco(char*, header);
uint32_t status_bloque(uint32_t, bool, uint32_t);
uint32_t deallocate_blocks(uint32_t, uint32_t);
uint32_t allocate_blocks(uint32_t, uint32_t);
uint32_t buscar_fst_bloque_libre();
t_fcb* get_fcb_lista(char*);
bool bloque_ocupado(int);
t_fcb* sacar_fcb_lista(char*);
void liberar_fcb(t_fcb*);
t_fcb* inicializar_fcb(char*);
void buscar_directorio();

// Archivo
void FS_CREATE(char*, char*, uint32_t);
uint32_t calcular_bloques(uint32_t);
bool FS_DELETE(char*, char*);
void escribir_metadata(t_fcb*, uint32_t, int);
t_config* crear_archivo_metadata(char*);

//FS_TRUNCATE
bool FS_TRUNCATE(parametros_fs*);
bool disminuir_archivo(t_fcb*, uint32_t, uint32_t, uint32_t);
bool aumentar_archivo(t_fcb*, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t espacio_contiguo_ady(uint32_t, uint32_t, uint32_t );
uint32_t espacio_contiguo_disp(uint32_t, uint32_t, uint32_t, char*);

// FS_compcatar
uint32_t compactar_fs(char*, int);
bool escribir_arch_bloques (void*, int, int);

void liberar_fcb(t_fcb*);

//FS_READ
char* FS_READ(char*, int, int);


//FS_WRITE
bool FS_WRITE (params_inst_fs*, void*, t_fcb*);


#endif