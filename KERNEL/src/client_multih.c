#include "../include/client_multih.h"

// Hacer handshake
void handshake_cliente_kernel(size_t bytes, handshake handshake, int conexion, int result) {
	
    bytes = send(conexion, &handshake, sizeof(int32_t), 0);
    if (bytes == -1) {
        log_error(logger_kernel_extra, "Error al enviar handshake al servidor. \n");
        terminar_programa_kernel(conexion);
        exit(EXIT_FAILURE);
    }

	bytes = recv(conexion, &result, sizeof(handshake), MSG_WAITALL);
    if (bytes == -1) {
        log_error(logger_kernel_extra, "Error al recibir respuesta del servidor. \n");
        terminar_programa_kernel(conexion);
        exit(EXIT_FAILURE);
    }

	if (result == 0 && handshake == HANDSHAKE_KERNEL_CPU) {
		log_info(logger_kernel_extra, "Handshake CPU->Kernel OK \n");
	} else if (result == 0 && handshake == HANDSHAKE_KERNEL_MEMORIA) {
        log_info(logger_kernel_extra, "Handshake Memoria->Kernel OK \n");
		send(conexion, &OK, sizeof(int32_t), 0);
		PATH_INSTRUCCIONES = guardar_mensaje(conexion);
    } else {
		log_error (logger_kernel_extra, "Handshake cliente ERROR \n");
		terminar_programa_kernel(conexion);
		exit(EXIT_FAILURE);
	}
}

// Establecer conexion
int establecer_conexion(char* IP, char* PUERTO){
    int conexion = crear_conexion_client(IP, PUERTO);
	if(conexion == -1) {
		log_error(logger_kernel_extra, "Error al crear conexion \n");
		terminar_programa_kernel(conexion);
		exit(EXIT_FAILURE);
	} else {
		log_info(logger_kernel_extra, "Conexion establecida con el servidor \n");
	}
	return conexion;
}