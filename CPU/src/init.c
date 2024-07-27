#include "../include/init.h"

// ----------------------Variables Globales-------------------------
int OK = 0;
int ERROR = -1;
int conexion_memoria;
int conexion_kernel_dispatch;
int conexion_kernel_interrupt;
int cpu_dispatch_socket;
int cpu_interrupt_socket;
int hubo_interrupcion = 0;
int INTERRUPT = 1;
int NOT_INTERRUPT = 0;
char* IP_CPU; //Lo agregamos, pero no está en la consigna. Ver después.
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;
int result;

uint32_t tam_pagina;
size_t bytes;
t_log* logger_cpu;
t_log* logger_cpu_extra;
t_config* config_cpu;	

// -----------logger_cpu_extra-------------------------

void iniciar_logger(){
	logger_cpu = log_create("cpu.log", "LOGS", true, LOG_LEVEL_INFO);
	logger_cpu_extra = log_create("cpu_extra.log", "LOGS", true, LOG_LEVEL_DEBUG);

	if (logger_cpu_extra == NULL) { // Error en la creacion, terminar programa
		log_error(logger_cpu_extra, "No se pudo crear logger_cpu_extra \n");
		exit(EXIT_FAILURE);
	}
}

// ----------------------Config-------------------------
void iniciar_config(char* config){
	config_cpu = config_create(config);

	if (config_cpu == NULL) { // Error en la creacion, terminar programa
		log_error(logger_cpu_extra, "No se pudo crear el config \n");
		exit(EXIT_FAILURE);
	}
}

void obtener_config(){
    IP_CPU = strdup(config_get_string_value(config_cpu, "IP_CPU"));
    IP_MEMORIA = strdup(config_get_string_value(config_cpu, "IP_MEMORIA"));
    PUERTO_MEMORIA = strdup(config_get_string_value(config_cpu, "PUERTO_MEMORIA"));
    PUERTO_ESCUCHA_DISPATCH = strdup(config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH"));
    PUERTO_ESCUCHA_INTERRUPT = strdup(config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT"));
    CANTIDAD_ENTRADAS_TLB = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    ALGORITMO_TLB = strdup(config_get_string_value(config_cpu, "ALGORITMO_TLB"));
}

void loggear_config(){
	log_info(logger_cpu_extra,
			"IP Memoria: %s,\n"
            "Puerto Memoria: %s,\n"
            "Puerto Escucha Dispatch: %s,\n"
            "Puerto Escucha Interrupt: %s,\n"
            "Cantidad Entradas TLB: %d,\n"
            "Algoritmo TLB: %s\n",
            IP_MEMORIA, 
            PUERTO_MEMORIA, 
            PUERTO_ESCUCHA_DISPATCH, 
            PUERTO_ESCUCHA_INTERRUPT, 
            CANTIDAD_ENTRADAS_TLB, 
            ALGORITMO_TLB
	);
}

// ----------------------Handshake-------------------------------
void handshake_cliente_cpu(size_t bytes, handshake handshake, int result) {
	
	bytes = send(conexion_memoria, &handshake, sizeof(handshake), 0);
	bytes = recv(conexion_memoria, &result, sizeof(int32_t), MSG_WAITALL);
	
	tam_pagina = 0;
	if (result == 0) {
		log_info(logger_cpu_extra, "Handshake Memoria->CPU OK \n");
		send(conexion_memoria, &tam_pagina, sizeof(uint32_t), 0);
		recv(conexion_memoria, &tam_pagina, sizeof(uint32_t), MSG_WAITALL);
	} else {
		log_error (logger_cpu_extra, "Handshake Memoria->CPU ERROR \n");
		terminar_programa();
		exit(EXIT_FAILURE);
	}
}

// ----------------------Terminar Programa-------------------------
void terminar_programa(){
	log_destroy(logger_cpu);
	log_destroy(logger_cpu_extra);
	config_destroy(config_cpu);
	close(conexion_memoria);
	close(cpu_dispatch_socket);
	close(cpu_interrupt_socket);
}