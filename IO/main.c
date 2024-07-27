#include "include/main.h"
pthread_mutex_t mutex_bitmap = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutex_block_count = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_fcbs = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_archivo_bloques = PTHREAD_MUTEX_INITIALIZER;
 

int main(int argc, char* argv[]) {
	
	if (argc < 3) {
        printf("Falta ingresar el nombre de la interfaz o el archivo de configuracion\n");
        return EXIT_FAILURE;
    }

	iniciar_logger();  // Creacion de logger_io

	// ---------------------- Conexion Cliente con Kernel -------------------------

	init_interfaz(argv[1], argv[2]); 

    terminar_programa();
	return EXIT_SUCCESS;
}