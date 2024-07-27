#include "../include/main.h"

pthread_mutex_t mutex_conexion_io_mem = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_frames_status = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_user_space = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_procesos = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Falta ingresar el archivo de configuracion\n");
        return EXIT_FAILURE;
    }
  
	iniciar_config(argv[1]); 		
	iniciar_logger();  // Creacion de logger_memoria


	obtener_config(); // Obtener config_memoria

	loggear_config(); // Loggear config_memoria
	
	init_memoria(); // Inicializar Memoria
	
	// Definir Servidor
	mem_serv_socket = iniciar_servidor(logger_memoria_extra, "MEM_SERVER", IP_MEMORIA, PUERTO_ESCUCHA);
	log_info(logger_memoria_extra, "Servidor listo para recibir al cliente \n");

	//----------------------------Conexión Servidor con CPU y con IO-----------------------------//

	// Esperar Cliente y establecer conexión
	while(server_escuchar(mem_serv_socket, "MEM_SERVER"));	

	// Terminar Programa
	terminar_programa();

	return EXIT_SUCCESS;
}