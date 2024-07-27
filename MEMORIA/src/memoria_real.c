#include "../include/memoria_real.h"

void* user_space = NULL;
uint32_t CANT_FRAMES;
uint32_t CANT_FRAMES_libres;
t_bitarray* frames_status;
char* bitmap = NULL;
bool SET = true;
bool clean = false;


// Inicialización de la memoria
void init_memoria() {
    user_space = malloc(TAM_MEMORIA);
    if (user_space == NULL) {
        log_error(logger_memoria_extra, "Error: No se pudo asignar memoria para el espacio de usuario\n");
        exit(EXIT_FAILURE); // O manejar el error de otra forma adecuada
    }
    
    CANT_FRAMES = TAM_MEMORIA / TAM_PAGINA;
    CANT_FRAMES_libres = CANT_FRAMES;

    inicializar_bitarray();
    procesos = list_create();
}

void inicializar_bitarray() {
    size_t size = (CANT_FRAMES + CHAR_BIT - 1) / CHAR_BIT; // Calcular el tamaño en bytes del bitarray necesario
    bitmap = malloc(size); // Asignar memoria para el bitarray

    if (bitmap == NULL) {
        log_error(logger_memoria_extra, "Error: No se pudo asignar memoria para el bitarray\n");
        exit(EXIT_FAILURE); // O manejar el error de otra forma adecuada
    }

    memset(bitmap, 0, size); // Inicializar el bitarray con todos los bits limpios (no usados)
    frames_status = bitarray_create_with_mode(bitmap, size, LSB_FIRST); // Crear el bitarray con modo LSB_FIRST

    if (frames_status == NULL) {
        log_error(logger_memoria_extra, "Error: No se pudo crear el bitarray\n");
        free(bitmap); // Liberar la memoria asignada antes de salir
        exit(EXIT_FAILURE); // O manejar el error de otra forma adecuada
    }
}

// ------------------------ Implementacion de BITARRAY
// Verificar si un marco está ocupado
bool marco_ocupado(int nro_marco) {
    if (nro_marco >= 0 && nro_marco < bitarray_get_max_bit(frames_status)) {
        return bitarray_test_bit(frames_status, nro_marco); // Devuelve el valor del bit de la posición indicada
    }
    return false; // Fuera de los límites o no ocupado
}

// Función auxiliar para asignar un marco de memoria
uint32_t allocate_frame() {
    if(CANT_FRAMES_libres == 0) 
        return UINT32_MAX; // No hay marcos disponibles

    for (int i = 0; i < bitarray_get_max_bit(frames_status); i++) {
        if (!bitarray_test_bit(frames_status, i)){
            status_frame(i, SET);
            return i;
        }
    }
    return UINT32_MAX; // No hay marcos disponibles
}

// Función auxiliar para liberar un marco de memoria
void deallocate_frame(uint32_t nro_frame) {
    if (nro_frame < bitarray_get_max_bit(frames_status)) {
        status_frame(nro_frame, clean);
    }
}

// Cambiar el estado de un marco (ocupar o limpiar)
void status_frame(uint32_t nro_frame, bool condicion) {
    pthread_mutex_lock(&mutex_frames_status);
    if (condicion) {
        bitarray_set_bit(frames_status, nro_frame);
        CANT_FRAMES_libres--;
    } else {
        bitarray_clean_bit(frames_status, nro_frame);
        CANT_FRAMES_libres++;
    }
    pthread_mutex_unlock(&mutex_frames_status);
}

bool add_pages(t_proceso* proceso, int num_pages, int init){
    for (int i = init; i < num_pages; i++) {
        uint32_t* nro_frame = malloc(sizeof(uint32_t));
        *nro_frame = allocate_frame(); 
        if (*nro_frame == UINT32_MAX) {
            return false;  // Fallo en la asignación
        }
        list_add(proceso->pageTable, nro_frame);
    }
    return true;  // Asignación exitosa
}


void remove_pages (t_proceso* proceso, int num_pages){
    for (int i = list_size(proceso->pageTable); i > num_pages; i--) {
        uint32_t frame = *(uint32_t*)list_get(proceso->pageTable, i - 1);
        deallocate_frame(frame);
        list_remove_and_destroy_element(proceso->pageTable, i - 1,&free);
    }
}

int modificar_tabla_paginas(uint32_t num_pages, uint32_t PID, uint32_t tam_resize) {
    t_proceso* proceso = encontrar_proceso_por_pid(PID);
    if (proceso == NULL) {
        log_error(logger_memoria_extra, "PID: %d no encontrado o lista de procesos vacía.\n", PID);
        return -1;
    }
    
    int tam_actual = list_size(proceso->pageTable);

    if (num_pages == tam_actual) {
        log_info(logger_memoria_extra, "PID: %d - No se modificó la tabla de páginas.\n", PID);
        return 0;
    }

    if (num_pages > tam_actual) {
        log_info(logger_memoria, "PID: %d - Tamaño Actual: %d. \n - Tamaño a Ampliar: %d \n", PID, tam_actual, num_pages - tam_actual);
        if (!add_pages(proceso, num_pages, tam_actual)) {
            log_error(logger_memoria, "Error: Out Of Memory.\n");
            return 1;
        }
    } else {
        log_info(logger_memoria, "PID: %d - Tamaño Actual: %d. \n - Tamaño a Reducir: %d \n", PID, tam_actual, tam_actual - num_pages);
        remove_pages(proceso, num_pages);
    }

    return 0;
}

void eliminar_proceso(t_proceso* proceso) {

    if(!list_is_empty(proceso->pageTable)){
        for(int i = 0; i < list_size(proceso->pageTable); i++){
            uint32_t frame = *(uint32_t*)list_get(proceso->pageTable, i);  
            deallocate_frame(frame);
        }   
        log_info(logger_memoria_extra, "PID: %d - Proceso finalizado \n", proceso->pid);
        log_info(logger_memoria, "PID: %d - Tamaño: %d \n", proceso->pid, list_size(proceso->pageTable));
        list_destroy_and_destroy_elements(proceso->pageTable, &free);
        list_destroy_and_destroy_elements(proceso->instrucciones, &free);
    }
    
    pthread_mutex_lock (&mutex_lista_procesos);
    for (int i = 0; i < list_size(procesos); i++){
        t_proceso* proceso_buscado = list_get(procesos, i);
        if (proceso->pid == proceso_buscado->pid){
            list_remove_and_destroy_element(procesos, i, &free);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_procesos);
}

uint32_t acceder_tabla_paginas(uint32_t pid, int page) {
    t_proceso* proceso = encontrar_proceso_por_pid(pid);
    if (proceso != NULL) {
        if (page < list_size(proceso->pageTable)) {
            uint32_t frame = *(uint32_t*)list_get(proceso->pageTable, page);
            log_info(logger_memoria,"PID: %d - Pagina: %d - Marco: %d\n", pid, page, frame);
            return frame;
        } else {
            log_error(logger_memoria_extra,"Error: La página %d no existe en el proceso %d\n", page, pid);
            return UINT32_MAX;
        }
    }
    log_error(logger_memoria_extra, "Error: No se encontró el proceso %d \n", pid);
    return UINT32_MAX;  // Error: No se encontró el proceso o la página
}

// Busca el proceso en la lista de procesos según el PID
t_proceso* encontrar_proceso_por_pid(uint32_t PID) {
    pthread_mutex_lock (&mutex_lista_procesos);
    if (!list_is_empty(procesos)){
        for (int i = 0; i < list_size(procesos); i++) {
            t_proceso* proceso = list_get(procesos, i);
            if (proceso->pid == PID) {
                pthread_mutex_unlock (&mutex_lista_procesos);
                return proceso;
            }
        }
    }
    pthread_mutex_unlock (&mutex_lista_procesos);
    return NULL;
}

char* acceso_usuario(uint32_t direcc_fisica, header action, void* dato, uint32_t size) {
    if (direcc_fisica >= TAM_MEMORIA) {
        log_error(logger_memoria_extra,"Error: Dirección física fuera de los límites de la memoria\n");
        return "ERROR";
    }
    
    // Convertir la dirección física en una dirección en el espacio de usuario
    char* user_address = user_space + direcc_fisica;
    char* value = NULL;

    if (action == LECTURA) {
        // Leer el valor en la dirección de usuario
        char* valor = malloc(size + 1);
        pthread_mutex_lock(&mutex_user_space);
        memcpy(valor, user_address, size);
        pthread_mutex_unlock(&mutex_user_space);
 
        valor[size] = '\0'; // Agregar el caracter nulo al final de la cadena
        
        return valor;
        
    } else if (action == ESCRITURA) {
        
        value = strdup ("OK");
        pthread_mutex_lock(&mutex_user_space);
        memcpy(user_address, dato, size);
        pthread_mutex_unlock(&mutex_user_space);
        /*
        if (*resultado != *user_address){
            log_error(logger_memoria_extra, "Error: Copia de memoria fallida\n");
            value = "ERROR";
        }
        */

        if (memcmp(user_address, dato, size) != 0) {
            log_error(logger_memoria_extra, "Error: Copia de memoria fallida. \n");
            value = strdup ("ERROR");
        } 
    
    } else {
            log_error(logger_memoria_extra, "Error: Acción desconocida\n");
            value = strdup ("ERROR");
    }
    return value;
}