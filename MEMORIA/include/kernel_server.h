#ifndef KERNEL_SERVER_H
#define KERNEL_SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <pthread.h>

#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"
#include "server_multih.h"

void* procesar_conexion_kernel(void*);
void eliminar_proceso(t_proceso*);

#endif