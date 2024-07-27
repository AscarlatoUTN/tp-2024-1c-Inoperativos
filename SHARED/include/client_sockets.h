#ifndef CLIENT_SOCKETS_H_
#define CLIENT_SOCKETS_H_

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/log.h>
#include <sys/types.h>
#include <commons/string.h>
#include <commons/config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int crear_conexion_client(char*, char*);

#endif