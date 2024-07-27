#include "include/main.h"

pthread_mutex_t mutex_interrupcion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pid_exec = PTHREAD_MUTEX_INITIALIZER; 

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Falta ingresar el archivo de configuracion\n");
        return EXIT_FAILURE;
    }


    iniciar_config(argv[1]); 		
	iniciar_logger();  // Creacion de logger_cpu


	obtener_config(); // Obtener Config

	loggear_config(); // Loggear Config

	init_tlb(); // Inicializar TLB

	// ----------------------Creacion de Conexion Cliente con Memoria -------------------------

	// Creamos una conexion hacia el servidor
	conexion_memoria = crear_conexion_client(IP_MEMORIA, PUERTO_MEMORIA);
	if(conexion_memoria == -1) {
		log_error(logger_cpu_extra, "Error al crear conexion \n");
		terminar_programa();
		exit(EXIT_FAILURE);
	} else {
		log_info(logger_cpu_extra, "Conexion establecida con el servidor \n");
	}

	// ----------------------HANDSHAKE CON MEMORIA------------------------

	handshake_cliente_cpu(bytes, HANDSHAKE_CPU_MEMORIA, result);

	// ----------------------Creacion de Conexion Servidor con KERNEL------------------------

	// Definir Servidor Dispatch
	cpu_dispatch_socket = iniciar_servidor(logger_cpu_extra, "CPU_DISPATCH_SERVER", IP_CPU, PUERTO_ESCUCHA_DISPATCH);
	log_info(logger_cpu_extra, "Servidor listo para recibir al cliente en puerto DISPATCH");

	// Definir Servidor interrupt
	cpu_interrupt_socket = iniciar_servidor(logger_cpu_extra, "CPU_INTERRUPT_SERVER", IP_CPU, PUERTO_ESCUCHA_INTERRUPT);
	log_info(logger_cpu_extra, "Servidor listo para recibir al cliente en puerto INTERRUPT");

	// Manejo de interrupts
	manejo_interrupts("CPU_INTERRUPT_SERVER");

	// Establecer conexion dispatch
	while(server_escuchar("CPU_DISPATCH_SERVER"));

	terminar_programa();
	return EXIT_SUCCESS;
}