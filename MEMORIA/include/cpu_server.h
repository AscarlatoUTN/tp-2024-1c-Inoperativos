#ifndef CPU_SERVER_H
#define CPU_SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <pthread.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/memoria.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"
#include "server_multih.h"
#include "memoria_real.h"

void* procesar_conexion_cpu(void*);

#endif