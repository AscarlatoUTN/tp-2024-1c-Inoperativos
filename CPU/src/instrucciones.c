#include "../include/instrucciones.h"

int sumar = 0;
int restar = 1;
int OOM = 1;

// -------------------------SET-------------------------
int SET(t_pcb* pcb, char* registro, void* valor) {
    void* reg = obtener_registro(pcb, registro);
    if (reg == NULL) {
        log_error(logger_cpu_extra, "Registro para SET desconocido: %s \n", registro);
        return 1;
    }

    size_t size = (registro[0] == 'E' || registro[0] == 'P' || registro[1] == 'I') ? sizeof(uint32_t) : sizeof(uint8_t);
    return setear_valor(reg, valor, size);
}

// -------------------------SUM-------------------------
int SUM(t_pcb* pcb,  char* registro_destino, char* registro_origen) {
    // Llamar a la función de operación aritmética con operación de sumar (0)
    return op_artimetica(pcb, registro_destino, registro_origen, sumar);
}

// -------------------------SUB-------------------------
int SUB(t_pcb* pcb, char* registro_destino, char* registro_origen) {
    // Llamar a la función de operación aritmética con operación de resta (1)
    return op_artimetica(pcb, registro_destino, registro_origen, restar);
}

// -------------------------JNZ-------------------------
int JNZ(t_pcb *pcb, char* registro_destino, char* salto) {
    void* reg_destino = obtener_registro(pcb, registro_destino);
    if (reg_destino == NULL) {
        log_error(logger_cpu_extra, "Registro JNZ desconocido: %s \n", registro_destino);
        return 1;
    }

    uint32_t valor_destino = (registro_destino[0] == 'E') ? *(uint32_t*)reg_destino : (uint32_t)*(uint8_t*)reg_destino;
    uint32_t jump = atoi(salto);

    if (valor_destino != 0) {
        pcb->PC = jump;
    } else {
        log_error(logger_cpu_extra, "No se cumple la condición. Continuando con la ejecución. \n");
    }
    return 0;
}

// ------------------------- MOV_IN -------------------------

int MOV_IN(t_pcb* pcb, char* registro_datos, char* registro_direccion) {
    t_tabla_r* tabla = malloc(sizeof(t_tabla_r));
    if (!tabla) {
        log_error(logger_cpu_extra, "Error al asignar memoria para la tabla.\n");
        return -1;
    }

    // Obtener los registros para los datos y la dirección
    void* reg_datos = obtener_registro(pcb, registro_datos);
    void* reg_direccion = obtener_registro(pcb, registro_direccion);

    if (!reg_datos || !reg_direccion) {
        log_error(logger_cpu_extra, "Error al obtener registros.\n");
        free(tabla);
        return -1;
    }

    // Determinar el tamaño a leer basado en el registro de datos
    size_t size = (registro_datos[0] == 'E' || registro_datos[0] == 'P' || registro_datos[1] == 'I') ? sizeof(uint32_t) : sizeof(uint8_t);

    size_t direc_logica = (registro_direccion[0] == 'E' || registro_direccion[0] == 'P' || registro_direccion[1] == 'I') ? *(uint32_t*)reg_direccion : *(uint8_t*)reg_direccion;
 
    uint32_t desplazamiento = direc_logica % tam_pagina;
    // Obtener las direcciones físicas necesarias
    t_list* df = list_dfs(pcb, direc_logica, size, desplazamiento);
    if (!df) {
        log_error(logger_cpu_extra, "Error al obtener las direcciones físicas o lista vacía.\n");
        free(tabla);
        return -1;
    }

    // Configurar los valores de tabla
    tabla->offset_size = desplazamiento;
    tabla->PID = pcb->PID;

    // Leer de memoria
    char* dato_leido = leer_memoria(logger_cpu, df, tabla, conexion_memoria, tam_pagina, size, LECTURA);
    if (!dato_leido) {
        log_error(logger_cpu_extra, "Error al leer de memoria.\n");
        list_destroy_and_destroy_elements(df, free);
        free(tabla);
        return -1;
    }
    

    // Liberar memoria utilizada
    free(dato_leido);
    list_destroy_and_destroy_elements(df, free);
    free(tabla);

    return 0;
}

// ------------------------- MOV_OUT --------------------------

int MOV_OUT(t_pcb* pcb, char* registro_datos, char* registro_direccion) {

    // Obtener los registros para los datos y la dirección
    void* reg_datos = obtener_registro(pcb, registro_datos);
    void* reg_direccion = obtener_registro(pcb, registro_direccion);

    if (!reg_datos || !reg_direccion) {
        log_error(logger_cpu_extra, "Error al obtener registros.\n");
        return -1;
    }

    size_t tamanio = (registro_datos[0] == 'E' || registro_datos[0] == 'P' || registro_datos[1] == 'I') ? sizeof(uint32_t) : sizeof(uint8_t);

    size_t direc_logica = (registro_direccion[0] == 'E' || registro_direccion[0] == 'P' || registro_direccion[1] == 'I') ? *(uint32_t*)reg_direccion : *(uint8_t*)reg_direccion;

    uint32_t desplazamiento = direc_logica % tam_pagina;
    
    // Obtener la lista de direcciones físicas
    t_list *df = list_dfs(pcb, direc_logica, tamanio, desplazamiento);
    if (!df) {
        log_error(logger_cpu_extra, "Error al obtener las direcciones físicas.\n");
        return -1;
    }

    // Escribir los datos en la memoria
    int result = escribir_memoria(logger_cpu, df, pcb->PID, conexion_memoria, reg_datos, tamanio, desplazamiento, tam_pagina, ESCRITURA);

    if(result == OK) {
        list_destroy_and_destroy_elements(df, &free); 
        return 0;
    }
    else if (result == ERROR){
        log_error(logger_cpu_extra, "Error en la escritura del dato en memoria \n");
        list_destroy_and_destroy_elements(df, &free); 
        return -1;
    }
    
    return 0;
}

// ------------------------- IO_GEN_SLEEP ------------------------
int IO_GEN_SLEEP(t_pcb *pcb, char *nombre_interfaz, char *unidades_de_trabajo)
{

    interfaz_a_enviar_gen *interfaz = malloc(sizeof(interfaz_a_enviar_gen));
    interfaz->length_nombre_interfaz = strlen(nombre_interfaz) + 1;
    interfaz->nombre_interfaz = strdup(nombre_interfaz);
    interfaz->unidades_trabajo = atoi(unidades_de_trabajo);

    interfaz->PCB = pcb;
    pthread_mutex_lock(&mutex_interrupcion);
    if(hubo_interrupcion != INTERRUPT_EXIT){
        pthread_mutex_unlock(&mutex_interrupcion);
        enviar_interfaz_io_gen(M_IO_GEN, interfaz, conexion_kernel_dispatch);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        // Liberamos memoria
        free(interfaz->nombre_interfaz);
        free(interfaz);
        return 0;
        
    }
    pthread_mutex_unlock(&mutex_interrupcion);
    // Liberamos memoria
    free(interfaz->nombre_interfaz);
    free(interfaz);
    return 1;
}

// ------------------------- IO_STD -------------------------------

uint32_t F_IO_STD(t_pcb* pcb, char* nombre_interfaz, char* registro_direccion, char* registro_tamanio, header motivo_desalojo){
    interfaz_a_enviar_std* interfaz = malloc(sizeof(interfaz_a_enviar_std)); 

    void* reg_tamanio = obtener_registro(pcb, registro_tamanio); // El tamanio a leer
    void* reg_direccion = obtener_registro(pcb, registro_direccion);
   
    uint32_t tamanio = (registro_tamanio[0] == 'E' || registro_tamanio[0] == 'P' || registro_tamanio[1] == 'I') ? *(uint32_t*)reg_tamanio : *(uint8_t*)reg_tamanio;

    log_info (logger_cpu_extra, "Tamanio a leer: %d \n", tamanio);
    
    size_t direc_logica = (registro_direccion[0] == 'E' || registro_direccion[0] == 'P' || registro_direccion[1] == 'I') ? *(uint32_t*)reg_direccion : *(uint8_t*)reg_direccion;
    
    uint32_t desplazamiento = direc_logica % tam_pagina; // Calcular el desplazamiento dentro de la pagina

    interfaz->list_df = list_dfs(pcb, direc_logica, tamanio, desplazamiento); // Devuelve la lista de direcciones fisicas requeridas
    interfaz->list_df_size = list_size(interfaz->list_df);
    interfaz->reg_tamanio = tamanio; 
    interfaz->offset = desplazamiento; 
       
    // Inicializar el resto de los parámetros de la interfaz
    interfaz->nombre_interfaz = strdup(nombre_interfaz);
    interfaz->length_nombre_interfaz = strlen(nombre_interfaz) + 1;
    interfaz->PCB = pcb;

    pthread_mutex_lock(&mutex_interrupcion);
    if(hubo_interrupcion != INTERRUPT_EXIT){
        pthread_mutex_unlock(&mutex_interrupcion);
        enviar_parametros_interfaz_std(motivo_desalojo, interfaz, conexion_kernel_dispatch);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        return 0;
    }
    pthread_mutex_unlock(&mutex_interrupcion);
    
    // Liberar memoria

    free(interfaz->nombre_interfaz);
    list_destroy_and_destroy_elements (interfaz->list_df, &free);
    free(interfaz); //Revisar
   return 1;
}

// ------------------------- RESIZE -------------------------------

int RESIZE (t_pcb* pcb, int tamanio_resize){
    t_resize* resize = malloc(sizeof(t_resize));
    if (!resize) {
        log_error(logger_cpu_extra, "Error al asignar memoria para la tabla.\n");
        return -1;
    }
    
    resize->cant_pags = ceil((double) tamanio_resize / tam_pagina);  
    resize->PID = pcb->PID;
    resize->tam_resize = tamanio_resize;

    enviar_pedido_resize(AJUSTAR_TAMANIO_PROCESO, conexion_memoria, resize);
    recv(conexion_memoria, &result, sizeof(int), MSG_WAITALL);
    free (resize);
    return result;
}


// ------------------------- COPY_STRING -------------------------------

int COPY_STRING(t_pcb* pcb, int tamanio_string) {
    t_tabla_r* tabla = malloc (sizeof(t_tabla_r)); 

    if (!tabla) {
        log_error(logger_cpu_extra, "Error al asignar memoria para la tabla.\n");
        return -1;
    }

    uint32_t reg_SI = *(uint32_t*)obtener_registro(pcb, "SI");
    uint32_t reg_DI = *(uint32_t*)obtener_registro(pcb, "DI");

    // Calcular el desplazamiento dentro de la pagina para SI
    uint32_t desplazamiento = reg_SI % tam_pagina; 

    // Obtener las direcciones físicas necesarias
    t_list* df_SI = list_dfs(pcb, reg_SI, tamanio_string, desplazamiento);
    if (!df_SI) {
        log_error(logger_cpu_extra, "Error al obtener las direcciones físicas o lista vacía.\n");
        free(tabla);
        return -1;
    }

    // Configurar los valores de tabla
    tabla->offset_size = desplazamiento;
    tabla->PID = pcb->PID;

    // Leer de memoria
    char* dato_leido = leer_memoria(logger_cpu, df_SI, tabla, conexion_memoria, tam_pagina, tamanio_string, LECTURA);
    if (dato_leido == NULL) {
        log_error(logger_cpu_extra, "Error al leer de memoria.\n");
        list_destroy_and_destroy_elements(df_SI, free);
        free(tabla);
        return -1;
    }

    // Calcular el desplazamiento dentro de la pagina para DI

    desplazamiento = reg_DI % tam_pagina;
    
    //Obtener las DF de DI
    t_list* df_DI = list_dfs(pcb, reg_DI, tamanio_string, desplazamiento);
    if (!df_DI) {
        log_error(logger_cpu_extra, "Error al obtener las direcciones físicas.\n");
        return -1;
    }

    // Copiar el dato leído a la posición apuntada por DI
    int result = escribir_memoria(logger_cpu, df_DI, pcb->PID, conexion_memoria, dato_leido, tamanio_string, desplazamiento, tam_pagina, ESCRITURA);
    
    if(result == OK) {
        free(dato_leido); 
        list_destroy_and_destroy_elements(df_SI, &free);
        list_destroy_and_destroy_elements(df_DI, &free);
        free(tabla);//Revisar      
        return 0;
    }
    else if (result == ERROR){
        free(dato_leido); 
        list_destroy_and_destroy_elements(df_SI, &free);
        list_destroy_and_destroy_elements(df_DI, &free);
        free(tabla);//Revisar
        return -1;
    }

    // Liberar memoria utilizada
    free(dato_leido);
    list_destroy_and_destroy_elements(df_SI, &free);
    list_destroy_and_destroy_elements(df_DI, &free);
    free(tabla);//Revisar

    return 0;
}

t_list* list_dfs(t_pcb* pcb, uint32_t registro1, uint32_t tamanio, uint32_t offset) {

    uint32_t num_pag = registro1 / tam_pagina; // Calcular el numero de pagina base 
    uint32_t espacio = tamanio + offset; // Calcular el espacio total necesario
    uint32_t cant_pags = ceil((double) espacio / tam_pagina); //Obtenemos la cant de pags a leer 

    t_list* df = list_create(); 
    if (df == NULL) {
        log_error(logger_cpu_extra, "Error al crear la lista. \n");
        return NULL;
    }

    for (int i = 0; i < cant_pags; i++) {
        int desp = (i == 0) ? offset : 0; // Ajustar el desplazamiento solo en la primera pagina
        int direc_fisica = traducir_direccion(pcb->PID, num_pag + i, desp);
        int* ptr_direc_fisica = malloc(sizeof(int));
        *ptr_direc_fisica = direc_fisica;
        if (direc_fisica == UINT32_MAX) {
            list_destroy_and_destroy_elements(df, &free); 
            return NULL; // Error en traduccion de direccion fisica
        }
        list_add(df, ptr_direc_fisica); // Agregar la direccion fisica a la lista
    }
    return df; 
}

//---------------------------FS_CREATE, FS_DELETE y FS_TRUNCATE--------------------------------------
uint32_t OP_FS(t_pcb* pcb, char* nombre_interfaz, char* nombre_archivo, char* registro_tamanio, header accion){
    uint32_t tamanio = 0;

    if(registro_tamanio != NULL) {
        void* reg_tamanio = obtener_registro(pcb, registro_tamanio); // tamanio a truncar
        tamanio = (registro_tamanio[0] == 'E' || registro_tamanio[0] == 'P' || registro_tamanio[1] == 'I') ? *(uint32_t*)reg_tamanio : *(uint8_t*)reg_tamanio;
        log_info (logger_cpu_extra, "Tamanio a truncar: %d \n", tamanio);
    } else
        tamanio = 0;
    
    interfaz_a_enviar_fs* interfaz = malloc(sizeof(interfaz_a_enviar_fs)); 
    interfaz->nombre_interfaz = strdup(nombre_interfaz);
    interfaz->length_nombre_interfaz = strlen(nombre_interfaz) + 1;
    interfaz->nombre_archivo = strdup(nombre_archivo);
    interfaz->length_nombre_archivo = strlen(nombre_archivo) + 1;
    interfaz->PCB = pcb;
    interfaz->reg_tamanio = tamanio;

    pthread_mutex_lock(&mutex_interrupcion);
    if(hubo_interrupcion != INTERRUPT_EXIT){
        pthread_mutex_unlock(&mutex_interrupcion);
        enviar_interfaz_fs(accion, interfaz, conexion_kernel_dispatch);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        return 0;
    }
    pthread_mutex_unlock(&mutex_interrupcion);
    

    free(interfaz);   
    return 1;
}
// ------------------------- IO_FS_WRITE - IO_FS_READ -----------------------------------------

uint32_t FS_RW(t_pcb* pcb, char* nombre_interfaz, char* registro_tamanio, char* puntero_archivo, char* registro_direccion, char* nombre_archivo, header tipo){
    
    // Obtener los valores de los registros
    void* reg_tamanio = obtener_registro(pcb, registro_tamanio);
    void* reg_direccion = obtener_registro(pcb, registro_direccion);
    void* reg_ptr_archivo = obtener_registro(pcb, puntero_archivo);

    parametros_fs_rw* parametros = malloc(sizeof(parametros_fs_rw));
    
    parametros->reg_tamanio = (registro_tamanio[0] == 'E' || registro_tamanio[0] == 'P' || registro_tamanio[1] == 'I') ? *(uint32_t*)reg_tamanio : *(uint8_t*)reg_tamanio;
    
    uint32_t direc_logica = (registro_direccion[0] == 'E' || registro_direccion[0] == 'P' || registro_direccion[1] == 'I') ? *(uint32_t*)reg_direccion : *(uint8_t*)reg_direccion;
    
    parametros->ptr_arch = (puntero_archivo[0] == 'E' || puntero_archivo[0] == 'P' || puntero_archivo[1] == 'I') ? *(uint32_t*)reg_ptr_archivo : *(uint8_t*)reg_ptr_archivo;

    uint32_t offset = direc_logica % tam_pagina; // Calcular el desplazamiento dentro de la página
    
    parametros->length_nombre_archivo = strlen(nombre_archivo) + 1;
    parametros->nombre_archivo = strdup(nombre_archivo);
    parametros->length_nombre_interfaz = strlen(nombre_interfaz) + 1;
    parametros->nombre_interfaz = strdup(nombre_interfaz);
    parametros->offset = offset;
 
    // Llamar a la función list_dfs
    parametros->lista_dfs = list_dfs(pcb, direc_logica, parametros->reg_tamanio, offset);
    parametros->lista_dfs_size= list_size(parametros->lista_dfs);
    parametros->PCB = pcb;
    
    if (parametros->lista_dfs == NULL) {
        log_error(logger_cpu_extra, "Error al obtener la lista de direcciones físicas.\n");
        free(parametros);
        return UINT32_MAX; // O algún código de error adecuado
    }

    pthread_mutex_lock(&mutex_interrupcion);
    if(hubo_interrupcion != INTERRUPT_EXIT){
        pthread_mutex_unlock(&mutex_interrupcion);
        enviar_param_fs_rw(tipo, parametros, conexion_kernel_dispatch);
        flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
        return 0;
    }
    pthread_mutex_unlock(&mutex_interrupcion);



    return 1;
}

// ------------------------- Definicion de Funciones Extra ------------------------
void* obtener_registro(t_pcb* pcb, char* registro) {
    if (registro[0] == 'E') {
        return obtener_regs32(pcb->reg, registro[1]);
    } else if (registro[1] == 'X') {
        return obtener_regs8(pcb->reg, registro[0]);
    } else if (registro[0] == 'P' || registro[1] == 'I') {
        return obtener_regs_especiales(pcb, registro[0]);
    }
    return NULL;
}

uint8_t* obtener_regs8(t_registros_gral* reg, char registro) {
    switch (registro) {
        case 'A': return &(reg->AX);
        case 'B': return &(reg->BX);
        case 'C': return &(reg->CX);
        case 'D': return &(reg->DX);
        default: return NULL;
    }
}

uint32_t* obtener_regs32(t_registros_gral* reg, char registro) {
    switch (registro) {
        case 'A': return &(reg->EAX);
        case 'B': return &(reg->EBX);
        case 'C': return &(reg->ECX);
        case 'D': return &(reg->EDX);
        default: return NULL;
    }
}

uint32_t* obtener_regs_especiales(t_pcb* pcb, char registro) {
    switch (registro) {
        case 'P': return &(pcb->PC);
        case 'S': return &(pcb->SI);
        case 'D': return &(pcb->DI);
        default: return NULL;
    }
}

int setear_valor(void* reg, void* valor, size_t size) {
    if (reg == NULL) {
        return 1; // Error
    }

    if (size == sizeof(uint32_t)) {
        *(uint32_t*)reg = *(uint32_t*)valor;
        printf("Valor seteado: %u\n", *(uint32_t*)reg);
    } else if (size == sizeof(uint8_t)) {
        *(uint8_t*)reg = *(uint8_t*)valor;
        printf("Valor seteado: %u\n", *(uint8_t*)reg);
    }

    return 0; // Éxito
}

int op_artimetica(t_pcb* pcb, char* registro_destino, char* registro_origen, int operacion) {
    void* reg_destino = obtener_registro(pcb, registro_destino);
    void* reg_origen = obtener_registro(pcb, registro_origen);

    if (reg_destino == NULL || reg_origen == NULL) {
        log_error(logger_cpu_extra, "Registro SUM o SUB desconocido: %s o %s \n", registro_destino, registro_origen);
        return 1;
    }

    size_t size_destino = (registro_destino[0] == 'E') ? sizeof(uint32_t) : sizeof(uint8_t);
    size_t size_origen = (registro_origen[0] == 'E') ? sizeof(uint32_t) : sizeof(uint8_t);
    
    return realizar_op_aritmetica(reg_destino, reg_origen, size_destino, size_origen, operacion);
}

int realizar_op_aritmetica(void* reg_destino, void* reg_origen, size_t size_destino, size_t size_origen, int op) {
    int mult = (op == 0) ? 1 : -1;

    if (size_destino == sizeof(uint32_t)) {
        *(uint32_t*)reg_destino += mult * ((size_origen == sizeof(uint32_t)) ? *(uint32_t*)reg_origen : (uint32_t)*(uint8_t*)reg_origen);
        log_info(logger_cpu_extra, "Resultado: %u\n", *(uint32_t*)reg_destino);
    } else if (size_destino == sizeof(uint8_t)) {
        *(uint8_t*)reg_destino += mult * ((size_origen == sizeof(uint32_t)) ? (uint8_t)*(uint32_t*)reg_origen : *(uint8_t*)reg_origen);
        log_info(logger_cpu_extra, "Resultado: %u\n", *(uint8_t*)reg_destino);
    } else {
        return 1; // Error en el tamaño
    }
    return 0;
}
