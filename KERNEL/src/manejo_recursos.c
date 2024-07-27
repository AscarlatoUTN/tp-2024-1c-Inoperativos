#include "../include/manejo_recursos.h"

// ------------------------------  Inicializacion de recursos -----------------------------------

// Inicializa recursos del sistema (Cola de BLOCKED, Semaforos, Mutex, etc)
void init_recursos_sist() {
    // Calcula el tamaño del array RECURSOS
    int tamanio_array = calcular_tamanio_array (RECURSOS);

    for(int i = 0; i < tamanio_array; i++){
        if (RECURSOS[i]!=NULL){
            // Inicializa el recurso con su nombre y el número de instancias correspondiente
            recurso_t* recurso = init_recurso(RECURSOS[i], INSTANCIAS_RECURSOS[i]);
            if(recurso == NULL) {
                log_error(logger_kernel_extra, "Hubo un error inicializando el recurso: %s \n", RECURSOS[i]);
                exit(EXIT_FAILURE);
            }

            // Crea un hilo para manejar la cola block del recurso
            pthread_t hilo_manejo_recursos; //Creamos un hilo por recurso para manejo cola block
            
            pthread_create(&hilo_manejo_recursos, NULL, (void *)manejador_recursos, recurso->nombre);
            // free(recurso);
            //free (INSTANCIAS_RECURSOS[i]); AGREGARLO AL FINALIZAR PROGRAMA
            // free (RECURSOS[i]); AGREGARLO AL FINALIZAR PROGRAMA
        }
    }
}

int calcular_tamanio_array(char** array) {
    int size = 0;
    while (array[size] != NULL) {
        size++;
    }
    return size;
}

// Inicializar un recurso con su nombre y número de instancias
recurso_t* init_recurso(char* nombre_recurso, char* instancias_recurso) {
    recurso_t* recurso = malloc(sizeof(recurso_t));
    if (recurso == NULL) {
        log_error(logger_kernel_extra, "Error al asignar memoria para el recurso: %s \n", nombre_recurso);
        return NULL;
    }
    recurso->nombre = strdup(nombre_recurso);
   
    if (recurso->nombre == NULL) {
        log_error(logger_kernel_extra, "Error al duplicar el nombre del recurso: %s \n", nombre_recurso);
        free(recurso);
        return NULL;
    }
    recurso->instancias = atoi(instancias_recurso);

    init_cola_blocked(recurso->nombre);
    init_mutex(recurso->nombre, INSTANCES);
    init_sem_contador_recursos (recurso); //Inicializamos el semaforo en la cantidad de instancias
    init_semaforo_block(recurso->nombre, SEM_RECURSO);
    
    // Agrega el recurso a la lista de recursos
    pthread_mutex_lock(&mutex_lista_recursos);
    list_add(recursos, recurso);
    pthread_mutex_unlock(&mutex_lista_recursos);
    return recurso;
}

// Inicializa el semáforo contador para un recurso específico
void init_sem_contador_recursos(recurso_t* recurso) {
    // Construir el nombre del semáforo asociado a la cola
    char* nombre_semaforo = construir_nombre_sem("sem_instancias_", recurso->nombre);

    // Crear y anclar el semáforo al diccionario
    sem_t* semaforo = malloc(sizeof(sem_t));
    if (semaforo == NULL) {
        log_error(logger_kernel_extra, "No se pudo asignar memoria para el semáforo.");
        free(nombre_semaforo);
        return;
    }

    // Inicializar el semáforo con valor de las instancias
    if (sem_init(semaforo, 0, recurso->instancias) != 0) {
        log_error(logger_kernel_extra, "No se pudo inicializar el semáforo.");
        free(semaforo);
        free(nombre_semaforo);
        return;
    }

    // Añade el semáforo al diccionario
    // pthread_mutex_lock(&mutex_dict_sem);
    dictionary_put(diccionario_sem, nombre_semaforo, semaforo);
    // pthread_mutex_unlock(&mutex_dict_sem);

    // Liberar el nombre del semáforo ya que está copiado en el diccionario
    free(nombre_semaforo); 
}

// ------------------------------  WAIT -----------------------------------

// Función que realiza el hilo 
void manejador_recursos(char* nombre_recurso){

    while(1){
        // Espera hasta que un proceso quede bloqueado por un recurso
        op_sem_block(nombre_recurso, WAIT, SEM_BINARIO, BLOCKED);
        op_sem_block(nombre_recurso, WAIT, SEM_CONTADOR, INSTANCES);

        t_pcb* PCB = cola_block_get(nombre_recurso);
        if(PCB == NULL){
            log_warning(logger_kernel_extra, "COLA BLOCK %s - No se pudo encontrar el proceso \n", nombre_recurso);
            return; 
        }

        // Verifica si hay instancias disponibles del recurso, si las hay se las asigna
        if (instancias_disp(PCB, nombre_recurso) == 0) {
            sacar_pcb_cola_block(nombre_recurso); 
            log_info(logger_kernel_extra, "Se asigno correctamente el recurso. Recurso: %s - PID: %d \n", nombre_recurso, PCB->PID);
            manejar_vrr(PCB, cola_aux_vrr, &mutex_cola_aux_vrr);
            sem_post(&sem_corto_plazo);
        } else {
            log_error(logger_kernel_extra, "Error al verificar instancias disponibles.");
        }
    }
}


// -------------------------- Funciones Auxiliares--------------------------

// ---------------------- WAIT
// Verifica si hay instancias disponibles de un recurso para un PCB, si las hay, se las asigna.
int instancias_disp(t_pcb* PCB, char* nombre_recurso) { 
    recurso_t* recurso_requerido = encontrar_recurso(nombre_recurso);
    if(recurso_requerido == NULL) {
        log_error(logger_kernel_extra, "Recurso invalido o desconocido: %s  \n", nombre_recurso);
        return -1;
    }
    
    op_mutex(nombre_recurso, LOCK);
    int result = 1; // Si es 1, no hay instancias
    if (recurso_requerido->instancias > 0) {
        recurso_requerido->instancias -= 1;
        result = 0;
    }
    op_mutex(nombre_recurso, UNLOCK);

    if (result == 0 && agregar_recurso_asoc(PCB, nombre_recurso) != 0) {
        log_error(logger_kernel_extra, "Error al asignar recursos. Recurso: %s - PID: %d \n", nombre_recurso, PCB->PID);
        result = -1;
    }

    return result; 
}

// ---------------------- SIGNAL
int aumentar_instancias_recurso(t_pcb* PCB, char* nombre_recurso){

    recurso_t* recurso_buscado = encontrar_recurso(nombre_recurso);
    
    //Si no se encuentra el recurso pedido, enviamos el proceso a EXIT.
    if(recurso_buscado == NULL) {
        log_error(logger_kernel_extra, "Recurso invalido o desconocido: %s  \n", nombre_recurso);
        free(nombre_recurso);
        return 1;
    }

    //Eliminamos el recurso asociado al proceso de la lista
    if(actualizar_recurso_asoc(PCB, nombre_recurso) != 0) {
        log_error(logger_kernel_extra, "Error al desalojar recursos. Recurso: %s - PID: %d \n", nombre_recurso, PCB->PID);
        free(nombre_recurso);
        return 1;
    }

    //Aumentamos en uno las instancias del recurso encontrado.
    op_mutex(recurso_buscado->nombre, LOCK);
    recurso_buscado->instancias += 1;
    op_mutex(recurso_buscado->nombre, UNLOCK);

    //Hacemos un sem_post del semaforo contador para aumentar en uno las instancias.
    op_sem_block(recurso_buscado->nombre, SIGNAL, SEM_CONTADOR, INSTANCES);
    return 0;
}

int actualizar_recurso_asoc(t_pcb* PCB, char* nombre_recurso){
    // Buscar el proceso en la lista de pid_recs
    pid_rec* proceso = encontrar_proceso_por_PID(PCB->PID);

    // Si no se encontró el proceso, no se puede eliminar el recurso
    if (proceso == NULL) {
        return 1;
    }

    // Buscar y eliminar el recurso de la lista de recursos asociados
    if (buscar_y_eliminar_recurso(proceso, nombre_recurso) == 0) {
        return 0; // Se encontró y eliminó el recurso
    }

    // Si no se encontró el recurso, devuelve un código de error
    return 1;
}

// Busca un recurso en la lista de recursos asociados de un proceso y lo elimina si lo encuentra
int buscar_y_eliminar_recurso(pid_rec* proceso, char* nombre_recurso) {
    for (int j = 0; j < list_size(proceso->recs_asoc); j++) {
        char* recurso_aux = list_get(proceso->recs_asoc, j);
        if (strcmp(recurso_aux, nombre_recurso) == 0) {
           // free(recurso_aux);
            list_remove(proceso->recs_asoc, j);
            return 0; // Indica que se encontró y eliminó el recurso
        }
    }
    return 1; // Indica que el recurso no se encontró en la lista
}

// Agrega el nombre del recurso a la lista de recursos asoc de un proceso
int agregar_recurso_asoc(t_pcb* PCB, char* nombre_recurso){
    // Buscar el proceso en la lista de pid_recs
    pid_rec* proceso = encontrar_proceso_por_PID(PCB->PID);

    // Si no se encontró el proceso, crear una nueva entrada para el mismo
    if (proceso == NULL) {
        proceso = crear_nuevo_proceso(PCB->PID);
        if (proceso == NULL) {
            return 1; // Error al crear el proceso
        }
    }

    char* nombre_recurso_lista = strdup(nombre_recurso);
    // Agregar el nombre del recurso a la lista de recursos asociados del proceso
    list_add(proceso->recs_asoc, nombre_recurso_lista);
    return 0;
}


// ---------------------- EXIT

int eliminar_recursos_asociados(t_pcb* PCB){
    pid_rec* proceso_actual = encontrar_proceso_por_PID(PCB->PID);
    if(proceso_actual == NULL) {
        return 0;
    }
    // Verificar si la lista de recursos asociados está vacía
    if (!list_is_empty(proceso_actual->recs_asoc)){
        int size = list_size(proceso_actual->recs_asoc);
    
        for (int i = size - 1; i >= 0; i--) {
            char* nombre_recurso = list_get(proceso_actual->recs_asoc, i);
            
            log_info(logger_kernel_extra, "Nombre del recurso: %s \n", nombre_recurso);
            // Desaloja al recurso del proceso, aumentando las instancias y haciendo signal
            if(aumentar_instancias_recurso(PCB, nombre_recurso)!= 0) {
                log_error(logger_kernel_extra, "Error al desalojar recursos. PID: %d, Recurso: %s \n", PCB->PID, nombre_recurso);
                //free(nombre_recurso); NO SE
                return 1; 
            }
        }

        // Eliminar la lista de recursos asociados del proceso
        list_destroy_and_destroy_elements(proceso_actual->recs_asoc, &free);
        proceso_actual->recs_asoc = NULL; // Asignar NULL después de destruir la lista
        // Elimina la entrada del proceso de la lista de PID_RECS
        if(eliminar_proceso_por_PID(PCB->PID) != 0) {
            log_error(logger_kernel_extra, "Error al eliminar su entrada de la lista PID_RECS. PID: %d \n",PCB->PID);
            return 1;
        }
    }
    return 0;
}

// ---------------------- GENERICAS

// Encuentra un recurso en la lista de recursos por su nombre
recurso_t* encontrar_recurso(char* nombre_recurso) {
    for (int i = 0; i < list_size(recursos); i++) {
        recurso_t* recurso_requerido = list_get(recursos, i);
        if (strcmp(recurso_requerido->nombre, nombre_recurso) == 0)
            return recurso_requerido;
    }
    return NULL;
}

// Busca el proceso en la lista de pid_recs por su PID
pid_rec* encontrar_proceso_por_PID(int PID) {
    pthread_mutex_lock(&mutex_lista_pid_recs);
    if (!list_is_empty (pid_recs)){
        for (int i = 0; i < list_size(pid_recs); i++) {
            pid_rec* pid_actual = list_get(pid_recs, i);
            if (pid_actual->PID == PID) {
                pthread_mutex_unlock (&mutex_lista_pid_recs);
                return pid_actual;
            }
        }
    }
    pthread_mutex_unlock (&mutex_lista_pid_recs);
    return NULL;
}

// Crea una nueva entrada de proceso en la lista de pid_recs
pid_rec* crear_nuevo_proceso(uint32_t PID) {
    pid_rec* nuevo_proceso = malloc(sizeof(pid_rec));
    if (nuevo_proceso == NULL) {
        log_error(logger_kernel_extra, "No se pudo asignar memoria para el proceso asociado.");
        return NULL;
    }
    nuevo_proceso->PID = PID;
    nuevo_proceso->recs_asoc = list_create();
    pthread_mutex_lock (&mutex_lista_pid_recs);
    list_add(pid_recs, nuevo_proceso);
    pthread_mutex_unlock (&mutex_lista_pid_recs);
    return nuevo_proceso;
}

void op_mutex(char* nombre_recurso, bool lock) {
    // Construir el nombre del mutex asociado al recurso
    char* nombre_mutex = construir_nombre_sem("mutex_instancias_", nombre_recurso);
    if (nombre_mutex == NULL) {
        log_error(logger_kernel_extra, "Error: No se pudo construir el nombre del mutex para '%s'. \n", nombre_recurso);
        free(nombre_mutex);
        return; 
    }

    // Obtener el mutex del diccionario
    pthread_mutex_t* mutex = dictionary_get(diccionario_mutex, nombre_mutex);
    if (mutex == NULL) {
        log_error(logger_kernel_extra, "Error: No se pudo obtener el mutex del diccionario para '%s'. \n", nombre_recurso);
        free(nombre_mutex);
        return;
    }
    
    // Bloquea o desbloquea el mutex según la operación solicitada
    int result = lock ? pthread_mutex_lock(mutex) : pthread_mutex_unlock(mutex);
    if (result != 0) {
        log_error(logger_kernel_extra, "Error: Fallo al %s bloquear el mutex para '%s'. \n", lock ? "" : "des", nombre_recurso);
        free(nombre_mutex);
        return;
    }
    free(nombre_mutex); 
}

int eliminar_proceso_por_PID(int PID) {
    pthread_mutex_lock(&mutex_lista_pid_recs);
    if (!list_is_empty (pid_recs)){
        for (int i = 0; i < list_size(pid_recs); i++) {
            pid_rec* pid_actual = list_get(pid_recs, i);
            if (pid_actual->PID == PID) {
                list_remove_and_destroy_element(pid_recs, i, &free);
                pthread_mutex_unlock(&mutex_lista_pid_recs);
                return 0;
            }
        }
    }
    pthread_mutex_unlock(&mutex_lista_pid_recs);

    return 1;
}