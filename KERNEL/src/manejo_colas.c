#include "../include/manejo_colas.h"

// ---------------------- Manejo de Colas de Planificación -------------------------


// Función para agregar un PCB a la cola de bloqueados asociada a un nombre de cola en el diccionario
bool agregar_pcb_a_cola(char* nombre, t_pcb* pcb) {
    // Verificar si la cola existe en el diccionario
    t_list* cola = get_cola_blocked(nombre);
    if(cola == NULL)
        return false;

    char* nombre_mutex = construir_nombre_sem("mutex_block_", nombre);

    // Obtener el mutex asociado a la cola
    pthread_mutex_t* mutex = dictionary_get(diccionario_mutex, nombre_mutex);
   
    // Verificar si el mutex está inicializado
    if (mutex == NULL) {
        log_error(logger_kernel_extra, "No se encontró el mutex asociado a la cola '%s'. \n", nombre);
        free(nombre_mutex);
        return false;
    }

    ingresar_a_block(cola, pcb, mutex);
    free(nombre_mutex);
    return true;
}


t_pcb* sacar_pcb_cola_block(char* nombre) {
    
    t_list* cola = get_cola_blocked(nombre);
    if(cola == NULL) 
        return NULL;

    pthread_mutex_t* mutex = obtener_mutex_diccionario(nombre);

    if (mutex == NULL) {
        log_error(logger_kernel_extra, "No se encontró el mutex asociado a la cola de '%s'. \n", nombre);
        return NULL;
    }

    if (list_is_empty(cola)) {
        log_warning(logger_kernel_extra, "La cola '%s' está vacía. \n", nombre);
        return NULL;
    }

    pthread_mutex_lock(mutex);
    t_pcb* PCB = list_remove(cola, 0);
    pthread_mutex_unlock(mutex);

    return PCB;
}

t_pcb* cola_block_get(char* nombre_recurso) {

    t_list* cola = get_cola_blocked(nombre_recurso);
    if(cola == NULL)
        return NULL;

    char* nombre_mutex = construir_nombre_sem("mutex_block_", nombre_recurso);

    pthread_mutex_t* mutex = dictionary_get(diccionario_mutex, nombre_mutex);
    if (mutex == NULL) {
        log_error(logger_kernel_extra, "No se encontró el mutex asociado a la cola '%s'. \n", nombre_recurso);
        free(nombre_mutex);
        return NULL;
    }
    if (list_is_empty(cola)) {
        log_error(logger_kernel_extra, "La cola '%s' está vacía. \n", nombre_recurso);
        free(nombre_mutex);
        return NULL;
    }

    pthread_mutex_lock(mutex);
    t_pcb* PCB = list_get(cola, 0);
    pthread_mutex_unlock(mutex);
    free(nombre_mutex);
    //free(nombre_recurso); funciona??????
    return PCB;
}

pthread_mutex_t* obtener_mutex_diccionario(char* nombre) {
    char* nombre_mutex = construir_nombre_sem("mutex_block_", nombre);
    pthread_mutex_t* mutex = dictionary_get(diccionario_mutex, nombre_mutex);
    free(nombre_mutex);
    return mutex;
}

// Función auxiliar para agregar el PCB a la cola auxiliar de VRR si el quantum no se ha agotado
void agregar_a_cola_aux_vrr(t_pcb* PCB_actualizado, t_list* cola_aux_vrr, pthread_mutex_t* mutex_cola_aux_vrr) {
    if (PCB_actualizado->q_restante < QUANTUM) {
        pthread_mutex_lock(mutex_cola_aux_vrr);
        list_add(cola_aux_vrr, PCB_actualizado);
        pthread_mutex_unlock(mutex_cola_aux_vrr);
        log_info(logger_kernel, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY \n", PCB_actualizado->PID);
        PCB_actualizado->p_status = READY;
    }
}

void ingresar_a_new(t_pcb* PCB) {
    log_info(logger_kernel, "PID: %d - Estado Anterior: ... - Estado Actual: NEW \n", PCB->PID);
    pthread_mutex_lock(&mutex_new);
    list_add(cola_new, PCB);
    pthread_mutex_unlock(&mutex_new);
    PCB->p_status = NEW;   
} 

void ingresar_a_ready(t_list* cola, t_pcb* PCB){
           
    if(cola != NULL) {
        sacar_pcb_lista(cola, PCB->PID, &mutex_ready);
        log_warning(logger_kernel_extra, "Se sacó el proceso %d de la cola \n", PCB->PID);
    }

    pthread_mutex_lock(&mutex_ready); 
    list_add(cola_ready, PCB);
    pthread_mutex_unlock(&mutex_ready);

    char* estado = estado_pcb (PCB->p_status);
    log_info(logger_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: READY \n", PCB->PID, estado);
    PCB->p_status = READY;
    
} 

void ingresar_a_block(t_list* cola, t_pcb* PCB, pthread_mutex_t* mutex_block){
            
    pthread_mutex_lock(mutex_block);
    list_add(cola, PCB);
    pthread_mutex_unlock(mutex_block); 
    char* estado = estado_pcb(PCB->p_status);
    log_info(logger_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: BLOCK \n", PCB->PID, estado);
    PCB->p_status = BLOCK;
    
}

t_pcb* sacar_pcb_lista(t_list* cola, uint32_t PID, pthread_mutex_t* mutex_cola){
	if (cola == NULL){
        return NULL;
    }
    
    pthread_mutex_lock(mutex_cola);
    for (int i = 0; i < list_size(cola); i++){
		t_pcb* pcb_aux = list_get(cola, i);
		if (pcb_aux->PID == PID){
			list_remove(cola, i);
            pthread_mutex_unlock(mutex_cola);
			return pcb_aux;
		}
	}
    pthread_mutex_unlock(mutex_cola);
    log_warning(logger_kernel_extra, "No se encontro el proceso %d en la cola \n", PID);
    return NULL;
}

t_pcb* finish_pcb(t_list* cola, uint32_t PID){
	if (cola == NULL){
        return NULL;
    }
    
    for (int i = 0; i < list_size(cola); i++){
		t_pcb* pcb_aux = list_get(cola, i);
		if (pcb_aux->PID == PID){
			list_remove(cola, i);
			return pcb_aux;
		}
	}
    log_error(logger_kernel_extra, "No se encontro el proceso %d en la cola \n", PID);
    return NULL;
}

t_pcb* obtener_sgte_ready()
{
    pthread_mutex_lock(&mutex_new);
    t_pcb* PCB = list_remove(cola_new, 0);
    pthread_mutex_unlock(&mutex_new);
    return PCB;
}