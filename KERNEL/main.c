#include "include/main.h"

pthread_t hilo_largo_plazo, hilo_server, hilo_consola, hilo_motivo_desalojo;
pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_dict_colas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_dict_sem = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_dict_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_gen = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_aux_vrr = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_interrupcion = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_tiempo_ejecutado = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_conexion_io_kernel = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_recursos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_pid_recs = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_stdin = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_stdout = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_dialfs = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pid = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_plani = PTHREAD_MUTEX_INITIALIZER;

sem_t sem_multiprogramacion;
sem_t sem_pcb_espera_ready;
sem_t sem_corto_plazo;
sem_t sem_exec;
sem_t sem_Q;
sem_t rr_exit;
sem_t sem_tiempo_ejecutado;
sem_t planificador_corto;
sem_t planificador_largo;

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Falta ingresar el archivo de configuracion\n");
        return EXIT_FAILURE;
    }

    iniciar_logger(); 				// Creacion de logger_kernel
    iniciar_config(argv[1]); 		// Creacion de Config
    obtener_config();				// Obtener Config
    loggear_config(); 				// Loggear Config

    init_colas(); 					// Creacion de colas de Planificacion, Manejo de Intefaces y Recursos
    init_sem_planificacion(); 		// Inicializacion de semaforos para todo el programa
    init_diccionario_colas(); 		// Creacion de Diccionaro que maneja colas varias
    init_dic_sem(); 	            // Creacion de Diccionarios que manejan semaforos y mutex varios

    init_recursos_sist(); 			// Inicialización de recursos del Sistema (Manejador de Recursos)
    
    // ---------------------- Creación de Conexiones Cliente CPU ---------------
    
    conexion_cpu_interrupt = establecer_conexion (IP_CPU, PUERTO_ESCUCHA_INTERRUPT);

    handshake_cliente_kernel(bytes, HANDSHAKE_KERNEL_CPU, conexion_cpu_interrupt, result);

    conexion_cpu_dispatch = establecer_conexion (IP_CPU, PUERTO_ESCUCHA_DISPATCH);
    
    handshake_cliente_kernel(bytes, HANDSHAKE_KERNEL_CPU, conexion_cpu_dispatch, result);

    // ---------------------- Creación de Conexiones Cliente MEMORIA ----------------------
    
    conexion_memoria = establecer_conexion (IP_MEMORIA, PUERTO_MEMORIA);
    
    handshake_cliente_kernel(bytes, HANDSHAKE_KERNEL_MEMORIA, conexion_memoria, result);

    // ---------------------- Creación de Conexiones Servidor IO ----------------------

    io_socket = iniciar_servidor(logger_kernel_extra, "io_socket", IP_KERNEL, PUERTO_KERNEL);
    log_info(logger_kernel_extra, "Servidor listo para recibir al cliente \n");

    // ---------------------- Creación de Conexiones CPU - IO ----------------------
    
    init_hilo_server(&hilo_server);

    // ---------------------- Creación de Hilos ----------------------

    init_hilo_largo_plazo(&hilo_largo_plazo);

    // ---------------------- Planificacion -------------------------

    init_hilo_motivos_desalojo(&hilo_motivo_desalojo);

    init_hilo_consola(&hilo_consola);

    planificador_corto_plazo();
    
    // ---------------------- Terminar Programa -------------------------
    terminar_programa_kernel();
    
    return EXIT_SUCCESS;
}
