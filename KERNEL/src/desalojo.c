#include "../include/desalojo.h"
bool existe_tipo, existencia_interfaz;
bool continuar = true;

// ----------------------------- Funcion Principal ---------------------------------

void motivo_desalojo(){
   
    while (continuar){
        // Espero a recibir el motivo de desalojo
        
        int motivo = recibir_operacion(conexion_cpu_dispatch);
        pcb_motivo* paquete_pcb = recibir_pcb_motivo(conexion_cpu_dispatch);
        if (strcmp (ALGORITMO_PLANIFICACION, "RR") == 0 || strcmp (ALGORITMO_PLANIFICACION, "VRR") == 0){
            manejar_quantum(paquete_pcb->pcb);
        } 
        continuar = manejar_motivo_desalojo(motivo, paquete_pcb->pcb, paquete_pcb->buffer_size);
        sem_post(&sem_exec);
        free(paquete_pcb);
    }
    exit(EXIT_FAILURE);
}

// ----------------------------- Switch ---------------------------------

bool manejar_motivo_desalojo(int motivo, t_pcb* PCB_actualizado, uint32_t buffer_size) {
    switch (motivo) {
        case M_FIN_QUANTUM:
            log_info(logger_kernel, "PID: %d - Desalojado por fin de Quantum \n", PCB_actualizado->PID);
            manejar_fin_quantum(PCB_actualizado); // Despierta a planificacion corto plazo
            break;
        case M_WAIT:
            log_info(logger_kernel_extra, "Motivo de desalojo: WAIT \n");
            manejar_wait(PCB_actualizado, buffer_size);
            break;
        case M_SIGNAL:
            log_info(logger_kernel_extra, "Motivo de desalojo: SIGNAL \n");
            manejar_signal(PCB_actualizado, buffer_size);            
            break;
        case M_IO_GEN:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO GEN SLEEP \n");
            manejar_io_gen_sleep(PCB_actualizado, buffer_size);
            break;
        case M_IO_STDIN:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO STDIN \n");
            manejar_io_std(PCB_actualizado, buffer_size, interfaces_stdin);
            break;
        case M_IO_STDOUT:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO STDOUT \n"); 
            manejar_io_std(PCB_actualizado, buffer_size, interfaces_stdout);
            break;
        case M_IO_FS_CREATE:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO FS_CREATE \n"); 
            manejar_fs(PCB_actualizado, buffer_size, IO_FS_CREATE);
            break;
        case M_IO_FS_TRUNCATE:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO FS_TRUNCATE \n");
            manejar_fs(PCB_actualizado, buffer_size, IO_FS_TRUNCATE);
            break;
        case M_IO_FS_DELETE:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO FS_DELETE \n");
            manejar_fs(PCB_actualizado, buffer_size, IO_FS_DELETE);
            break;
        case M_IO_FS_READ:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO FS_READ \n");
            manejar_fs_rw(PCB_actualizado, buffer_size, IO_FS_READ);
            break;
        case M_IO_FS_WRITE:
            log_info(logger_kernel_extra, "Motivo de desalojo: IO FS_WRITE \n");
            manejar_fs_rw(PCB_actualizado, buffer_size, IO_FS_WRITE);
            break;
        case M_EXIT:
            log_info(logger_kernel, "Finaliza el proceso %d - Motivo: SUCCESS \n", PCB_actualizado->PID);
            manejar_exit(PCB_actualizado);
            break;
        case M_PROC_INTERRUPT:
            log_info(logger_kernel, "Finaliza el proceso %d - Motivo: INTERRUPTED BY USER \n", PCB_actualizado->PID);
            manejar_exit(PCB_actualizado);
            iniciar_planificacion ();//
            break;
        case M_OOM:
            log_info(logger_kernel, "Finaliza el proceso %d - Motivo: OUT_OF_MEMORY \n", PCB_actualizado->PID);
            manejar_exit(PCB_actualizado);
            break;
        case -1:
            log_error(logger_kernel_extra, "Error al recibir el motivo de desalojo \n");
            continuar = false;
            break;
        default:
            log_error(logger_kernel_extra, "Motivo de desalojo desconocido: %d \n", motivo);
            continuar = false;
            break;
    }
    return continuar;
}

// ----------------------------- Funciones Auxiliares de Manejo de Casos ---------------------------------
// Funcion que, en caso de VRR se encarga de realizar la cuenta de q restante
void manejar_quantum(t_pcb* PCB_actualizado){
    sem_post(&sem_Q);
    sem_wait(&sem_tiempo_ejecutado);
    log_warning(logger_kernel, "PID: %d - Quantum %d \n", PCB_actualizado->PID, PCB_actualizado->q_restante);
    //pthread_mutex_lock(&mutex_tiempo_ejecutado);///
    if (PCB_actualizado->q_restante > tiempo_ejecutado){
        PCB_actualizado->q_restante -= tiempo_ejecutado;
       // log_warning(logger_kernel_extra, "Quantum restante %d - Tiempo ejecutado: %d \n", PCB_actualizado->q_restante, tiempo_ejecutado);
    }
    else
        PCB_actualizado->q_restante = QUANTUM;
   // pthread_mutex_unlock(&mutex_tiempo_ejecutado); ///
}

//-----------------MANEJAR FIN DE QUANTUM---------------------------
void manejar_fin_quantum(t_pcb* PCB_actualizado)
{
    PCB_actualizado->q_restante = QUANTUM;
    ingresar_a_ready(NULL, PCB_actualizado); // Se agrega al final de la lista, se le cambia el estado

    sem_post(&sem_corto_plazo);
    log_warning(logger_kernel, "Fin de Quantum \n");
}

//-----------------MANEJAR IO GEN SLEEP---------------------------
void manejar_io_gen_sleep(t_pcb* PCB_actualizado,uint32_t size) {
    interfaz_a_enviar_gen* int_params = recibir_parametros_io_gen(conexion_cpu_dispatch, size);

    char* nombre_interfaz = strdup(int_params->nombre_interfaz);
    t_io_gen* interfaz;
    for (int i = 0; i < list_size(interfaces_gen); i++) {
        interfaz = list_get(interfaces_gen, i);
        if (strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) {
            existe_tipo = true;
            break;
        }
    }

    io_gen_pid* interfaz_gen = malloc(sizeof(io_gen_pid));
    interfaz_gen->PID = PCB_actualizado->PID;
    interfaz_gen->unidades_trabajo = int_params->unidades_trabajo;
    
    if (interfaz_valida(PCB_actualizado, nombre_interfaz, existe_tipo)) 
        enviar_gen_pid(interfaz->fd_interfaz, interfaz_gen);
    
    liberar_parametros_io_gen(int_params);
    free(interfaz_gen);
    free(nombre_interfaz);
}

//-------------------------------- MANEJAR IO STDIN-STDOUT ---------------------------------

void manejar_io_std(t_pcb* PCB_actualizado, uint32_t size, t_list* lista_int) {
    interfaz_a_enviar_std* int_params = recibir_parametros_interfaz_std(conexion_cpu_dispatch, size);
    df_size* para_enviar = malloc(sizeof(df_size));

    char* nombre_interfaz = strdup(int_params->nombre_interfaz);
    para_enviar->list_df_io = int_params->list_df;
    para_enviar->list_df_io_size = int_params->list_df_size;
    para_enviar->reg_tamanio = int_params->reg_tamanio;
    para_enviar->offset = int_params->offset;
    para_enviar->PID = PCB_actualizado->PID;

    t_io_std* interfaz;

    for (int i = 0; i < list_size(lista_int); i++) {
        interfaz = list_get(lista_int, i);
        if (strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) {
            existe_tipo = true;
            break;   
        }
    }

    if(interfaz_valida(PCB_actualizado, nombre_interfaz, existe_tipo))
        enviar_df_size(DIRECC_MEM, interfaz->fd_io_kernel, para_enviar);
    
    liberar_parametros_io_std(int_params);
    free(nombre_interfaz);
    free(para_enviar);
}

// Función genérica para manejar la existencia de una interfaz
bool interfaz_valida(t_pcb* PCB_actualizado, char *nombre_interfaz, bool existe_tipo)
{

    if (!existe_tipo) {
        log_info(logger_kernel, "Finaliza el proceso %d - Motivo: INVALID_INTERFACE \n", PCB_actualizado->PID);
        manejar_exit(PCB_actualizado);
        return false;
    } else {
        bool existencia_interfaz = agregar_pcb_a_cola(nombre_interfaz, PCB_actualizado);

        if (existencia_interfaz) {
            log_info(logger_kernel, "PID: %d - Bloqueado por: Interfaz %s \n", PCB_actualizado->PID, nombre_interfaz);
            if (op_sem_block(nombre_interfaz, WAIT, SEM_BINARIO, BLOCKED) == NULL)
                return false;
            return true;
        } else {
            log_error(logger_kernel_extra, "No se pudo ingresar el PID: %d en la cola de BLOCK \n", PCB_actualizado->PID);
            // Enviar proceso a exit
            manejar_exit(PCB_actualizado);
            return false;
        }
    }
}

//-----------------MANEJAR FS_RW -----------------------------------
void manejar_fs_rw(t_pcb* PCB_actualizado, uint32_t buffer_size, header accion){
    bool existe_tipo = false;
    parametros_fs_rw* int_params = recibir_params_fs_rw(conexion_cpu_dispatch, buffer_size);
    
    t_io_df* interfaz;
    for (int i = 0; i < list_size(interfaces_dialfs); i++) {
        interfaz = list_get(interfaces_dialfs, i);
        if (strcmp(interfaz->nombre_interfaz, int_params->nombre_interfaz) == 0) {
            existe_tipo = true;
            break;
        }
    }
    
    if(!interfaz_valida(PCB_actualizado, int_params->nombre_interfaz, existe_tipo)){
        liberar_parametros_fs_rw(int_params);
        manejar_exit(PCB_actualizado);
        return;
    }

    params_inst_fs* parametros = malloc(sizeof(params_inst_fs));    
    parametros->nombre_archivo = strdup(int_params->nombre_archivo);
    parametros->length_nombre_archivo = int_params->length_nombre_archivo;
    parametros->PID = PCB_actualizado->PID;
    parametros->lista_dfs_size = int_params->lista_dfs_size;
    parametros->lista_dfs = int_params->lista_dfs;
    parametros->reg_tamanio = int_params->reg_tamanio;
    parametros->ptr_arch = int_params->ptr_arch;
    parametros->offset = int_params->offset;
    
    enviar_params_inst(accion, parametros, interfaz->fd_io_kernel);
    liberar_parametros_fs_rw(int_params);
    liberar_params_inst_fs(parametros); // prob este mal
}

void liberar_parametros_fs_rw(parametros_fs_rw* parametros){
    free(parametros->nombre_archivo);
    free(parametros->nombre_interfaz);
    free(parametros);
}

//-----------------MANEJAR FS (TRUNCATE, DELETE, CREATE) ---------------------------
void manejar_fs(t_pcb* PCB_actualizado, uint32_t buffer_size, header accion){
    bool existe_tipo = false;
    interfaz_a_enviar_fs* int_params = recibir_parametros_kernel_fs(conexion_cpu_dispatch, buffer_size);
    
    t_io_df* interfaz;
    for (int i = 0; i < list_size(interfaces_dialfs); i++) {
        interfaz = list_get(interfaces_dialfs, i);
        if (strcmp(interfaz->nombre_interfaz, int_params->nombre_interfaz) == 0) {
            existe_tipo = true;
            break;
        }
    }
    
    if(!interfaz_valida(PCB_actualizado, int_params->nombre_interfaz, existe_tipo)){ // PCB liberado dentro
        liberar_int_a_enviar_fs(int_params);
        return;
    }
    
    parametros_fs* parametros = malloc(sizeof(parametros_fs));    
    parametros->nombre_archivo = strdup(int_params->nombre_archivo);
    parametros->length_nombre_archivo = int_params->length_nombre_archivo;
    parametros->PID = PCB_actualizado->PID;
    parametros->reg_tamanio = int_params->reg_tamanio;
    
    enviar_parametros_fs(accion, parametros, interfaz->fd_io_kernel);
    liberar_int_a_enviar_fs(int_params);
}
//-----------------MANEJAR EXIT---------------------------
void manejar_exit(t_pcb* PCB) {
    char* estado = estado_pcb (PCB->p_status);
    log_info(logger_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: EXIT \n", PCB->PID, estado);

    PCB->p_status = EXIT;
    // Eliminamos los recursos asociados al proceso, aumentamos en uno las instancias de los recuros sencontrados y hacemos un signal

    if(eliminar_recursos_asociados(PCB) != 0) {
        log_error(logger_kernel_extra, "Error al desalojar recursos. PID: %d \n",PCB->PID);
        exit(EXIT_FAILURE);
    }

    // Enviamos la indicacion a memoria de eliminar las estructuras asociadas al proceso
    enviar_nums(ELIMINAR_PROCESO, conexion_memoria, PCB->PID);
    int result;
    recv(conexion_memoria, &result, sizeof(int), MSG_WAITALL);
    if(result != 0) {
        log_error(logger_kernel_extra, "Error al recibir confirmación de eliminación de estructuras en memoria \n");
        exit(EXIT_FAILURE);
    }

    log_info(logger_kernel_extra, "Finaliza el proceso %d \n", PCB->PID);
    liberar_pcb(PCB);
    sem_post(&sem_multiprogramacion);
}

//-----------------MANEJAR WAIT---------------------------

void manejar_wait(t_pcb* PCB, uint32_t buffer_size) {
    char* nombre_recurso = recibir_recurso(conexion_cpu_dispatch, buffer_size);
    
    int result;
    t_list* cola_blocked = get_cola_blocked(nombre_recurso);
    if(cola_blocked == NULL) {
        result = -1;
    } else if(list_is_empty(cola_blocked)) {
        result = instancias_disp(PCB, nombre_recurso);
    } else {
        log_info(logger_kernel_extra, "No se puede asignar porque hay procesos esperando. \n");
        result = 1;
    }

    // Verifica si hay instancias disponibles del recurso, si las hay se las asigna
    if(result == 0) {
        op_sem_block(nombre_recurso, WAIT, SEM_CONTADOR, INSTANCES);
        log_info(logger_kernel_extra, "Se asigno correctamente el recurso. Recurso: %s - PID: %d \n", nombre_recurso, PCB->PID);
        sem_post(&sem_tiempo_ejecutado); // ?
        enviar_pcb(conexion_cpu_dispatch, PCB, CTXT_EXEC);
        
    } else if (result == -1){
        log_info(logger_kernel, "Finaliza el proceso %d - Motivo: INVALID_RESOURCE \n", PCB->PID);
        manejar_exit(PCB);
    } else if (result == 1){
        log_info(logger_kernel_extra, "No hay instancias disponibles del recurso: %s \n Enviando proceso a BLOCK...\n", nombre_recurso);
        // Agrega el PCB a la cola de bloqueados del recurso y cambia su estado a BLOCK
        if (agregar_pcb_a_cola(nombre_recurso, PCB)) {
            log_info(logger_kernel, "PID: %d - Bloqueado por: %s \n", PCB->PID, nombre_recurso);
            op_sem_block(nombre_recurso, SIGNAL, SEM_BINARIO, BLOCKED);
        } else {
            log_error(logger_kernel_extra, "Error al agregar PCB a la cola BLOCK. PID: %d \n", PCB->PID);
        }   
    }
    free(nombre_recurso);
}

// Función para obtener la cola bloqueada por nombre de recurso
t_list* get_cola_blocked(char* nombre_recurso) {
    pthread_mutex_lock(&mutex_dict_colas);
    t_list* cola = dictionary_get(diccionario_colas, nombre_recurso);
    pthread_mutex_unlock(&mutex_dict_colas);

    if (cola == NULL) {
        log_warning(logger_kernel_extra, "No se encontró la cola para el recurso: %s. \n", nombre_recurso);
    }

    return cola;
}    

// ------------------------------ MANEJAR SIGNAL -----------------------------
void manejar_signal(t_pcb* PCB, uint32_t buffer_size) {
    char* nombre_recurso = recibir_recurso(conexion_cpu_dispatch, buffer_size);

    // Aumenta las instancias del recurso y envía el PCB a ejecución
    if (aumentar_instancias_recurso(PCB, nombre_recurso) == 0) {
        sem_post(&sem_tiempo_ejecutado);
        enviar_pcb(conexion_cpu_dispatch, PCB, CTXT_EXEC);    
    } else {
        log_error(logger_kernel_extra, "Error al aumentar instancias del recurso.");
        manejar_exit(PCB);
    }
    free(nombre_recurso); ///
} 