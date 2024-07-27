#ifndef SERV_SOCKETS_H_
#define SERV_SOCKETS_H_


#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <stdbool.h>

// aca se definen las operaciones basicas de iniciar servidor, esperar cliente, crear conexion y liberar conexion

int iniciar_servidor(t_log*, const char*,char*, char*);
int esperar_cliente(t_log*, const char*, int);

#endif