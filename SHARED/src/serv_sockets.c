#include "../include/serv_sockets.h"

// aca se definen todas las funciones declaradas en serv_sockets.h, 
// es decir iniciar servidor, esperar cliente, crear conexion y liberar conexion

// Funcion para iniciar un servidor que escucha en una direccion IP y puerto especificos
int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto) {
    int socket_servidor;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Permite IPv4 o IPv6
    hints.ai_socktype = SOCK_STREAM; // Tipo de socket TCP
    hints.ai_flags = AI_PASSIVE; // Para uso en bind, para que pueda aceptar conexiones

    // Obtiene la informacion de la direccion y la guarda en servinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    bool conecto = false; 
    // Itera por cada addrinfo devuelto  
    for (p = servinfo; p != NULL; p = p->ai_next) {
        // Intenta crear un socket 
        socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_servidor == -1) {// Si falla la creacion del socket, continua con el siguiente addrinfo
            printf("Error creando el socket para %s:%s\n", ip, puerto);
            printf ("Falla el socket");
            continue;
        } 
        // Intenta hacer bind al socket
        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            // Si falla el bind, cierra el socket y continua con el siguiente addrinfo
            printf("Error haciendo bind para %s:%s\n", ip, puerto);
            printf ("Falla el bind \n");
            close(socket_servidor);
            continue; 
        }

        // Si se conecta, sale del bucle
        conecto = true;
        break;
    }

    // Si no se pudo conectar, libera la memoria y retorna 0
    if(!conecto) {
        free(servinfo);
        printf ("No se pudo conectar \n"); //Sacar cuando ande
        return 0;
    }

        
    // Comienza a escuchar conexiones en el socket
    listen(socket_servidor, SOMAXCONN);

    // Registra en el logger que el servidor está escuchando
    log_info(logger, "Escuchando en %s:%s: %s \n", ip, puerto, name);

    // Libera la memoria de servinfo
    freeaddrinfo(servinfo);

    // Retorna el descriptor del socket del servidor
    return socket_servidor;
}

// Funcion para esperar una conexion de cliente en un servidor abierto
int esperar_cliente(t_log* logger, const char* name, int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    // Acepta una conexion entrante
    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
   
    // Registra en el logger que un cliente se ha conectado
    log_info(logger, "Cliente conectado a: %s\n", name);

    // Retorna el descriptor del socket del cliente
    return socket_cliente;
}