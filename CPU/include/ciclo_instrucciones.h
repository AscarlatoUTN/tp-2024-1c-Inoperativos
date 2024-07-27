#ifndef CPU_CICLO_INSTRUCCIONES_H
#define CPU_CICLO_INSTRUCCIONES_H

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

#include "instrucciones.h"
#include "init.h"

extern char* nombre_interfaz;
extern char* unidades_de_trabajo;
extern char* registro_direccion;
extern char* registro_tamanio;
extern char* nombre_archivo;
extern char* recurso;
extern char* puntero_archivo;
extern int espacios;

t_instruccion_recibida* procesar_instruccion(char*);
int contadorEspacios(char*);
t_pcb cargar_pcb(t_pcb*, t_list*);
void liberar_instruccion_recibida(t_instruccion_recibida*);
void free_instruccion_recibida(t_instruccion_recibida*, int);

int execute_SET(t_pcb*, char*, int,  t_instruccion_recibida*, int);
int execute_SUM(t_pcb*, char*, char*, t_instruccion_recibida*, int);
int execute_SUB(t_pcb*, char*, char*, t_instruccion_recibida*, int);
int execute_JNZ(t_pcb*, char*, char*, t_instruccion_recibida*, int);
int execute_EXIT(t_pcb*);
void execute_recursos(header, t_pcb*, char*);
int execute_IO_GEN_SLEEP(t_pcb*, char*, char*);
uint32_t execute_IO_STDIN_READ(t_pcb*, char*, char*, char*);
uint32_t execute_IO_STDOUT_WRITE(t_pcb*, char*, char*, char*);
int execute_MOV_IN(t_pcb*, char*, char*,  t_instruccion_recibida*, int);
int execute_MOV_OUT(t_pcb*, char*, char*, t_instruccion_recibida*, int );
int execute_RESIZE(t_pcb*, int, t_instruccion_recibida*, int);
int execute_COPY_STRING(t_pcb*, int, t_instruccion_recibida*, int);
int execute_OOM(t_pcb*);

#endif