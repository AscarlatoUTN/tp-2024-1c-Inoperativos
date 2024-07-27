#include "../include/client_sockets.h"

// aca se definen todas las funciones declaradas en client_sockets.h, 
// es decir crear conexion y liberar conexion del lado del cliente

int crear_conexion_client(char *ip, char* puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // No importa si es IPv4 o IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // Rellenar mi IP automáticamente

    getaddrinfo(ip, puerto, &hints, &server_info);

    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    // Establecer la opción SO_REUSEADDR en el socket
    int yes = 1;
    if (setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("Setsockopt: ");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        perror("Error al conectar: ");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(server_info);

    return socket_cliente;
}