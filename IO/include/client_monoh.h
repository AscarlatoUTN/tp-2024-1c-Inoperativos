#ifndef IO_CLIENT_MONOH_H_
#define IO_CLIENT_MONOH_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <math.h>

#include "../../SHARED/include/client_sockets.h"
#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/memoria.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"

#include "init.h"
#include "ops_dialfs.h"

extern handshake HANDSHAKE;
extern uint32_t tam_pag;
extern t_list* lista_fcbs;

void handshake_cliente_io(size_t, handshake, int, t_config*, char*, ESTADO estado); 
int establecer_conexion(void*);
int conexion_dispatch_io_gen(char*, t_io_gen*);
int conexion_cliente_stdin(t_io_std*);
int conexion_cliente_stdout(t_io_std*);
int conexion_cliente_dialfs(t_io_df*);

#endif