#include "../include/init.h"

// ----------------------Variables Globales-------------------------
int conexion_kernel;
int conexion_memoria;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;
t_log* logger_io;
t_log* logger_io_extra;
t_config* config_io;

// ----------------------logger_io_extra-------------------------
void iniciar_logger(){
	logger_io = log_create("io.log", "LOGS", true, LOG_LEVEL_INFO);
	logger_io_extra = log_create("io_extra.log", "LOGS", true, LOG_LEVEL_DEBUG);
	if (logger_io_extra == NULL || logger_io == NULL) { // Error en la creacion, terminar programa
		exit(EXIT_FAILURE);
	}
}

t_interfaz_MAP mapa_interfaz [] = {
	{T_IO_GEN, "IO_GEN"},
	{T_IO_STDIN, "IO_STDIN"},
	{T_IO_STDOUT, "IO_STDOUT"},
	{T_IO_DIALFS, "IO_DIALFS"},
	{0, NULL}
};

void init_interfaz(char* nombre_interfaz, char* config_interfaz){

	config_io = config_create(config_interfaz);
	
	t_init_interfaz* data_interfaz = malloc(sizeof(t_init_interfaz));
	data_interfaz->nombre = strdup(nombre_interfaz);

	char* tipo = strdup(config_get_string_value(config_io, "TIPO_INTERFAZ"));
	IP_KERNEL = strdup(config_get_string_value (config_io, "IP_KERNEL"));
	PUERTO_KERNEL = strdup(config_get_string_value (config_io, "PUERTO_KERNEL"));
	IP_MEMORIA = strdup(config_get_string_value (config_io, "IP_MEMORIA"));
	PUERTO_MEMORIA = strdup(config_get_string_value(config_io,"PUERTO_MEMORIA"));
	PATH_BASE_DIALFS = strdup(config_get_string_value(config_io, "PATH_BASE_DIALFS"));
	BLOCK_SIZE = config_get_int_value(config_io, "BLOCK_SIZE");
	BLOCK_COUNT = config_get_int_value(config_io, "BLOCK_COUNT");
	FREE_BLOCK_COUNT = BLOCK_COUNT;
	RETRASO_COMPACTACION = config_get_int_value(config_io, "RETRASO_COMPACTACION") * 1000;

	for(uint8_t i = 0; mapa_interfaz[i].tipo_interfaz != NULL; i++) {
        // Si encontramos la instrucción en el mapa
        if(strcmp(tipo, mapa_interfaz[i].tipo_interfaz) == 0) {
            data_interfaz->tipo = mapa_interfaz[i].tipo;
			break;
		}            
	}

	crear_conexion(data_interfaz, config_io);
	free(tipo);
	config_destroy(config_io);
}


//--------------- Inicializacion de Interfaces -------------------------
void crear_conexion(t_init_interfaz* data_interfaz, t_config* config_io){

    header tipo = data_interfaz->tipo;
    switch (tipo){
        case T_IO_GEN:
            t_io_gen* io_gen = crear_estructura_io_gen(config_io, data_interfaz);	
            handshake_cliente_io(bytes, HANDSHAKE_IO_GEN_KERNEL, io_gen->fd_interfaz, config_io, io_gen->nombre_interfaz,io_gen->estado);
			enviar_interfaz_gen(T_IO_GEN, io_gen->fd_interfaz, io_gen);	
            conexion_dispatch_io_gen("KERNEL_SERVER", io_gen);
            
            free(io_gen);
            
            break;
        case T_IO_STDIN:

			t_io_std* io_stdin = crear_estructura_io_std(config_io, data_interfaz);	
            handshake_cliente_io(bytes, HANDSHAKE_IO_STDIN_KERNEL, io_stdin->fd_io_kernel, config_io, io_stdin->nombre_interfaz, io_stdin->estado);
			enviar_interfaz_std(T_IO_STDIN, io_stdin->fd_io_kernel, io_stdin);
			handshake_cliente_io(bytes, HANDSHAKE_IO_STDIN_MEMORIA, io_stdin->fd_io_mem, config_io, io_stdin->nombre_interfaz, io_stdin->estado);
			conexion_cliente_stdin(io_stdin);

			free(io_stdin);
			
            break;
        case T_IO_STDOUT:
			t_io_std* io_stdout = crear_estructura_io_std(config_io, data_interfaz);	
			handshake_cliente_io(bytes, HANDSHAKE_IO_STDOUT_KERNEL, io_stdout->fd_io_kernel, config_io, io_stdout->nombre_interfaz, io_stdout->estado);
			enviar_interfaz_std(T_IO_STDOUT, io_stdout->fd_io_kernel, io_stdout);
			handshake_cliente_io(bytes, HANDSHAKE_IO_STDOUT_MEMORIA, io_stdout->fd_io_mem, config_io, io_stdout->nombre_interfaz,io_stdout->estado);
			conexion_cliente_stdout(io_stdout);

			free(io_stdout);
			
			break;
        case T_IO_DIALFS:
			t_io_df* io_dialfs = crear_estructura_io_dialfs(config_io, data_interfaz);
			handshake_cliente_io(bytes, HANDSHAKE_IO_DIALFS_KERNEL, io_dialfs->fd_io_kernel, config_io, io_dialfs->nombre_interfaz, io_dialfs->estado);
			enviar_interfaz_dialfs(T_IO_DIALFS, io_dialfs->fd_io_kernel, io_dialfs);
			handshake_cliente_io(bytes, HANDSHAKE_IO_DIALFS_MEMORIA, io_dialfs->fd_io_mem, config_io, io_dialfs->nombre_interfaz, io_dialfs->estado);
			conexion_cliente_dialfs(io_dialfs);

			free(io_dialfs);
			break;
			
        default:
			log_error(logger_io_extra, "No existe el tipo de operacion recibido : %d \n" , tipo);
            break;
    }

	free(data_interfaz->nombre);
}

// ----------------------------------------- GENERICA ----------------------------------------------------
t_io_gen* crear_estructura_io_gen (t_config* config, t_init_interfaz* data_interfaz){
	
	t_io_gen* interfaz_io_gen = malloc(sizeof(t_io_gen));
	interfaz_io_gen->nombre_interfaz = data_interfaz->nombre;
	interfaz_io_gen->length_nombre_interfaz = strlen(data_interfaz->nombre) + 1;
	interfaz_io_gen->tipo_interfaz = data_interfaz->tipo;
	interfaz_io_gen->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO"); 
	interfaz_io_gen->fd_interfaz = crear_conexion_client(IP_KERNEL,PUERTO_KERNEL);

	return interfaz_io_gen;
}


// ----------------------- STD ------------------------------
t_io_std* crear_estructura_io_std(t_config* config_io, t_init_interfaz* data_interfaz){
	
	t_io_std* interfaz_io_stdin = malloc(sizeof(t_io_std));
	interfaz_io_stdin->nombre_interfaz = data_interfaz->nombre;
	interfaz_io_stdin->length_nombre_interfaz = strlen(data_interfaz->nombre) + 1;
	interfaz_io_stdin->tipo_interfaz = data_interfaz->tipo;
	interfaz_io_stdin->fd_io_kernel = crear_conexion_client(IP_KERNEL,PUERTO_KERNEL);
	interfaz_io_stdin->fd_io_mem = crear_conexion_client(IP_MEMORIA,PUERTO_MEMORIA);

	return interfaz_io_stdin;
}

//-----------------------DIALFS------------------------------------- 
t_io_df* crear_estructura_io_dialfs(t_config* config, t_init_interfaz* data_interfaz){

	t_io_df* interfaz_io_dialfs = malloc(sizeof(t_io_df));
	interfaz_io_dialfs->nombre_interfaz = data_interfaz->nombre;
	interfaz_io_dialfs->length_nombre_interfaz = strlen(data_interfaz->nombre) + 1;
	interfaz_io_dialfs->tipo_interfaz = data_interfaz->tipo;
	interfaz_io_dialfs->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
	interfaz_io_dialfs->retraso_compactacion = RETRASO_COMPACTACION;
	interfaz_io_dialfs->fd_io_kernel = crear_conexion_client(IP_KERNEL,PUERTO_KERNEL);
	interfaz_io_dialfs->fd_io_mem = crear_conexion_client(IP_MEMORIA,PUERTO_MEMORIA);
	interfaz_io_dialfs->block_size = BLOCK_SIZE;
	interfaz_io_dialfs->block_count = BLOCK_COUNT;
	interfaz_io_dialfs->length_path = strlen(PATH_BASE_DIALFS) + 1;
	interfaz_io_dialfs->path = strdup(PATH_BASE_DIALFS);
	
	return interfaz_io_dialfs;
}

// ----------------------Terminar Programa-------------------------
void terminar_programa(){	
	log_destroy(logger_io_extra);
	close(conexion_memoria);
	close(conexion_kernel);
	// Verificar y destruir lista solo si se ha creado
    if (lista_fcbs != NULL)
        list_destroy_and_destroy_elements(lista_fcbs, (void*)liberar_fcb);
}