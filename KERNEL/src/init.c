#include "../include/init.h"

// ----------------------Variables Globales-------------------------
int io_socket;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int conexion_memoria;
char *IP_CPU;
char *IP_MEMORIA;
char *IP_KERNEL;
char *PUERTO_KERNEL;
char *PUERTO_MEMORIA;
char *PUERTO_ESCUCHA_DISPATCH;
char *PUERTO_ESCUCHA_INTERRUPT;
char *ALGORITMO_PLANIFICACION;
int QUANTUM;
char **RECURSOS;
char **INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION;
char *PATH_INSTRUCCIONES;

int32_t INTERRUPT = 1;
int32_t NOT_INTERRUPT = 0;
int32_t OK = 0;
int32_t ERROR = -1;
uint32_t BLOCKED = 1;
uint32_t INSTANCES = 0;
int32_t SEM_CONTADOR = 1;
int32_t SEM_BINARIO = 0;
bool WAIT = true;
bool SIGNAL = false;
bool LOCK = true;
bool UNLOCK = false;

handshake HANDSHAKE;
int32_t result;
size_t bytes;

t_dictionary* diccionario_colas;
t_dictionary* diccionario_sem;
t_dictionary* diccionario_mutex;

t_log *logger_kernel;
t_log *logger_kernel_extra;
t_config *config_kernel;

// Colas
t_list *cola;
t_list *cola_ready; // Ver que hacer con cola ready global
t_list *cola_new;
t_list *cola_aux_vrr;
t_list *interfaces_gen;
t_list *recursos;
t_list *pid_recs;
t_list *interfaces_stdin;
t_list *interfaces_stdout;
t_list *interfaces_dialfs;

// ----------------------logger_kernel-------------------------

void iniciar_logger()
{
	logger_kernel = log_create("kernel.log", "LOGS", true, LOG_LEVEL_INFO);
	logger_kernel_extra = log_create("kernel_extra.log", "LOGS_EXTRA", true, LOG_LEVEL_DEBUG);

	if (logger_kernel == NULL || logger_kernel_extra == NULL)
	{ // Error en la creacion, terminar programa
		exit(EXIT_FAILURE);
	}
}

// ----------------------Config-------------------------

void iniciar_config(char* config)
{

	config_kernel = config_create(config);

	if (config_kernel == NULL) { // Error en la creacion, terminar programa
		log_error(logger_kernel_extra, "No se pudo crear el config");
		exit(EXIT_FAILURE);
	}
}

void obtener_config()
{

	IP_MEMORIA = strdup(config_get_string_value(config_kernel, "IP_MEMORIA"));
	IP_CPU = strdup(config_get_string_value(config_kernel, "IP_CPU"));
	IP_KERNEL = strdup(config_get_string_value(config_kernel, "IP_KERNEL"));
	PUERTO_MEMORIA = strdup(config_get_string_value(config_kernel, "PUERTO_MEMORIA"));
	PUERTO_KERNEL = strdup(config_get_string_value(config_kernel, "PUERTO_KERNEL"));
	PUERTO_ESCUCHA_DISPATCH = strdup(config_get_string_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH"));
	PUERTO_ESCUCHA_INTERRUPT = strdup(config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT"));
	ALGORITMO_PLANIFICACION = strdup(config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION"));
	QUANTUM = config_get_int_value(config_kernel, "QUANTUM") * 1000;
	RECURSOS = config_get_array_value(config_kernel, "RECURSOS");
	INSTANCIAS_RECURSOS = config_get_array_value(config_kernel, "INSTANCIAS_RECURSOS");
	GRADO_MULTIPROGRAMACION = config_get_int_value(config_kernel, "GRADO_MULTIPROGRAMACION");
}

// Log: valores de la config
void loggear_config()
{
	log_info(logger_kernel_extra,
			 "IP Memoria: %s,\n"
			 "IP CPU: %s,\n"
			 "IP_KERNEL : %s,\n"
			 "Puerto Memoria: %s,\n"
			 "Puerto Kernel: %s,\n"
			 "Puerto CPU Dispatch: %s,\n"
			 "Puerto CPU Interrupt: %s,\n"
			 "Algoritmo Planificacion: %s,\n"
			 "Quantum: %d,\n"
			 "Recursos: %p,\n"
			 "Instancias Recursos: %p,\n"
			 "Grado Multiprogramacion: %d,\n",
			 IP_MEMORIA,
			 IP_CPU,
			 IP_KERNEL,
			 PUERTO_MEMORIA,
			 PUERTO_KERNEL,
			 PUERTO_ESCUCHA_DISPATCH,
			 PUERTO_ESCUCHA_INTERRUPT,
			 ALGORITMO_PLANIFICACION,
			 QUANTUM,
			 RECURSOS,
			 INSTANCIAS_RECURSOS,
			 GRADO_MULTIPROGRAMACION
			 );
}

void init_diccionario_colas()
{
	// Crear el diccionario de colas
	diccionario_colas = dictionary_create();
	if (diccionario_colas == NULL)
	{
		log_error(logger_kernel_extra, "No se pudo crear el diccionario de colas.");
		exit(EXIT_FAILURE);
	}
}

void init_dic_sem()
{
	diccionario_sem = dictionary_create();
	diccionario_mutex = dictionary_create();
	if (diccionario_sem == NULL || diccionario_mutex == NULL)
	{
		log_error(logger_kernel_extra, "No se pudo crear los diccionarios de semaforos.");
		exit(EXIT_FAILURE);
	}
}

void init_hilo_largo_plazo(pthread_t *hilo_planificacion)
{
	pthread_create(hilo_planificacion, NULL, (void *)planificador_largo_plazo, NULL);
	pthread_detach(*hilo_planificacion);
}

void init_hilo_server(pthread_t *hilo_server)
{
	t_procesar_conexion_args *server_args = malloc(sizeof(t_procesar_conexion_args));
	server_args->kernel_serv_name = "io_socket";

	pthread_create(hilo_server, NULL, (void *)server_escuchar, (void *)server_args);
	pthread_detach(*hilo_server);
}

void init_hilo_consola(pthread_t *hilo_consola)
{
	pthread_create(hilo_consola, NULL, (void *)consola, NULL);
	pthread_detach(*hilo_consola);
}

void init_hilo_motivos_desalojo(pthread_t *hilo_motivo_desalojo)
{
	pthread_create(hilo_motivo_desalojo, NULL, (void *)motivo_desalojo, NULL);
	pthread_detach(*hilo_motivo_desalojo);
}

void init_colas()
{
	cola_new = list_create();
	cola_ready = list_create();
	cola_aux_vrr = list_create();
	interfaces_gen = list_create();
	recursos = list_create();
	pid_recs = list_create();
	interfaces_stdin = list_create();
	interfaces_stdout = list_create();
	interfaces_dialfs = list_create();
}

// ---------------------- Inicialización de Semaforos -------------------------

void init_sem_planificacion()
{

	// PLANIFICACION --> MEMORIA
	sem_init(&sem_multiprogramacion, 0, GRADO_MULTIPROGRAMACION);
	sem_init(&sem_pcb_espera_ready, 0, 0);
	sem_init(&sem_corto_plazo, 0, 0);
	sem_init(&sem_exec, 0, 0);
	sem_init(&sem_Q, 0, 0);
	sem_init(&rr_exit, 0, 1);
	sem_init(&sem_tiempo_ejecutado, 0, 0);
	sem_init(&planificador_corto, 0, 1);
	sem_init(&planificador_largo, 0, 1);
}

// ----------------------Terminar Programa-------------------------

void terminar_programa_kernel()
{
	log_destroy(logger_kernel);
	log_destroy (logger_kernel_extra);
	config_destroy(config_kernel);
	close(conexion_cpu_dispatch);
	close(conexion_cpu_interrupt);
	close(conexion_memoria);

	list_destroy_and_destroy_elements(cola_new, (void*)&liberar_pcb);
	list_destroy_and_destroy_elements(cola_ready, (void*)&liberar_pcb);
	list_destroy_and_destroy_elements(cola_aux_vrr, (void*)&liberar_pcb);
	list_destroy_and_destroy_elements(interfaces_gen, (void*)&liberar_io_gen);
	list_destroy_and_destroy_elements(interfaces_stdin, (void*)&liberar_io_std);
	list_destroy_and_destroy_elements(interfaces_stdout, (void*)&liberar_io_std);
	list_destroy_and_destroy_elements(interfaces_dialfs, (void*)&liberar_io_dialfs);
	list_destroy_and_destroy_elements(recursos, (void*)&liberar_recurso);
	list_destroy_and_destroy_elements(pid_recs, (void*)&liberar_pid_recs);
	
	dictionary_destroy_and_destroy_elements(diccionario_sem, &destruir_sem);
	dictionary_destroy_and_destroy_elements(diccionario_mutex, &destruir_mutex);
	dictionary_destroy_and_destroy_elements(diccionario_colas, &destruir_lista_de_colas);

	// Destruir todos los semaforos
    sem_destroy(&sem_multiprogramacion);
    sem_destroy(&sem_pcb_espera_ready);
    sem_destroy(&sem_corto_plazo);
    sem_destroy(&sem_exec);
    sem_destroy(&sem_Q);
    sem_destroy(&rr_exit);
    sem_destroy(&sem_tiempo_ejecutado);
    sem_destroy(&planificador_corto);
    sem_destroy(&planificador_largo);
}

void destruir_sem(void* sem_ptr) {
    sem_t* semaphore = (sem_t*)sem_ptr;
    sem_destroy(semaphore);
    free(semaphore); 
}

void destruir_mutex(void* t_mutex) {
    pthread_mutex_t* mutex = (pthread_mutex_t*) t_mutex;
    pthread_mutex_destroy(mutex);
    free(mutex);
}

void liberar_io_gen(t_io_gen *io_gen)
{
	free(io_gen->nombre_interfaz);
	free(io_gen);
}

void liberar_io_std(t_io_std* io_std)
{
	free(io_std->nombre_interfaz);
	free(io_std);
}

void liberar_io_dialfs(t_io_df* io_dialfs)
{
	free(io_dialfs->nombre_interfaz);
	free(io_dialfs->path);
	free(io_dialfs);
}

void liberar_recurso(recurso_t *recurso)
{
	free(recurso->nombre);
	free(recurso);
}

void liberar_pid_recs(pid_rec *pid_rec)
{	
	list_destroy(pid_rec->recs_asoc); 
	free(pid_rec);
}

void liberar_diccionario_colas(t_list *lista)
{
	list_destroy_and_destroy_elements(lista, (void*)liberar_pcb);
}


void destruir_lista_de_colas(void* lista) {
    list_destroy_and_destroy_elements((t_list*)lista, (void*)liberar_pcb);
}

void liberar_int_a_enviar_fs(interfaz_a_enviar_fs* params) {
    free(params->nombre_interfaz);
    free(params->nombre_archivo);
    free(params);
}
