#ifndef CPU_INSTRUCCIONES_H
#define CPU_INSTRUCCIONES_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <math.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include "../../SHARED/include/memoria.h"
/*

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"
*/

#include "init.h"
//#include "traduccion.h"

//#include "ciclo_instrucciones.h"

extern char* registro;
extern char* registro_origen;
extern char* reg_destino;
extern t_pcb* pcb;
extern uint8_t valor;
extern int OOM;

// Estructura

typedef enum {
    I_SET,
    I_SUM,
    I_SUB,
    I_JNZ,
    I_MOV_IN,
    I_MOV_OUT,
    I_RESIZE,
    I_COPY_STRING,
    I_WAIT,
    I_SIGNAL,
    I_EXIT,
    I_IO_GEN_SLEEP,
    I_IO_STDIN_READ,
    I_IO_STDOUT_WRITE,
    I_IO_FS_CREATE,
    I_IO_FS_DELETE,
    I_IO_FS_TRUNCATE,
    I_IO_FS_WRITE,
    I_IO_FS_READ,   
} t_instruccion;

typedef enum {
    TIPO_DESCONOCIDO,
    TIPO_UINT8,
    TIPO_UINT32,
} tipoDato;

typedef struct {
    t_instruccion tipo_instruccion;
    char* nombre_instruccion;
} t_instruccion_MAP;

// Struct para procesar_instruccion
typedef struct {
    t_instruccion tipo_instruccion;
    char** argumentos;
    bool es_valido;
} t_instruccion_recibida;

// Registros
uint8_t* obtener_regs8(t_registros_gral*, char);
uint32_t* obtener_regs32(t_registros_gral*, char);
uint32_t* obtener_regs_especiales(t_pcb*, char);
void* obtener_registro(t_pcb*, char*);
int setear_valor(void*, void*, size_t); 
int op_artimetica( t_pcb*, char*, char*, int);
int realizar_op_aritmetica(void*, void*, size_t, size_t, int);
size_t retornar_tamanio (tipoDato);

//funciones
int SET(t_pcb*, char*, void*);
int SUM(t_pcb*, char*, char*);
int SUB(t_pcb*, char*, char*);
int JNZ(t_pcb*, char*, char*);
int MOV_IN(t_pcb*, char*, char*);
int MOV_OUT(t_pcb* , char* , char* );
int IO_GEN_SLEEP(t_pcb*, char*, char*);
uint32_t F_IO_STD(t_pcb*, char*, char*, char*, header);
int RESIZE (t_pcb*, int);
int COPY_STRING(t_pcb*, int);
uint32_t OP_FS(t_pcb*, char*, char*, char* , header);


//Funciones memoria
int traducir_direccion(uint32_t, int, int);
t_list* list_dfs(t_pcb*, uint32_t, uint32_t , uint32_t );

//void IO_FS_TRUNCATE(t_pcb*, char*, int, int);
uint32_t FS_RW(t_pcb*, char*, char*, char*, char*, char*, header);

#endif