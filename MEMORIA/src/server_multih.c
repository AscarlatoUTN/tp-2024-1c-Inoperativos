#include "../include/server_multih.h"

handshake HANDSHAKE;
int result;
int tamanio;
int tamanio_pagina;
int OK = 0;
int ERROR = -1;
int codigo_instruccion;
int RECIBIDO = 0;
int RECHAZADO = -1;
int32_t ped_tam_pag = 0;
// Función para escuchar del servidor
int server_escuchar(int socket_servidor, char* mem_serv_name){
    // Esperar a que el cliente se conecte
    int cliente_socket = esperar_cliente(logger_memoria_extra, mem_serv_name, socket_servidor);
    
    // Si el cliente se conecta correctamente, crear un nuevo hilo para procesar la conexión
    if (cliente_socket!=-1){
        pthread_t thread;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args-> mem_serv_socket = cliente_socket;
        args->mem_serv_name = mem_serv_name;
        
        recv(cliente_socket, &HANDSHAKE, sizeof(handshake), MSG_WAITALL);

        if (HANDSHAKE == HANDSHAKE_CPU_MEMORIA){
            log_info(logger_memoria_extra, "Handshake CPU->Memoria OK \n");
            send(cliente_socket, &OK, sizeof(int32_t), 0);
            recv(cliente_socket, &tamanio_pagina, sizeof(uint32_t), MSG_WAITALL);
            send(cliente_socket, &TAM_PAGINA, sizeof(uint32_t), 0);
            pthread_create(&thread,NULL,(void*) procesar_conexion_cpu,(void*) args); 
            pthread_detach(thread);
        }
        else if (HANDSHAKE == HANDSHAKE_KERNEL_MEMORIA)
        {
            log_info(logger_memoria_extra, "Handshake Kernel->Memoria OK \n");
            send(args->mem_serv_socket, &OK, sizeof(int32_t), 0);
            int path;
            recv(args->mem_serv_socket, &path, sizeof(int32_t), MSG_WAITALL);
            enviar_mensaje(PATH_INSTRUCCIONES, args->mem_serv_socket, HANDSHAKE_KERNEL_MEMORIA);
            pthread_create(&thread, NULL, (void *)procesar_conexion_kernel, (void *)args);
            pthread_detach(thread);
        }

        else if (HANDSHAKE == HANDSHAKE_IO_STDIN_MEMORIA)
        {
            log_info(logger_memoria_extra, "Handshake IO STDIN->Memoria OK \n");
            send(cliente_socket, &OK, sizeof(int32_t), 0);
        
            // Enviar a IO el tamanio de pagina
            recv(cliente_socket, &ped_tam_pag, sizeof(int32_t), MSG_WAITALL);
            send(cliente_socket, &TAM_PAGINA, sizeof(uint32_t), 0);
            
            pthread_create(&thread,NULL,(void*) procesar_conexion_io,(void*) args); 
            pthread_detach(thread);
        }
        else if (HANDSHAKE == HANDSHAKE_IO_STDOUT_MEMORIA)
        {
            log_info(logger_memoria_extra, "Handshake IO_STDOUT->Memoria OK \n");
            send(cliente_socket, &OK, sizeof(int32_t), 0);

            // Enviar a IO el tamanio de pagina
            recv(cliente_socket, &ped_tam_pag, sizeof(int32_t), MSG_WAITALL);
            send(cliente_socket, &TAM_PAGINA, sizeof(uint32_t), 0);
            
            pthread_create(&thread,NULL,(void*) procesar_conexion_io,(void*) args); 
            pthread_detach(thread);
        }
        else if (HANDSHAKE == HANDSHAKE_IO_DIALFS_MEMORIA)
        {
            log_info(logger_memoria_extra, "Handshake Kernel->Memoria OK \n");
            send(cliente_socket, &OK, sizeof(int32_t), 0);
            
            // Enviar a IO el tamanio de pagina
            recv(cliente_socket, &ped_tam_pag, sizeof(int32_t), MSG_WAITALL);
            send(cliente_socket, &TAM_PAGINA, sizeof(uint32_t), 0);
            
            pthread_create(&thread,NULL,(void*) procesar_conexion_io,(void*) args);  
            pthread_detach(thread);
        }
        else
        {
            log_error(logger_memoria_extra, "Handshake inexistente \n");
            send(cliente_socket, &ERROR, sizeof(int32_t), 0);
            close(cliente_socket);
        }

        return 1;
    }
    return 0;
}