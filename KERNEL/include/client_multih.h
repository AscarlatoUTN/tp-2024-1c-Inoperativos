// Para la conexión Dispatch e Interrupt con CPU

#ifndef KERNEL_CLIENT_MULTIH_H_
#define KERNEL_CLIENT_MULTIH_H_

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

void handshake_cliente_kernel(size_t, handshake, int, int); 
int establecer_conexion(char*, char*);

#endif