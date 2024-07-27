#include "../include/planificacion.h"
// ---------------------- Variables globales -------------------------
static int lastPID = 0; // Para el PCB
int i = 0;
int offset = 0;
int hubo_interrupcion = 0;
int tiempo_ejecutado = 0;
int GET = 1;
int REMOVE = 0;
bool PLANIFICACION = true;
int IMPRIMIR = 1;
int SINCRO = 0;
int PID_EXEC = 0;
// ---------------------- Funciones por Consola (Planificador Largo Plazo) -------------------------

// ---------------- Iniciar Proceso
void iniciar_proceso(char* path) {
    
    if(path == NULL || strcmp(path, "")==0){
        log_warning(logger_kernel_extra, "Falta el path \n");
        return;
    }

    t_pcb* PCB = crear_pcb();
    if (PCB == NULL) {
        log_error(logger_kernel_extra, "Error al crear PCB \n");
        return;
    }
    ingresar_a_new(PCB);
    
    enviar_proceso_memoria(PCB, path);
    
    // Recibir el resultado de la operación en memoria
    if(recv(conexion_memoria, &result, sizeof(int), MSG_WAITALL) == -1){
        log_error(logger_kernel_extra, "Error al recibir el PATH del proceso en memoria \n");
        sacar_pcb_lista(cola_new, PCB->PID, &mutex_new);
    } else if (result == 0){ // Proceso creado correctamente en memoria
        log_info(logger_kernel, "Se crea el proceso %d en New \n", PCB->PID);
        sem_post(&sem_pcb_espera_ready);
    } else {
        log_error(logger_kernel_extra, "Error al crear el proceso %d \n", PCB->PID);
        sacar_pcb_lista(cola_new, PCB->PID, &mutex_new);
    }
}

// ---------------- Finalizar Proceso

void finalizar_proceso(char* PID_a_eliminar){
    if (PID_a_eliminar == NULL || strcmp (PID_a_eliminar, "") == 0){
        log_error(logger_kernel_extra, "Falta ingresar el PID \n");
        return;
    }
    detener_planificacion();
    flag_interrupcion(&mutex_interrupcion, INTERRUPT, &hubo_interrupcion);
    uint32_t PID = atoi(PID_a_eliminar);
    t_pcb* PCB_a_eliminar = buscar_pcb(PID);
    int check;
    
    if(PCB_a_eliminar == NULL){
        enviar_nums(INTR, conexion_cpu_interrupt, PID);
        recv (conexion_cpu_interrupt, &check, sizeof (int), MSG_WAITALL);
        if (check != OK)
            iniciar_planificacion();
        return;
    }
    log_info(logger_kernel, "Finaliza el proceso %d - Motivo: INTERRUPTED_BY_USER \n", PCB_a_eliminar->PID);
    manejar_exit(PCB_a_eliminar);
    iniciar_planificacion();
}

// Función para buscar un PCB por su PID en una lista específica
t_pcb* buscar_pcb_lista(t_list* lista, uint32_t PID) {
    if (list_is_empty(lista)) 
        return NULL;
    
    for (int i = 0; i < list_size(lista); i++) {
        t_pcb* PCB_a_eliminar = list_get(lista, i);
        if (PCB_a_eliminar->PID == PID) {
            return finish_pcb(lista, PID);
        }
    }

    return NULL;
}

// Función principal para buscar un PCB por su PID
t_pcb* buscar_pcb(uint32_t PID) {
    t_pcb* PCB_a_eliminar;

    // Buscar en listas principales
    PCB_a_eliminar = buscar_pcb_lista(cola_new, PID);
    if (PCB_a_eliminar != NULL) 
        return PCB_a_eliminar;

    PCB_a_eliminar = buscar_pcb_lista(cola_ready, PID);
    if (PCB_a_eliminar != NULL) 
        return PCB_a_eliminar;
    
    PCB_a_eliminar = buscar_pcb_lista(cola_aux_vrr, PID);
    if (PCB_a_eliminar != NULL) 
        return PCB_a_eliminar;

    for (int i = 0; i < list_size(recursos); i++) {
        recurso_t* recurso = list_get(recursos, i);

        PCB_a_eliminar = buscar_pcb_rec_int(recurso->nombre, PID);
        if (PCB_a_eliminar != NULL) 
            return PCB_a_eliminar; 
    }

    for (int i = 0; i < list_size(interfaces_gen); i++) {
        t_io_gen* interfaz = list_get(interfaces_gen, i);
        PCB_a_eliminar = buscar_pcb_rec_int(interfaz->nombre_interfaz, PID);
        if (PCB_a_eliminar != NULL) 
            return PCB_a_eliminar; 
    }

    for (int i = 0; i < list_size(interfaces_stdin); i++) {
        t_io_std* interfaz = list_get(interfaces_stdin, i);
        PCB_a_eliminar = buscar_pcb_rec_int(interfaz->nombre_interfaz, PID);
        if (PCB_a_eliminar != NULL) 
            return PCB_a_eliminar;
    }

    for (int i = 0; i < list_size(interfaces_stdout); i++) {
        t_io_std* interfaz = list_get(interfaces_stdout, i);
        PCB_a_eliminar = buscar_pcb_rec_int(interfaz->nombre_interfaz, PID);
        if (PCB_a_eliminar != NULL) 
            return PCB_a_eliminar;
    }

    for (int i = 0; i < list_size(interfaces_dialfs); i++) {
        t_io_df* interfaz = list_get(interfaces_dialfs, i);
        PCB_a_eliminar = buscar_pcb_rec_int(interfaz->nombre_interfaz, PID);
        if (PCB_a_eliminar != NULL) 
            return PCB_a_eliminar;
    }

    // Si no se encuentra en el sistema, devolver NULL
    return NULL;
}

t_pcb* buscar_pcb_rec_int(char* nombre, uint32_t PID) {
    t_list* cola_rec = get_cola_blocked(nombre);
    return buscar_pcb_lista(cola_rec, PID);
}

// ---------------- Detener Planificacion

void detener_planificacion() {
    if (!PLANIFICACION)
        return;

    sem_wait(&planificador_corto);
    sem_wait(&planificador_largo);

    pthread_mutex_lock(&mutex_new);
    pthread_mutex_lock(&mutex_ready);
    pthread_mutex_lock(&mutex_cola_aux_vrr);

    accion_int_rec(SINCRO, true);

    PLANIFICACION = false;
    log_info(logger_kernel_extra, "Planificación detenida \n");
}

// ---------------- Iniciar Planificacion

void iniciar_planificacion() {
    if (PLANIFICACION)
        return;

    pthread_mutex_unlock(&mutex_new);
    pthread_mutex_unlock(&mutex_ready);
    pthread_mutex_unlock(&mutex_cola_aux_vrr);

    accion_int_rec(SINCRO, false);

    PLANIFICACION = true;
    sem_post(&planificador_corto);
    sem_post(&planificador_largo);
    sem_post(&planificador_corto);
    sem_post(&planificador_largo);

    log_info(logger_kernel_extra, "Planificación reanudada \n");
}

void accion_int_rec(int accion, bool bloquear) {
    for (int i = 0; i < list_size(interfaces_gen); i++) {
        t_io_gen* interfaz = list_get(interfaces_gen, i);
        
        if(accion == IMPRIMIR) {
            imprimir_rec_int(interfaz->nombre_interfaz);
        } else if(accion == SINCRO) {
            pthread_mutex_t* mutex = obtener_mutex_diccionario(interfaz->nombre_interfaz);
            if(mutex == NULL){
                log_error(logger_kernel_extra, "Mutex no encontrado \n");
                return;
            }

            bloquear ? pthread_mutex_lock(mutex) : pthread_mutex_unlock(mutex);
        }  
    }

    for (int i = 0; i < list_size(interfaces_stdin); i++) {
        t_io_std* interfaz = list_get(interfaces_stdin, i);
        
        if(accion == IMPRIMIR) {
            imprimir_rec_int(interfaz->nombre_interfaz);
        }  else if(accion == SINCRO) {
            pthread_mutex_t* mutex = obtener_mutex_diccionario(interfaz->nombre_interfaz);
            if(mutex == NULL){
                log_error(logger_kernel_extra, "Mutex no encontrado \n");
                return;
            }
            bloquear ? pthread_mutex_lock(mutex) : pthread_mutex_unlock(mutex);
        }
    }

    for (int i = 0; i < list_size(interfaces_stdout); i++) {
        t_io_std* interfaz = list_get(interfaces_stdout, i);
        if(accion == IMPRIMIR) {
            imprimir_rec_int(interfaz->nombre_interfaz);
        }  else if(accion == SINCRO) {
            pthread_mutex_t* mutex = obtener_mutex_diccionario(interfaz->nombre_interfaz);
            if(mutex == NULL){
                log_error(logger_kernel_extra, "Mutex no encontrado \n");
                return;
            }
            bloquear ? pthread_mutex_lock(mutex) : pthread_mutex_unlock(mutex);
        }
    }

    for (int i = 0; i < list_size(interfaces_dialfs); i++) {
        t_io_df* interfaz = list_get(interfaces_dialfs, i);
        if(accion == IMPRIMIR) {
            imprimir_rec_int(interfaz->nombre_interfaz);
        }  else if(accion == SINCRO) {
            pthread_mutex_t* mutex = obtener_mutex_diccionario(interfaz->nombre_interfaz);
            if(mutex == NULL){
                log_error(logger_kernel_extra, "Mutex no encontrado \n");
                return;
            }
            bloquear ? pthread_mutex_lock(mutex) : pthread_mutex_unlock(mutex);
        }
    }

    for (int i = 0; i < list_size(recursos); i++) {
        recurso_t* recurso = list_get(recursos, i);

        if(accion == IMPRIMIR) {
            imprimir_rec_int(recurso->nombre);
        } else if(accion == SINCRO) {
            pthread_mutex_t* mutex = obtener_mutex_diccionario(recurso->nombre);
            if(mutex == NULL){
                log_error(logger_kernel_extra, "Mutex no encontrado \n");
                return;
            }
            bloquear ? pthread_mutex_lock(mutex) : pthread_mutex_unlock(mutex);
        }
    }
}

void imprimir_rec_int(char* nombre) {
    t_list* cola = get_cola_blocked(nombre);
    if(cola == NULL)
        return;
        
    size_t longitud_total = strlen("Block - ") + strlen(nombre) + 1; 
    char* mensaje = (char*)malloc(longitud_total);
    strcpy(mensaje, "Block - ");
    strcat(mensaje, nombre);

    listar_procesos(mensaje, cola);
    free(mensaje);

    }

void listar_procesos(char* nombre_estado, t_list* lista_procesos) {
    log_info(logger_kernel, "Cola %s: [", nombre_estado);
    if (!list_is_empty(lista_procesos)) {
        for (int i = 0; i < list_size(lista_procesos); i++) {
            t_pcb* PCB = list_get(lista_procesos, i);
            log_info(logger_kernel, "PID: %d", PCB->PID);
        }
    }
    log_info(logger_kernel, "]\n");
}

// ------------------ Funciones de Inicialización ----------------------
int generarPID() { // Genera un PID unico
    return ++lastPID; 
}

t_pcb* crear_pcb(){
    t_pcb* PCB = malloc(sizeof(t_pcb));
    if (PCB == NULL) return NULL;
    PCB->PID = generarPID();
    PCB->PC = 0;
    PCB->SI = 0;
    PCB->DI = 0;
    PCB->q_restante = QUANTUM;
    PCB->reg = init_registros();
    if (PCB->reg == NULL) {
        free(PCB);
        return NULL;
    }
    return PCB;
}

t_registros_gral* init_registros(){
    t_registros_gral* registros = malloc (sizeof(t_registros_gral));
    if (registros == NULL) return NULL;
    registros->AX = 0;
    registros->BX = 0;
    registros->CX = 0;
    registros->DX = 0;
    registros->EAX = 0;
    registros->EBX = 0;
    registros->ECX = 0;
    registros->EDX = 0;
    return registros;
}

// ------------------ Funciones de Conexion con Memoria ----------------------

// Creamos la estructura y le asignamos los valores que enviamos a memoria
proceso_memoria* crear_proceso_memoria(uint32_t PID, char* path){
	proceso_memoria* crear_proceso = malloc (sizeof (proceso_memoria));
    if (crear_proceso == NULL) return NULL;
	crear_proceso->PID = PID;
	crear_proceso->path = string_duplicate(path);
    if (crear_proceso->path == NULL) {
        free(crear_proceso);
        return NULL;
    }
	crear_proceso->length_path = strlen(path) + 1; // +1 Para el caracter nulo
    return crear_proceso;
}

// Enviamos el proceso a memoria
void enviar_proceso_memoria(t_pcb* PCB, char* path){

    proceso_memoria* proceso = crear_proceso_memoria(PCB->PID, path);    
    t_paquete* paquete = crear_paquete(CREAR_PROCESO);
    agregar_a_proceso(paquete, proceso);

    sem_wait(&sem_multiprogramacion);
    log_info(logger_kernel_extra, "Grado de Multiprogramación permite enviar proceso a MEMORIA \n");

    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

// ------------------ Funciones de Planificacion ----------------------

void planificador_largo_plazo(){

    while(true) {
        sem_wait(&sem_pcb_espera_ready);
        t_pcb* PCB = obtener_sgte_ready();
        manejar_ingreso_a_ready(PCB);
        sem_post (&sem_corto_plazo);

        // Detencion de planificacion
        if(!PLANIFICACION)
            sem_wait(&planificador_largo);
    }
}

void planificador_corto_plazo() {
    while(true){    
        sem_wait(&sem_corto_plazo); // Esperamos a que haya un proceso en READY
        t_pcb* PCB = obtener_sgte_pcb_exec();

        if (PCB != NULL) {
            PCB->p_status = EXEC;
            
            pthread_mutex_lock(&mutex_pid);
            PID_EXEC = PCB->PID;
            pthread_mutex_unlock(&mutex_pid);
            
            manejar_planificacion(PCB);
            sem_wait(&sem_exec); // Esperamos a que el CPU termine de ejecutar el proceso
        } else {
            log_error(logger_kernel_extra, "Error al obtener PCB de la cola READY");
            terminar_programa_kernel();
            exit(EXIT_FAILURE);
        }
        // Detencion de planificacion
        if(!PLANIFICACION)
            sem_wait(&planificador_corto);
    }
}

// ------------------ Funciones de Planificacion ----------------------

void planificar_quantum(t_pcb* PCB) { 
    t_temporal* timer = temporal_create(); // Crea y comienza el contador
    
	t_hilo_plani_quantum* args = init_args_hilo_plani_quantum(PCB);
	pthread_t hilo_plani_quantum; // Crear hilo para simular la ejecución de la rafaga de CPU

	enviar_pcb(conexion_cpu_dispatch, PCB, CTXT_EXEC); // Envía el PCB al CPU para su ejecución
	
    pthread_create(&hilo_plani_quantum, NULL,(void*)rafaga_cpu_interrupt,(void*) args); // Hilo que simula la ejecución del CPU
    sem_wait(&sem_Q);

    pthread_mutex_lock(&mutex_interrupcion); 
    if (hubo_interrupcion == NOT_INTERRUPT){
        pthread_mutex_unlock(&mutex_interrupcion); 
        
        pthread_cancel(hilo_plani_quantum); 
        temporal_stop(timer); // Para el timer cuando se recibe el desalojo de CPU
        actualizar_tiempo_exec(timer);
    }else if (hubo_interrupcion == INTERRUPT){
        pthread_mutex_unlock(&mutex_interrupcion);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
    } else {
        pthread_mutex_unlock(&mutex_interrupcion);
        log_error(logger_kernel_extra, "Error en la interrupción del proceso %d \n", PCB->PID);
    } 
    
    sem_post(&rr_exit);
    sem_post(&sem_tiempo_ejecutado);
	pthread_detach(hilo_plani_quantum);
    temporal_destroy(timer); // Destruye el contador 
}


// Si se termina el quantum interrumpe, sino dispatch lo termina antes.
void rafaga_cpu_interrupt(void* args){
    t_hilo_plani_quantum* hilo_args = (t_hilo_plani_quantum*) args;
    
    uint32_t PID = hilo_args->PID;
    uint32_t q_restante = hilo_args->q_restante;
   // free (hilo_args->pcb);
    free(hilo_args);
    usleep(q_restante); //Bloqueamos mientras se ejecuta el proceso
    sem_wait(&rr_exit);

	flag_interrupcion(&mutex_interrupcion, INTERRUPT, &hubo_interrupcion);
    enviar_nums(FIN_Q, conexion_cpu_interrupt, PID);
}

// Inicializar args del hilo
t_hilo_plani_quantum* init_args_hilo_plani_quantum(t_pcb* PCB){
    t_hilo_plani_quantum* args = malloc(sizeof(t_hilo_plani_quantum));
	args->PID = PCB->PID;
    args->q_restante = PCB->q_restante;
	return args;
}

// ---------------------- Liberar Estructuras -------------------------
void liberar_parametros_io_gen(interfaz_a_enviar_gen* int_params){
    free(int_params->nombre_interfaz);
    free(int_params);
}

void liberar_parametros_io_std(interfaz_a_enviar_std* int_params){
    list_destroy_and_destroy_elements(int_params->list_df, &free);
    free(int_params->nombre_interfaz);
    free(int_params);
}

// ---------------------------------------

void manejar_ingreso_a_ready(t_pcb* PCB) {
    ingresar_a_ready(NULL, PCB);
    log_info(logger_kernel, "Cola Ready: [");
    // Imprimir dentro de un while el PID de cada PCB que se encuentre en la cola ready

    pthread_mutex_lock(&mutex_ready);
    for (int i = 0; i < list_size(cola_ready); i++) {
        t_pcb* PCB = list_get(cola_ready, i);
        log_info(logger_kernel, "%d, ", PCB->PID);
    }
    pthread_mutex_unlock(&mutex_ready);
    log_info(logger_kernel, "]\n");
}

// Función para obtener el siguiente PCB en READY
t_pcb* obtener_sgte_pcb_exec() {
    t_pcb* PCB = NULL;
    if (!list_is_empty(cola_aux_vrr) && strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
        pthread_mutex_lock(&mutex_cola_aux_vrr);
        PCB = list_remove(cola_aux_vrr, 0);
        pthread_mutex_unlock(&mutex_cola_aux_vrr);
    } else if (!list_is_empty(cola_ready)){
        pthread_mutex_lock(&mutex_ready); 
        PCB = list_remove(cola_ready, 0); 
        pthread_mutex_unlock(&mutex_ready);
    }
    return PCB;
}

// Función para manejar la planificación del PCB según el algoritmo
void manejar_planificacion(t_pcb* PCB) {
    if (strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0) {
        enviar_pcb(conexion_cpu_dispatch, PCB, CTXT_EXEC);
    } else if (strcmp(ALGORITMO_PLANIFICACION, "RR") == 0 || strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
        planificar_quantum(PCB);
    }
}

// Funcion que agrega a la cola auxiliar de VRR segun el q restante PONER SIEMPRE QUE VUELVA A READY
void manejar_vrr(t_pcb* PCB_actualizado, t_list* cola_aux_vrr, pthread_mutex_t* mutex_cola_aux_vrr) {
    if (strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0 && PCB_actualizado->q_restante < QUANTUM) {
        agregar_a_cola_aux_vrr(PCB_actualizado, cola_aux_vrr, mutex_cola_aux_vrr);
    } else {
        PCB_actualizado->q_restante = QUANTUM;
        ingresar_a_ready(NULL, PCB_actualizado); // Se agrega al final de la lista, se le cambia el estado
    }
}

void actualizar_tiempo_exec(t_temporal* timer) {
    if (strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
       // pthread_mutex_lock(&mutex_tiempo_ejecutado);///
        tiempo_ejecutado = temporal_gettime(timer) * 1000; // Calcula el tiempo que estuvo en ejecución 
        //pthread_mutex_unlock(&mutex_tiempo_ejecutado);///
    }
}