#ifndef KERNEL_MAIN_H_
#define KERNEL_MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>

#include "../../SHARED/include/client_sockets.h"
#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"
#include "client_multih.h"
#include "server_multih.h"
#include "planificacion.h"
#include "consola.h"

#endif