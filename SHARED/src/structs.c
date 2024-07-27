#include "../include/structs.h"

// ----------------------Inicializar Semaforos-------------------------

// Función de incremento para el semáforo binario
void signal_bin(sem_t *sem) {
    int valor_actual;
    sem_getvalue(sem, &valor_actual);
    if (valor_actual == 0) {
        sem_post(sem);  // Incrementar solo si el valor actual es 0
    }
}

void flag_interrupcion(pthread_mutex_t* mutex_interrupcion, int32_t flag, int* hubo_interrupcion){
    pthread_mutex_lock(mutex_interrupcion);
    *hubo_interrupcion = flag;
    pthread_mutex_unlock(mutex_interrupcion);
}


// -------------- Liberar Estructuras Comunes ----------------

void liberar_df_size(df_size* recibido){
    list_destroy_and_destroy_elements(recibido->list_df_io, &free);
    free(recibido);
}

void liberar_params_inst_fs (params_inst_fs* int_eliminar) {
    list_destroy_and_destroy_elements(int_eliminar->lista_dfs, &free);
    free(int_eliminar->nombre_archivo);
    free(int_eliminar);
}

void liberar_pcb(t_pcb* PCB){
    free(PCB->reg);
    free(PCB);
}

char* estado_pcb(ESTADOS estado) {
    static char* estados[] = { "NEW", "READY", "EXEC", "BLOCK", "EXIT" };
    if (estado < 0 || estado >= sizeof(estados) / sizeof(estados[0])) {
        return NULL;
    }
    return estados[estado];
}

void liberar_tabla_w(t_tabla_w* tabla) {
    free(tabla->dato);
    free(tabla);
}
