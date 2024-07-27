#include "../include/server_multih.h"

uint8_t SEM_RECURSO = 0;
uint8_t SEM_INTERFAZ = 1;
int STDIN = 1;
int STDOUT = 0;

// ---------------------------- Funciones que init interfaces --------------------------
void manejar_init_gen(int conexion_interfaz, char* kernel_serv_name) {
    recibir_operacion(conexion_interfaz); // INUTIL, pero para no cambiar mas cosas !
    t_io_gen* interfaz = recibir_interfaz_gen(conexion_interfaz);
    interfaz->fd_interfaz = conexion_interfaz;
    inicializar_interfaz(interfaz, interfaz->nombre_interfaz, &mutex_lista_gen, interfaces_gen, conexion_interfaz);
    crear_hilo_interfaz(interfaz->fd_interfaz, interfaz->nombre_interfaz, interfaz->tipo_interfaz);
}

void manejar_init_std(int conexion_interfaz, char* kernel_serv_name, int caso) {
    recibir_operacion(conexion_interfaz); // INUTIL, pero para no cambiar mas cosas !
    t_io_std* interfaz = recibir_io_std(conexion_interfaz);
    interfaz->fd_io_kernel = conexion_interfaz;

    if(caso == STDIN)
        inicializar_interfaz(interfaz, interfaz->nombre_interfaz, &mutex_lista_stdin, interfaces_stdin, conexion_interfaz);
    else if (caso == STDOUT)
        inicializar_interfaz(interfaz, interfaz->nombre_interfaz, &mutex_lista_stdout, interfaces_stdout, conexion_interfaz);

    crear_hilo_interfaz(interfaz->fd_io_kernel, interfaz->nombre_interfaz, interfaz->tipo_interfaz);
}

void manejar_init_dialfs(int conexion_interfaz, char* kernel_serv_name) {
    recibir_operacion(conexion_interfaz); // INUTIL, pero para no cambiar mas cosas !
    t_io_df* interfaz = recibir_interfaz_dialfs(conexion_interfaz);
    interfaz->fd_io_kernel = conexion_interfaz;
    inicializar_interfaz(interfaz, interfaz->nombre_interfaz, &mutex_lista_dialfs, interfaces_dialfs, conexion_interfaz);
    crear_hilo_interfaz(interfaz->fd_io_kernel, interfaz->nombre_interfaz, interfaz->tipo_interfaz);
}

// Función principal para el hilo de espera
void* esperar_fin_io(void* void_args) {
    t_hilo_interfaz* hilo_interfaz = (t_hilo_interfaz*)void_args;
    int io_kernel_socket = hilo_interfaz->conexion_io_kernel;
    char* nombre_int = strdup(hilo_interfaz->nombre_interfaz);
    header tipo = hilo_interfaz->tipo_interfaz;
    free(hilo_interfaz);

    while (1) {
        if (procesar_operacion(io_kernel_socket, nombre_int, tipo) != 0) {
            free(nombre_int);
            pthread_exit(NULL);
        }
    }
}

// Procesar operación recibida
int procesar_operacion(int io_kernel_socket, char* nombre_int, header tipo) {
    int codigo_instruccion = recibir_operacion(io_kernel_socket);
    char* nombre_interfaz = guardar_mensaje_fin_io(io_kernel_socket);

    if(nombre_interfaz == NULL){
        eliminar_interfaz(nombre_int, tipo, io_kernel_socket);
        log_error(logger_kernel_extra, "Interfaz %s desconectada. \n", nombre_int);
        free(nombre_interfaz);
        return -1;
    }

    if (strcmp(nombre_interfaz, nombre_int) != 0) {
        eliminar_interfaz(nombre_int, tipo, io_kernel_socket);
        log_error(logger_kernel_extra, "Interfaz %s desconectada o inválida.\n", nombre_int);
        free(nombre_interfaz);
        return -1;
    }

    log_info(logger_kernel_extra, "Fin de Operación - Interfaz %s.\n", nombre_interfaz);

    t_pcb* PCB = sacar_pcb_cola_block(nombre_interfaz);
    int exit_status = manejar_codigo_instruccion(codigo_instruccion, PCB, nombre_interfaz);

    free(nombre_interfaz);
    return exit_status;
}

// Manejar código de instrucción recibido
int manejar_codigo_instruccion(int codigo_instruccion, t_pcb* PCB, char* nombre_interfaz) {
    if (codigo_instruccion == M_EXIT) {
        manejar_exit(PCB);
        return 0;
    }

    if (PCB != NULL) {
        if (strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
            manejar_vrr(PCB, cola_aux_vrr, &mutex_cola_aux_vrr);
        } else {
            ingresar_a_ready(NULL, PCB);
        }
        sem_post(&sem_corto_plazo);
    }

    op_sem_block(nombre_interfaz, SIGNAL, SEM_BINARIO, BLOCKED);
    return 0;
}

void eliminar_interfaz(char* nombre_int, header tipo_int, int io_socket) {
    t_list* cola_block = get_cola_blocked(nombre_int);

    if (cola_block == NULL) {
        log_error(logger_kernel_extra, "La cola de '%s' no existe en el diccionario. \n", nombre_int);
        return;
    }

    for (int i = 0; i < list_size(cola_block); i++) {
        t_pcb* PCB = list_get(cola_block, i);
        manejar_exit(PCB);
    }

    //list_destroy(cola_block);
    dictionary_remove_and_destroy(diccionario_colas, nombre_int, (void*)list_destroy);

    pthread_mutex_t* mutex_lista;
    t_list* lista_interfaz;
    void (*liberar_funcion)(void*);
    int (*comparar_funcion)(void*, char*);

    if (tipo_int == T_IO_GEN) {
        mutex_lista = &mutex_lista_gen;
        lista_interfaz = interfaces_gen;
        liberar_funcion = (void (*)(void*)) liberar_io_gen;
        comparar_funcion = (int (*)(void*, char*)) comparar_io_gen;
    } else if (tipo_int == T_IO_STDIN) {
        mutex_lista = &mutex_lista_stdin;
        lista_interfaz = interfaces_stdin;
        liberar_funcion = (void (*)(void*)) liberar_io_std;
        comparar_funcion = (int (*)(void*, char*)) comparar_io_std;
    } else if (tipo_int == T_IO_STDOUT) {
        mutex_lista = &mutex_lista_stdout;
        lista_interfaz = interfaces_stdout;
        liberar_funcion = (void (*)(void*)) liberar_io_std;
        comparar_funcion = (int (*)(void*, char*)) comparar_io_std;
    } else if (tipo_int == T_IO_DIALFS) {
        mutex_lista = &mutex_lista_dialfs;
        lista_interfaz = interfaces_dialfs;
        liberar_funcion = (void (*)(void*)) liberar_io_dialfs;
        comparar_funcion = (int (*)(void*, char*)) comparar_io_dialfs;
    } else {
        log_error(logger_kernel_extra, "Error: Tipo de interfaz inválido. \n");
        return;
    }

    pthread_mutex_lock(mutex_lista);
    for (int i = 0; i < list_size(lista_interfaz); i++) {
        void* interfaz = list_get(lista_interfaz, i);
        if (comparar_funcion(interfaz, nombre_int) == 0) {
            list_remove_and_destroy_element(lista_interfaz, i, liberar_funcion);
            break;
        }
    }
    pthread_mutex_unlock(mutex_lista);

    close(io_socket);
}

int comparar_io_gen(void* interfaz, char* nombre_int) {
    return strcmp(((t_io_gen*)interfaz)->nombre_interfaz, nombre_int);
}

int comparar_io_std(void* interfaz, char* nombre_int) {
    return strcmp(((t_io_std*)interfaz)->nombre_interfaz, nombre_int);
}

int comparar_io_dialfs(void* interfaz, char* nombre_int) {
    return strcmp(((t_io_df*)interfaz)->nombre_interfaz, nombre_int);
}

// ----------------------------- Funciones Auxiliares de Inicializacion -------------------------------

void inicializar_interfaz(void* interfaz, char* nombre_interfaz, pthread_mutex_t* mutex_lista, t_list* lista_interfaces, int conexion_interfaz) {
    log_info(logger_kernel_extra, "%s inicializada. \n", nombre_interfaz);

    // Inicializar una cola bloqueada y su mutex asociado
    init_cola_blocked(nombre_interfaz);
    init_semaforo_block(nombre_interfaz, SEM_INTERFAZ);

    // Agregar la interfaz a la lista de interfaces
    pthread_mutex_lock(mutex_lista);
    list_add(lista_interfaces, interfaz);
    pthread_mutex_unlock(mutex_lista);

    enviar_confirmacion("Interfaz inicializada correctamente \n", conexion_interfaz);
}

void enviar_confirmacion(char* mensaje, int socket) {
    pthread_mutex_lock(&mutex_conexion_io_kernel);
    enviar_mensaje(mensaje, socket, CONFIRMACION);
    pthread_mutex_unlock(&mutex_conexion_io_kernel);
}

void cerrar_conexion(int conexion_interfaz, char* kernel_serv_name) {
    log_warning(logger_kernel_extra, "El cliente se desconecto de %s \n", kernel_serv_name);
    close(conexion_interfaz);
    pthread_exit(NULL);
}

void crear_hilo_interfaz(int conexion_io_kernel, char* nombre_interfaz, header tipo) {
    pthread_t hilo_nombre_interfaz;
    t_hilo_interfaz* hilo_interfaz = malloc(sizeof(t_hilo_interfaz));
    hilo_interfaz->conexion_io_kernel = conexion_io_kernel;
    hilo_interfaz->nombre_interfaz = nombre_interfaz;
    hilo_interfaz->tipo_interfaz = tipo;

    pthread_create(&hilo_nombre_interfaz, NULL, (void*)esperar_fin_io, (void*)hilo_interfaz);
    pthread_detach(hilo_nombre_interfaz);
}

// --------------------------- Inicializar Elementos para Interfaces --------------------------

// Función para inicializar una cola bloqueada y su mutex asociado en el diccionario
void init_cola_blocked(char* nombre_interfaz) {

    t_list* list = list_create();
    if (list == NULL) {
        log_error(logger_kernel_extra, "No se pudo crear la lista. \n");
        return;
    }

    // Anclar la cola creada con el nombre de la interfaz
    pthread_mutex_lock(&mutex_dict_colas);
    dictionary_put(diccionario_colas, nombre_interfaz, list);
    pthread_mutex_unlock(&mutex_dict_colas);

    // Inicializar el mutex asociado a la cola
    init_mutex(nombre_interfaz, BLOCKED);
}

// Función para inicializar el mutex asociado a una cola bloqueada
void init_mutex(char* nombre_interfaz, int32_t caso) {
    
    char* nombre_mutex = generar_nombre_mutex(caso, nombre_interfaz);
    if (nombre_mutex == NULL) {
        log_error(logger_kernel_extra, "Error al generar el nombre del mutex. \n");
        free(nombre_mutex);
        return;
    }
    
    // Crear y anclar el mutex al diccionario
    pthread_mutex_t* mutex = inicializar_mutex();
    if (mutex == NULL) {
        free(nombre_mutex);
        return;
    }

    pthread_mutex_lock(&mutex_dict_mutex);
    dictionary_put(diccionario_mutex, nombre_mutex, mutex);
    pthread_mutex_unlock(&mutex_dict_mutex);
    free(nombre_mutex);
}

pthread_mutex_t* inicializar_mutex() {
    pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) {
        log_error(logger_kernel_extra, "No se pudo asignar memoria para el mutex. \n");
        return NULL;
    }

    if (pthread_mutex_init(mutex, NULL) != 0) {
        log_error(logger_kernel_extra, "No se pudo inicializar el mutex. \n");
        free(mutex);
        return NULL;
    }

    return mutex;
}


char* generar_nombre_mutex(int32_t caso, char* nombre_interfaz) {
    char* prefijo = (caso == 1) ? "mutex_block_" : "mutex_instancias_";
    return construir_nombre_sem(prefijo, nombre_interfaz);
}

void init_semaforo_block(char* nombre_recurso, uint8_t caso) {
    // Construir el nombre del semáforo asociado a la cola
    char* nombre_semaforo = construir_nombre_sem("sem_block_", nombre_recurso);

    if (nombre_semaforo == NULL) {
        log_error(logger_kernel_extra, "Error: No se pudo construir el nombre del semáforo. \n");
        free(nombre_semaforo);
        return;
    }

    // Crear y anclar el semáforo al diccionario
    sem_t* semaforo = malloc(sizeof(sem_t));
    if (semaforo == NULL) {
        log_error(logger_kernel_extra, "No se pudo asignar memoria para el semáforo. \n");
        free(nombre_semaforo);
        return;
    }

    // Inicializar el semáforo binario con valor caso
    if (sem_init(semaforo, 0, caso) != 0) {
        log_error(logger_kernel_extra, "No se pudo inicializar el semáforo. \n");
        free(semaforo);
        free(nombre_semaforo);
        return;
    }

    // pthread_mutex_lock(&mutex_dict_sem);
    dictionary_put(diccionario_sem, nombre_semaforo, semaforo);
    // pthread_mutex_unlock(&mutex_dict_sem);

    // Liberar el nombre del semáforo ya que está copiado en el diccionario
    free(nombre_semaforo);
}

char* construir_nombre_sem(char* prefijo, char* nombre_interfaz) {
    char* nombre_sem = malloc(strlen(prefijo) + strlen(nombre_interfaz) + 1);
    if (nombre_sem == NULL) {
        log_error(logger_kernel_extra, "Error: No se pudo asignar memoria para construir el nombre del semáforo. \n");
        return NULL;
    }

    strcpy(nombre_sem, prefijo);
    strcat(nombre_sem, nombre_interfaz); 
    nombre_sem[strlen(prefijo) + strlen(nombre_interfaz)] = '\0'; // Añadir el carácter centinela

    return nombre_sem;
}

void* op_sem_block(char* nombre, bool esperar, int tipo_semaforo, int caso) {
    char* nombre_semaforo = obtener_nombre_semaforo(nombre, caso);
    if (nombre_semaforo == NULL) {
        log_warning(logger_kernel_extra, "Advertencia: No se pudo obtener el nombre del semáforo para operación en '%s'. \n", nombre);   
        free(nombre_semaforo);
        return NULL;
    }

    sem_t* semaforo = obtener_semaforo(nombre_semaforo);
    if (semaforo == NULL) {
        log_error(logger_kernel_extra, "Error: No se pudo obtener el semáforo del diccionario para operación en '%s'. \n", nombre);
        free(nombre_semaforo);
        return NULL;
    }

    if (esperar) {
        sem_wait(semaforo);
        log_info(logger_kernel_extra, "Operación de espera completada en el semáforo '%s'. \n", nombre_semaforo); 
    } else {
        signal_semaforo(semaforo, tipo_semaforo);
    }    
    
    free(nombre_semaforo);
    return semaforo; // Puede ir cualquier cosa, pero tiene q retornar algo
}

char* obtener_nombre_semaforo(char* nombre, int caso) {
    char* prefijo = (caso == BLOCKED) ? "sem_block_" : "sem_instancias_";
    
    return construir_nombre_sem(prefijo, nombre);
}

sem_t* obtener_semaforo(char* nombre_semaforo) {
    sem_t* semaforo = dictionary_get(diccionario_sem, nombre_semaforo);
    if (semaforo == NULL) {
        log_error(logger_kernel_extra, "Error: No se pudo encontrar el semáforo en el diccionario para '%s'. \n", nombre_semaforo);
        return NULL;
    }
    return semaforo;
}

void signal_semaforo(sem_t* semaforo, int tipo_semaforo) {
    if (tipo_semaforo == SEM_BINARIO) {
        signal_bin(semaforo);
    } else if (tipo_semaforo == SEM_CONTADOR) {
        sem_post(semaforo);
    }
}

// ---------------------------- Funcion para escuchar del servidor --------------------------

int server_escuchar(void* void_args){
    
    t_procesar_conexion_args* args = (t_procesar_conexion_args*)void_args;
    char* kernel_serv_name = args->kernel_serv_name;

    while(true){
        
        // Esperar a que el cliente se conecte
        int conexion_interfaz = esperar_cliente(logger_kernel_extra, kernel_serv_name, io_socket);

        // Si el cliente se conecta correctamente, crear un nuevo hilo para procesar la conexión
        if (conexion_interfaz!=-1){

            recv(conexion_interfaz, &HANDSHAKE, sizeof(handshake), MSG_WAITALL);
            
            if (HANDSHAKE == HANDSHAKE_IO_GEN_KERNEL){
                log_info(logger_kernel_extra, "Handshake IO GEN -> Kernel OK \n");
                result = OK;
                send(conexion_interfaz, &result, sizeof(int32_t), 0);
                manejar_init_gen(conexion_interfaz,kernel_serv_name);
               continue;
            }   
            if (HANDSHAKE == HANDSHAKE_IO_STDIN_KERNEL || HANDSHAKE == HANDSHAKE_IO_STDOUT_KERNEL){
                log_info(logger_kernel_extra, "Handshake IO STD ->Kernel OK \n");
                result = OK;
                send(conexion_interfaz, &result, sizeof(int32_t), 0);
                int caso = HANDSHAKE == HANDSHAKE_IO_STDIN_KERNEL ? STDIN : STDOUT;
                manejar_init_std(conexion_interfaz, kernel_serv_name, caso);
                continue;
            }
            if (HANDSHAKE == HANDSHAKE_IO_DIALFS_KERNEL){
                log_info(logger_kernel_extra, "Handshake IO DIAL FS->Kernel OK \n");
                result = OK;
                send(conexion_interfaz, &result, sizeof(int32_t), 0);
                manejar_init_dialfs(conexion_interfaz, kernel_serv_name);                
                continue;

            } else {
                log_error(logger_kernel_extra, "Handshake inexistente \n");
                result = ERROR; 
                send(conexion_interfaz, &result, sizeof(int32_t), 0);
                close(conexion_interfaz);
            }
            return 0;
        }
        break;
    }
    return 1;
}

