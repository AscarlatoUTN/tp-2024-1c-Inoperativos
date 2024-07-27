#include "../include/ciclo_instrucciones.h"

// Declaración de las variables
char* reg_origen;
char* reg_destino;
char* nombre_interfaz;
char* unidades_de_trabajo;
char* reg_direccion;
char* reg_tamanio;
char* nombre_archivo;
char* recurso;
char* ptr_arch;
int i = 0;

t_instruccion_MAP instruccionMap[] = {
    {I_SET, "SET"}, {I_SUM, "SUM"},
    {I_SUB, "SUB"}, {I_JNZ, "JNZ"},
    {I_MOV_IN, "MOV_IN"},{I_MOV_OUT, "MOV_OUT"},
    {I_RESIZE, "RESIZE"},{I_COPY_STRING, "COPY_STRING"},
    {I_WAIT, "WAIT"},{I_SIGNAL, "SIGNAL"},
    {I_EXIT, "EXIT"},{I_IO_GEN_SLEEP, "IO_GEN_SLEEP"},
    {I_IO_STDIN_READ, "IO_STDIN_READ"},
    {I_IO_STDOUT_WRITE, "IO_STDOUT_WRITE"},
    {I_IO_FS_CREATE, "IO_FS_CREATE"},
    {I_IO_FS_DELETE, "IO_FS_DELETE"},
    {I_IO_FS_TRUNCATE, "IO_FS_TRUNCATE"},
    {I_IO_FS_WRITE, "IO_FS_WRITE"},
    {I_IO_FS_READ, "IO_FS_READ"},
    {0, NULL}
};

t_pcb* pedir_instrucciones(t_pcb* pcb){

    while(hubo_interrupcion == NOT_INTERRUPT){        
        t_pedir_instruccion* instruccion = malloc(sizeof(t_pedir_instruccion));
        instruccion->PID = pcb->PID; 
        instruccion->PC = pcb->PC; 

        // Pedimos la instrucción a la memoria
        pedir_instruccion(instruccion, conexion_memoria, SGTE_INSTRUCC);

        // Recibimos la instrucción
        char* instruccion_recibida = guardar_mensaje(conexion_memoria);

        log_info(logger_cpu, "PID: %d - FETCH - Program Counter: %d \n", pcb->PID, pcb->PC);

        // Compara el cod_op con cada uno de los cod_op creados en instruccionMap[]
        t_instruccion_recibida* recibirInstruccion = procesar_instruccion(instruccion_recibida);

        int espacios = contadorEspacios(instruccion_recibida);

        if(!recibirInstruccion->es_valido){
            log_error(logger_cpu_extra, "Instruccion desconocida: %s \n", instruccion_recibida);
            free(instruccion_recibida);
            free(instruccion); 
            free_instruccion_recibida(recibirInstruccion, espacios);
            flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
            execute_EXIT(pcb);
            return NULL;
        }

        free(instruccion_recibida);
        free(instruccion); 
        
        //muestra la instruccion recibida y sus parametros 
        log_info(logger_cpu, "PID: %d - Ejecutando: %s - \n", pcb->PID, instruccionMap[recibirInstruccion->tipo_instruccion].nombre_instruccion); 
        while(i < espacios){
            log_info(logger_cpu_extra, "%s", recibirInstruccion->argumentos[i]);
            i++;
        }
        pcb->PC++; // Actualizamos PC
        
        // Ejecutar la instrucción correspondiente
        switch (recibirInstruccion->tipo_instruccion) {
            case I_SET:
                
                if(execute_SET(pcb, recibirInstruccion->argumentos[0], atoi(recibirInstruccion->argumentos[1]), recibirInstruccion, espacios) == 1)
                    return NULL;
                    
                break;
            case I_SUM:
                
                if(execute_SUM(pcb, recibirInstruccion->argumentos[0], recibirInstruccion->argumentos[1], recibirInstruccion, espacios) == 1)
                    return NULL;
                    
                break;
            case I_SUB:
                
                if(execute_SUB(pcb, recibirInstruccion->argumentos[0], recibirInstruccion->argumentos[1], recibirInstruccion, espacios) == 1)
                    return NULL;
                    
                break;
            case I_JNZ:

                if(execute_JNZ(pcb, recibirInstruccion->argumentos[0], recibirInstruccion->argumentos[1], recibirInstruccion, espacios) == 1)
                    return NULL;

                break;
            case I_MOV_IN:
                            
                reg_destino = recibirInstruccion->argumentos[0]; //Registro datos
                reg_origen = recibirInstruccion->argumentos[1]; //Registro direccion
                if (execute_MOV_IN(pcb, reg_destino,  reg_origen, recibirInstruccion, espacios) == 1)
                    return NULL;
                
                break;
            case I_MOV_OUT:
                
                reg_destino = recibirInstruccion->argumentos[0]; //Registro direccion
                reg_origen = recibirInstruccion->argumentos[1]; //Registro datos
                if (execute_MOV_OUT(pcb, reg_origen, reg_destino, recibirInstruccion, espacios) == 1)
                    return NULL;

                break;
            case I_RESIZE:
    
                int tamanio_resize = atoi(recibirInstruccion->argumentos[0]);
                if (execute_RESIZE(pcb, tamanio_resize, recibirInstruccion, espacios) == 1)
                    return NULL;

                break;
            case I_COPY_STRING:
                    
                int tamanio_string = atoi(recibirInstruccion->argumentos[0]);
                execute_COPY_STRING(pcb, tamanio_string, recibirInstruccion, espacios);
                
                break;
            case I_WAIT:

                execute_recursos(M_WAIT, pcb, recibirInstruccion->argumentos[0]);  
                free_instruccion_recibida(recibirInstruccion, espacios); 
                liberar_pcb(pcb);

                return NULL;            
            case I_SIGNAL:

                execute_recursos(M_SIGNAL, pcb, recibirInstruccion->argumentos[0]); 
                free_instruccion_recibida(recibirInstruccion, espacios);
                liberar_pcb(pcb);

                return NULL;   
            case I_EXIT:

                execute_EXIT(pcb);

                free(recibirInstruccion->argumentos);
                free(recibirInstruccion);

                return NULL;
            case I_IO_GEN_SLEEP:
                
                nombre_interfaz = recibirInstruccion->argumentos[0];
                unidades_de_trabajo = recibirInstruccion->argumentos[1];
                if (execute_IO_GEN_SLEEP(pcb, nombre_interfaz, unidades_de_trabajo)==0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                };

                break;
            case I_IO_STDIN_READ:
                
                nombre_interfaz = recibirInstruccion->argumentos[0];
                reg_direccion = recibirInstruccion->argumentos[1];
                reg_tamanio = recibirInstruccion->argumentos[2];
                if (execute_IO_STDIN_READ(pcb, nombre_interfaz, reg_direccion, reg_tamanio)==0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                };
                break;
            case I_IO_STDOUT_WRITE:
                
                nombre_interfaz = recibirInstruccion->argumentos[0];
                reg_direccion = recibirInstruccion->argumentos[1];
                reg_tamanio = recibirInstruccion->argumentos[2]; 
                if (execute_IO_STDOUT_WRITE(pcb, nombre_interfaz, reg_direccion, reg_tamanio)==0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                };
        
                break;
            case I_IO_FS_CREATE:

                nombre_interfaz = recibirInstruccion->argumentos[0];
                nombre_archivo = recibirInstruccion->argumentos[1];

                if (OP_FS(pcb, nombre_interfaz, nombre_archivo, NULL, M_IO_FS_CREATE) == 0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                }
                
                break;
            case I_IO_FS_DELETE:
                // Asignar el valor a los parametros
                nombre_interfaz = recibirInstruccion->argumentos[0];
                nombre_archivo = recibirInstruccion->argumentos[1];

                if (OP_FS(pcb, nombre_interfaz, nombre_archivo, NULL, M_IO_FS_DELETE) == 0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                }

                break;
            case I_IO_FS_TRUNCATE:

                nombre_interfaz = recibirInstruccion->argumentos[0];
                nombre_archivo = recibirInstruccion->argumentos[1];
                reg_tamanio = recibirInstruccion->argumentos[2];

                if (OP_FS(pcb, nombre_interfaz, nombre_archivo, reg_tamanio, M_IO_FS_TRUNCATE) == 0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                }

                break;
            case I_IO_FS_WRITE:
                // Asignar el valor a los parametros
                nombre_interfaz = recibirInstruccion->argumentos[0];
                nombre_archivo = recibirInstruccion->argumentos[1];
                reg_direccion = recibirInstruccion->argumentos[2];
                reg_tamanio = recibirInstruccion->argumentos[3];
                ptr_arch = recibirInstruccion->argumentos[4];
                
                int result = FS_RW(pcb, nombre_interfaz, reg_tamanio, ptr_arch, reg_direccion, nombre_archivo, M_IO_FS_WRITE);

                if ( result == 0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                }else if (result == UINT32_MAX){
                    execute_EXIT (pcb);
                    free(recibirInstruccion->argumentos);
                    free(recibirInstruccion);
                    return NULL;
                }
                
                break;
            case I_IO_FS_READ:
                
                nombre_interfaz = recibirInstruccion->argumentos[0];
                nombre_archivo = recibirInstruccion->argumentos[1];
                reg_direccion = recibirInstruccion->argumentos[2];
                reg_tamanio = recibirInstruccion->argumentos[3];
                ptr_arch = recibirInstruccion->argumentos[4];

                // Procedemos a IO_FS_READ
                int resultado = FS_RW(pcb, nombre_interfaz, reg_tamanio, ptr_arch, reg_direccion, nombre_archivo, M_IO_FS_READ);

                if (resultado == 0){
                    if (hubo_interrupcion == NOT_INTERRUPT)
                        liberar_pcb(pcb);
                    free_instruccion_recibida(recibirInstruccion, espacios);
                    return NULL;
                }else if (resultado == UINT32_MAX){
                    execute_EXIT (pcb);
                    free(recibirInstruccion->argumentos);
                    free(recibirInstruccion);
                    return NULL;
                }
            
                break;
            default:
                log_error(logger_cpu_extra, "Instruccion desconocida switch case: %p \n", recibirInstruccion);
                exit(EXIT_FAILURE);
        }
    free_instruccion_recibida(recibirInstruccion, espacios);
    }
    
    return pcb;
}


// Procesa una instrucción y devuelve una estructura con la información de la instrucción
t_instruccion_recibida* procesar_instruccion(char* instruccion) {
    // Contamos la cantidad de espacios en la instrucción para saber cuántos argumentos tiene
    int espacios = contadorEspacios(instruccion);
    // Dividimos la instrucción en un vector de strings usando los espacios como separadores
    char** vector_instruccion = string_n_split(instruccion, espacios + 1, " ");

    t_instruccion_recibida* instruccion_recibida = malloc(sizeof(t_instruccion_recibida));

    // Recorremos el mapa de instrucciones para encontrar la instrucción correspondiente
    for(uint8_t i = 0; instruccionMap[i].nombre_instruccion != NULL; i++) {
        // Si encontramos la instrucción en el mapa
        if(strcmp(vector_instruccion[0], instruccionMap[i].nombre_instruccion) == 0) {
            // Asignamos el tipo de instrucción y marcamos la instrucción como válida
            instruccion_recibida->tipo_instruccion = instruccionMap[i].tipo_instruccion;            
            instruccion_recibida->es_valido = true;
            // Reservamos memoria para los argumentos de la instrucción
            instruccion_recibida->argumentos = malloc(sizeof(char*) * espacios);

            // Copiamos los argumentos de la instrucción al vector de argumentos
            for (int j = 0; j < espacios; j++){
                instruccion_recibida->argumentos[j] = strdup(vector_instruccion[j+1]);
            } 

            string_array_destroy(vector_instruccion);

            // Devolvemos la instrucción procesada
            return instruccion_recibida;
        }
    }
    // Si no encontramos la instrucción en el mapa, registramos un error y marcamos la instrucción como inválida
    log_error(logger_cpu_extra, "Instruccion desconocida procesar_instr: %s \n", vector_instruccion[0]);
    instruccion_recibida->es_valido = false;
    string_array_destroy(vector_instruccion);
    return instruccion_recibida;
}

// Esta función cuenta la cantidad de espacios en una cadena
int contadorEspacios(char* instruccion) {
    int espacios = 0;
    
    // Recorremos la cadena y contamos los espacios
    for(int i = 0; i < strlen(instruccion); i++){
        if(instruccion[i] == ' '){
            espacios++;
        }
    }
    return espacios;
}

// Esta función libera la memoria utilizada por una instrucción
void free_instruccion_recibida(t_instruccion_recibida* instruccion_recibida, int espacios) {
    // Liberamos la memoria del vector de argumentos
    for(int i = 0; i < espacios; i++){
        free(instruccion_recibida->argumentos[i]);
    }
    free(instruccion_recibida->argumentos);
    free(instruccion_recibida);
}

// ------------------ Ejecutar Instrucciones --------------------

int execute_SET(t_pcb* pcb, char* registro, int valor, t_instruccion_recibida* recibirInstruccion, int espacios) {
    if (SET(pcb, registro, &valor) == 1) {
        log_error(logger_cpu_extra, "Error en SET. Finalizando proceso... \n");
        enviar_pcb(conexion_kernel_dispatch, pcb, M_EXIT);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}

// Función para ejecutar una instrucción SUM
int execute_SUM(t_pcb* pcb, char* reg_destino, char* reg_origen, t_instruccion_recibida* recibirInstruccion, int espacios) {
    if (SUM(pcb, reg_destino, reg_origen) == 1) {
        log_error(logger_cpu_extra, "Error en SUM. Finalizando proceso... \n");
        enviar_pcb(conexion_kernel_dispatch, pcb, M_EXIT);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}

// Función para ejecutar una instrucción SUB
int execute_SUB(t_pcb* pcb, char* reg_destino, char* reg_origen, t_instruccion_recibida* recibirInstruccion, int espacios) {
    if (SUB(pcb, reg_destino, reg_origen) == 1) {
        log_error(logger_cpu_extra, "Error en SUB. Finalizando proceso... \n");
        enviar_pcb(conexion_kernel_dispatch, pcb, M_EXIT);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}

// Función para ejecutar una instrucción JNZ
int execute_JNZ(t_pcb* pcb, char* reg_destino, char* reg_origen, t_instruccion_recibida* recibirInstruccion, int espacios) {
    if (JNZ(pcb, reg_destino, reg_origen) == 1) {
        log_error(logger_cpu_extra, "Error en JNZ. Finalizando proceso... \n");
        enviar_pcb(conexion_kernel_dispatch, pcb, M_EXIT);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}

// Función para ejecutar una instrucción EXIT
int execute_EXIT(t_pcb* pcb) {
    log_info(logger_cpu_extra, "Instruccion EXIT recibida. Finalizando proceso... \n");
    enviar_pcb(conexion_kernel_dispatch, pcb, M_EXIT);
    flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
    return 0;
}


// Función para ejecutar una instrucción EXIT
int execute_OOM(t_pcb* pcb) {
    log_info(logger_cpu_extra, "Out of Memory: Finalizando proceso... \n");
    enviar_pcb(conexion_kernel_dispatch, pcb, M_OOM);
    flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
    return 0;
}


// Función para ejecutar una instrucción IO_GEN_SLEEP
int execute_IO_GEN_SLEEP(t_pcb* pcb, char* nombre_interfaz, char* unidades_de_trabajo) {
    // Procedemos a IO_GEN_SLEEP
    if (IO_GEN_SLEEP(pcb, nombre_interfaz, unidades_de_trabajo) == 0)
        return 0;
    return 1;
}

void execute_recursos(header m_header, t_pcb* pcb, char* recurso) {
    enviar_pcb_rec* pcb_rec = malloc(sizeof(enviar_pcb_rec));
    pcb_rec->length_nombre_recurso = strlen(recurso) + 1;
    pcb_rec->nombre_recurso = strdup(recurso);
    pcb_rec->PCB = pcb;
    pthread_mutex_lock(&mutex_interrupcion);
    if (hubo_interrupcion != INTERRUPT_EXIT){
        pthread_mutex_unlock(&mutex_interrupcion);
        enviar_pcb_recurso(m_header, pcb_rec, conexion_kernel_dispatch);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
    }
    pthread_mutex_unlock(&mutex_interrupcion);
    free(pcb_rec->nombre_recurso);
    free(pcb_rec);
}

uint32_t execute_IO_STDIN_READ(t_pcb* pcb, char* nombre_interfaz, char* reg_direccion, char* reg_tamanio) {
    return F_IO_STD(pcb, nombre_interfaz, reg_direccion, reg_tamanio, M_IO_STDIN);
}

uint32_t execute_IO_STDOUT_WRITE(t_pcb* pcb, char* nombre_interfaz, char* reg_direccion, char* reg_tamanio) {
    return F_IO_STD(pcb, nombre_interfaz, reg_direccion, reg_tamanio, M_IO_STDOUT);
}

int execute_MOV_IN(t_pcb* pcb, char* registro_datos, char* reg_direccion, t_instruccion_recibida* recibirInstruccion, int espacios) {
    // Procedemos a MOV_IN
    if (MOV_IN(pcb, registro_datos, reg_direccion) == -1){
        log_error(logger_cpu_extra, "Error en MOV_IN. Finalizando proceso... \n");
        execute_EXIT(pcb);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}

int execute_MOV_OUT(t_pcb* pcb, char* reg_destino, char* reg_origen, t_instruccion_recibida* recibirInstruccion, int espacios) {
    // Procedemos a MOV_IN
    if (MOV_OUT(pcb, reg_destino, reg_origen) == -1){
        log_error(logger_cpu_extra, "Error en MOV_OUT. Finalizando proceso... \n");
        execute_EXIT(pcb);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}

int execute_RESIZE(t_pcb* pcb, int tamanio_resize, t_instruccion_recibida* recibirInstruccion, int espacios) { 

    int result = RESIZE(pcb, tamanio_resize);

    if(result == ERROR) {
        log_error(logger_cpu_extra, "Error Desconocido: RESIZE - PID: %d \n", pcb->PID);
        exit(EXIT_FAILURE);

    } else if(result == OK){
        log_info(logger_cpu_extra, "RESIZE realizado correctamente - PID: %d \n", pcb->PID);

    } else if(result == OOM){
        log_error(logger_cpu_extra, "Out of Memory - PID: %d \n", pcb->PID);
        execute_OOM(pcb); 
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}

int execute_COPY_STRING(t_pcb* pcb, int tamanio_string, t_instruccion_recibida* recibirInstruccion, int espacios){
    
    if (COPY_STRING(pcb, tamanio_string) ==-1){
        log_error(logger_cpu_extra, "Error en COPY_STRING. Finalizando proceso... \n");
        execute_EXIT(pcb);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        free_instruccion_recibida(recibirInstruccion, espacios); 
        return 1;
    }
    return 0;
}
