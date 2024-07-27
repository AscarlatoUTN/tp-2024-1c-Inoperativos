#ifndef KERNEL_CONSOLA_H
#define KERNEL_CONSOLA_H

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

#include "init.h"
#include "planificacion.h"

typedef struct {
    void (*func)(char*);
    char* consoleName;
} t_console;

typedef struct {
    char* input;
} hilo_de_consola;

void procesar_comando(void *);
void consola();

void multiprogramacion(char*);
void bajar_multiprogramacion();
void ejecutar_script(char*);
void proceso_estado();
void listar_procesos(char*, t_list*);

#endif
