#ifndef CPU_SERVER_MULTIH_H_
#define CPU_SERVER_MULTIH_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"

extern handshake HANDSHAKE;
extern uint32_t PID_EXEC;


typedef struct {
    char* cpu_serv_name;
} t_procesar_conexion_args;

void* establecer_conexion_interrupt(void*);
void* establecer_conexion_dispatch(char*);
int server_escuchar(char*);
int manejo_interrupts(char*);

#endif