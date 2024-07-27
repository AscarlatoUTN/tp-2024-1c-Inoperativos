#include "../include/init.h"

// ----------------------Variables Globales-------------------------
char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
int TAM_MEMORIA;
uint32_t TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;
t_log* logger_memoria;
t_log* logger_memoria_extra;
t_config* config_memoria;
t_list* procesos;
int mem_serv_socket;
int io_socket;
int kernel_socket;
int cpu_socket;

// ----------------------logger_memoria-------------------------
void iniciar_logger(){
	logger_memoria = log_create("memoria.log", "LOGS", true, LOG_LEVEL_INFO);
	logger_memoria_extra = log_create ("mem_extra.log", "LOGS_EXTRA", true, LOG_LEVEL_DEBUG);

	if (logger_memoria_extra == NULL || logger_memoria_extra == NULL) { // Error en la creacion, terminar programa
		log_error(logger_memoria_extra, "No se pudo crear el logger_memoria \n");
		exit(EXIT_FAILURE);
	}
}

// ----------------------config_memoria-------------------------
void iniciar_config(char* config){
	config_memoria = config_create(config);

	if (config_memoria == NULL) { // Error en la creacion, terminar programa
		log_error(logger_memoria_extra, "No se pudo crear el config_memoria \n");
		log_destroy(logger_memoria_extra);
		exit(EXIT_FAILURE);
	}
}

void obtener_config(){
	PUERTO_ESCUCHA = strdup(config_get_string_value(config_memoria, "PUERTO_ESCUCHA"));
	TAM_MEMORIA = config_get_int_value(config_memoria, "TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(config_memoria, "TAM_PAGINA");
	PATH_INSTRUCCIONES = strdup(config_get_string_value(config_memoria, "PATH_INSTRUCCIONES"));
	RETARDO_RESPUESTA = config_get_int_value(config_memoria, "RETARDO_RESPUESTA") * 1000;
	IP_MEMORIA = strdup(config_get_string_value(config_memoria, "IP_MEMORIA"));
}

// Log: Valores de la config_memoria
void loggear_config(){
	log_info(logger_memoria_extra, 
			"Puerto Escucha: %s,\n" 
			"Tamanio Memoria: %d,\n" 
			"Tamanio Pagina: %d,\n" 
			"Path Instrucciones: %s,\n" 
			"Retardo Respuesta: %d\n",
			PUERTO_ESCUCHA, 
			TAM_MEMORIA, 
			TAM_PAGINA, 
			PATH_INSTRUCCIONES, 
			RETARDO_RESPUESTA
		);
}

// ----------------------Terminar Programa-------------------------
void terminar_programa() {	
	if(procesos != NULL)
		list_destroy_and_destroy_elements(procesos, (void*)liberar_procesos);
	close(io_socket);
	close(kernel_socket);
	close(cpu_socket);
	bitarray_destroy(frames_status);
	free(bitmap);
	free(user_space);
	config_destroy(config_memoria);
	log_destroy(logger_memoria);
	log_destroy(logger_memoria_extra);
}

void liberar_procesos(t_proceso* proceso) {
	if(proceso->instrucciones != NULL || proceso->pageTable != NULL){
		list_destroy_and_destroy_elements(proceso->instrucciones, &free);
		list_destroy_and_destroy_elements(proceso->pageTable, &free);
	}
	free(proceso);
}