#ifndef IO_CONSOLA_IO_H
#define IO_CONSOLA_IO_H

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

void consola_io();
void string_split_free(char***);
char* consola_io_stdin(uint32_t);

bool procesar_comando(char*);

#endif
