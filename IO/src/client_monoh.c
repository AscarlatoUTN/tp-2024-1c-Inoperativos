#include "../include/client_monoh.h"

handshake HANDSHAKE;
size_t bytes;
int32_t result;
int32_t OK = 0;
int32_t ERROR = -1;
uint32_t tiempo_ms, tiempo_microseg;
uint32_t unidades_trabajo, unids;
uint32_t tam_pag = 0;
const int32_t PEDIDO_TAM_PAG = 1;
t_list* lista_fcbs;

// Función para realizar el handshake y obtener el tamaño de página si es necesario
void handshake_cliente_io(size_t bytes, handshake HANDSHAKE, int conexion, t_config* config, char* nombre_interfaz, ESTADO estado) {

    bytes = send(conexion, &HANDSHAKE, sizeof(int32_t), 0);
	bytes = recv(conexion, &result, sizeof(int32_t), MSG_WAITALL);

	if (result == OK) {
        if (HANDSHAKE == HANDSHAKE_IO_STDIN_MEMORIA || HANDSHAKE == HANDSHAKE_IO_STDOUT_MEMORIA || HANDSHAKE == HANDSHAKE_IO_DIALFS_MEMORIA) {
            log_info(logger_io_extra, "Handshake Memoria -> %s OK \n", nombre_interfaz);
            estado = (HANDSHAKE == HANDSHAKE_IO_STDIN_MEMORIA) ? NOT_READING : NOT_PRINTING;
            send(conexion, &PEDIDO_TAM_PAG, sizeof(int32_t), 0);
            recv(conexion, &tam_pag, sizeof(uint32_t), MSG_WAITALL);
            log_warning(logger_io_extra, "Recibí el tamaño de página: %d \n", tam_pag);
        } else {
            log_info(logger_io_extra, "Handshake Kernel -> %s OK \n", nombre_interfaz);
        }
    } else {
        log_error(logger_io_extra, "Handshake ERROR \n");
        terminar_programa(conexion,0, config);
        exit(EXIT_FAILURE);
    }
}

int establecer_conexion(void* arg) {
    t_conexion_args* args = (t_conexion_args*)arg;
    args->conexion = crear_conexion_client(args->ip, args->puerto);
    if (args->conexion == -1) {
        log_error(logger_io_extra, "Error al crear la conexión \n");
        terminar_programa(args->conexion, 0, NULL);
        exit(EXIT_FAILURE);
    } else {
        log_info(logger_io_extra, "Conexión establecida con el servidor \n");
    }
    return args->conexion;
}

// ------------------------------- IO GEN --------------------------------------

int conexion_dispatch_io_gen(char* io_serv_name, t_io_gen* interfaz){

    int kernel_socket = interfaz->fd_interfaz;

    while (true){

        int header = recibir_operacion(kernel_socket);
        switch(header){

            case CONFIRMACION:
                recibir_mensaje(kernel_socket, logger_io_extra);
                interfaz->estado = WORKING;
                break;          
            
            case UNIDS_TRABAJO:

                io_gen_pid* recibido = recibir_io_gen_pid(kernel_socket);    
                uint32_t unidades_trabajo = recibido->unidades_trabajo;

                log_info(logger_io, "PID: %d - Operacion: IO GEN SLEEP \n", recibido->PID);

                // 1. Convertir unidades de trabajo a ms
                tiempo_ms = interfaz->tiempo_unidad_trabajo * unidades_trabajo;
                tiempo_microseg = tiempo_ms * 1000;

                // 2. Actualizar el estado de la interfaz a sleeping
                log_info(logger_io_extra, "Interfaz %s durmiendo por %d ms \n", interfaz->nombre_interfaz, tiempo_ms);
                interfaz->estado = SLEEPING;

                // 3. Aplicar sleep por la cantidad de unidades de trabajo
                usleep(tiempo_microseg);

                // 4. Actualizar el estado de la interfaz a working
                interfaz->estado = WORKING;
                log_info(logger_io_extra,"Interfaz %s terminó de dormir \n", interfaz->nombre_interfaz);

                // 5. Avisar a KERNEL que la interfaz terminó de dormir
                enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, T_IO_GEN);
                free(recibido);
                break;
            case -1:
                log_error(logger_io_extra, "Cliente desconectado de %s... \n", io_serv_name);

                return 1; 
            default:
                log_error(logger_io_extra, "Algo anduvo mal en el server de %s. Cop: %d \n", io_serv_name, header);
                return 1;
        }
    }
}

//----------------------------------DIALFS-------------------------------------------------

int conexion_cliente_dialfs(t_io_df* interfaz){

    int kernel_socket = interfaz->fd_io_kernel;
    int mem_socket = interfaz->fd_io_mem;
    uint32_t bloque_base;
    char* nombre_archivo;
    while(true){
        int header = recibir_operacion(kernel_socket);
        
        // Retardo Obligatorio por cada Op de FS (1 tiempo_unidad_trabajo)
        tiempo_microseg = interfaz->tiempo_unidad_trabajo;
        log_info(logger_io_extra, "INTERFAZ: %s - Retardo de %d ms \n", interfaz->nombre_interfaz, interfaz->tiempo_unidad_trabajo);
        usleep(tiempo_microseg);

        switch(header){
            case CONFIRMACION:
                recibir_mensaje(kernel_socket, logger_io_extra);
                if(crear_archivo_bitmap() == NULL || crear_archivo_bloques() == NULL)
                    return 1;

                lista_fcbs = list_create();
                buscar_directorio();
                    
                log_info(logger_io_extra, "Archivos creados con éxito \n");
                break;
            case IO_FS_CREATE: 

                // 1. Recibir de memoria los parametros de la IO DIALFS
                parametros_fs* recibido = recibir_parametros_io_fs(kernel_socket);
                if(recibido == NULL) {
                    log_error(logger_io_extra, "Error al recibir datos de KERNEL \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(recibido->nombre_archivo);
                    free(recibido);
                    return 1;
                }

                // 2. Hacer los Logs Minimos y Obligatorios
                log_info(logger_io, "PID: %d - Operacion: IO FS CREATE \n", recibido->PID);
                log_info(logger_io, "PID: %d - Crear Archivo: %s \n", recibido->PID, recibido->nombre_archivo);
                
                nombre_archivo = strdup(recibido->nombre_archivo);
                
                // 3. Verficar si existe el archivo en el directorio
                if (archivo_duplicado(nombre_archivo)){
                    log_warning(logger_io_extra, "Archivo %s ya se encuentra en el directorio. \n", nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, IO_FS_CREATE);
                    free(nombre_archivo);
                    free(recibido->nombre_archivo);
                    free(recibido);
                    break;
                }

                // 4. Verificar si hay algun bloque libre
                bloque_base = actualizar_espacio_disco(nombre_archivo, IO_FS_CREATE);
                if(bloque_base == UINT32_MAX){
                    log_error(logger_io_extra, "No hay bloques libres para crear el archivo %s \n", nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(nombre_archivo);
                    free(recibido->nombre_archivo);
                    free(recibido);
                    break;
                }   
                // 5. Crear archivo (Si no hay errores)
                FS_CREATE(interfaz->path, nombre_archivo, bloque_base);
                
                log_info(logger_io_extra, "Archivo %s creado exitosamente \n", nombre_archivo);
                // 6. Enviar mensaje de confirmacion a Kernel
                enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, IO_FS_CREATE);

                free(nombre_archivo);
                free(recibido->nombre_archivo);
                free(recibido);

                break;
             case IO_FS_DELETE: 

                // 1. Recibir de memoria los parametros de la IO DIALFS
                parametros_fs* recibido_d = recibir_parametros_io_fs(kernel_socket); // sin recibir buffer motivo desalojo
                if(recibido == NULL) {
                    log_error(logger_io_extra, "Error al recibir datos de KERNEL \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(recibido_d->nombre_archivo);
                    free(recibido_d);
                    return 1;
                }

                log_info(logger_io, "PID: %d - Operacion: IO FS delete \n", recibido->PID);
                
                nombre_archivo = strdup(recibido_d->nombre_archivo);
                free(recibido_d->nombre_archivo);

                // 3. Verificar si el archivo existe
                if (!archivo_duplicado(nombre_archivo)){ 
                    log_warning(logger_io_extra, "Archivo %s no se encuentra en el directorio. \n", nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(nombre_archivo);
                    free(recibido_d);
                    break;
                }

                // 4. Actualizar el bitmap y el bitmap.dat. Eliminar el FCB
                bloque_base = actualizar_espacio_disco(nombre_archivo, IO_FS_DELETE);
                if(bloque_base == UINT32_MAX){
                    log_error(logger_io_extra, "No se pudo eliminar el archivo %s del disco \n", nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(nombre_archivo);
                    free(recibido_d);
                    break;
                }   
                // 5. Eliminar archivo (Si no hay errores)
                if (!FS_DELETE(interfaz->path, nombre_archivo)){
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(nombre_archivo);
                    free(recibido_d);
                    break;
                }
                log_info(logger_io, "PID: %d - ELiminar Archivo: %s \n", recibido_d->PID, nombre_archivo);

                //6. Avisamos a kernel que el archivo se elimino correctamente
                enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, IO_FS_DELETE);
                free(nombre_archivo);
                free(recibido_d);
                break;
                
            case IO_FS_TRUNCATE: 
            
                // 1. Recibir de memoria los parametros de la IO DIALFS
                parametros_fs* recibido_t = recibir_parametros_io_fs(kernel_socket); 

                if(recibido_t == NULL) {
                    log_error(logger_io_extra, "Error al recibir datos de KERNEL \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(recibido_t->nombre_archivo);
                    free(recibido_t);
                    return 1;
                }
                
                // 2. Hacer los Logs Minimos y Obligatorios
                log_info(logger_io, "PID: %d - Operacion: IO FS TRUNCATE \n", recibido_t->PID);
                log_info(logger_io, "PID: %d - Truncar  Archivo: %s - Tamaño: %d \n", recibido_t->PID, recibido_t->nombre_archivo, recibido_t->reg_tamanio);
                
                // 3. Verificar si el archivo existe
                if (!archivo_duplicado(recibido_t->nombre_archivo)){ 
                    log_warning(logger_io_extra, "Archivo %s no se encuentra en el directorio. \n", recibido_t->nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(recibido_t->nombre_archivo);
                    free(recibido_t);
                    break;
                }

                // 4. Realiza el cambio de tamaño
                int resultado = FS_TRUNCATE(recibido_t) ? IO_FS_TRUNCATE : M_EXIT;  // cambiar despues?
                if (resultado == IO_FS_TRUNCATE){
                    log_info(logger_io_extra, "Archivo %s truncado exitosamente \n", recibido_t->nombre_archivo);
                } else {
                    log_error(logger_io_extra, "Error al truncar el archivo %s \n", recibido_t->nombre_archivo);
                }
                enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, resultado);
                free(recibido_t->nombre_archivo);
                free(recibido_t);
                break;
                
            case IO_FS_READ: 
                
                // 1. Recibir de kernel los parametros de la IO DIALFS
                params_inst_fs* recibido_r = recibir_params_inst_fs(kernel_socket);
                if(recibido_r == NULL) {
                    log_error(logger_io_extra, "Error al recibir datos de KERNEL \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_r);
                    return 1;
                }

                // 2. Hacer los Logs Minimos y Obligatorios
                log_info(logger_io, "PID: %d - Operacion: IO FS READ \n", recibido_r->PID);
                log_info(logger_io, "PID: %d - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d \n",  recibido_r->PID, recibido_r->nombre_archivo, recibido_r->reg_tamanio, recibido_r->ptr_arch);
                
                // 3. Verificar si existe el archivo en el directorio
                if (!archivo_duplicado(recibido_r->nombre_archivo)){
                    log_warning(logger_io_extra, "Archivo %s no se encuentra en el directorio. \n", recibido_r->nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_r);
                    break;
                }
                
                // 4. Leer de disco

                char* dato_leido = FS_READ(recibido_r->nombre_archivo, recibido_r->reg_tamanio, recibido_r->ptr_arch);

                if (dato_leido == NULL) {
                    log_error(logger_io_extra, "Error al leer el archivo %s \n", recibido_r->nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    free(dato_leido);
                    liberar_params_inst_fs(recibido_r);
                    return 1;
                }

                log_info (logger_io_extra, "Datos leídos de archivo: %s \n", dato_leido);
                // 5. Escribir memoria

                if (escribir_memoria(NULL, recibido_r->lista_dfs, recibido_r->PID, mem_socket, dato_leido, recibido_r->reg_tamanio, recibido_r->offset, tam_pag, IO_WRITE) == -1){
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_r);
                    return 1;
                }
                
                // 6. Notificar a Kernel
                enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, IO_FS_READ);
                
                // 7. Liberar memoria
                free(dato_leido);
                liberar_params_inst_fs(recibido_r);
                break;
                
            case IO_FS_WRITE:

                // 1. Recibir de kernel los parametros de la IO DIALFS
                params_inst_fs* recibido_w = recibir_params_inst_fs(kernel_socket);
                if(recibido_w == NULL) {
                    log_error(logger_io_extra, "Error al recibir datos de KERNEL \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_w);
                    return 1;
                }
                
                // 2. Hacer los Logs Minimos y Obligatorios
                log_info(logger_io, "PID: %d - Operacion: IO FS WRITE \n", recibido_w->PID);
                log_info(logger_io, "PID: %d - Escribir Archivo: %s - Tamaño a escribir: %d - Puntero Archivo %d \n", recibido_w->PID, recibido_w->nombre_archivo, recibido_w->reg_tamanio, recibido_w->ptr_arch);
                
                // 3. Verificar si existe el archivo en el directorio
                if (!archivo_duplicado(recibido_w->nombre_archivo)){
                    log_warning(logger_io_extra, "Archivo %s no se encuentra en el directorio. \n", recibido_w->nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_w);
                    break;
                }

                // 4. Verificar que se trate de una escritura valida
                t_fcb* fcb = get_fcb_lista(recibido_w->nombre_archivo);
                if (fcb == NULL) {
                    log_error(logger_io_extra, "No se encontró el archivo %s en la lista de archivos\n", recibido_w->nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_w);
                    return 1; 
                }
                
                if (recibido_w->ptr_arch < 0 || recibido_w->ptr_arch + recibido_w->reg_tamanio > fcb->tamanio) {
                    log_error(logger_io_extra, "Escritura inválida para el archivo %s\n", recibido_w->nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_w);
                    break;
                }
                
                // 5. Leer memoria
                char* dato_escribir = malloc(recibido_w->reg_tamanio);

                //Peticion y recepcion del dato de memoria
                t_tabla_r* tabla = malloc(sizeof(t_tabla_r));
                tabla->PID = recibido_w->PID;
                tabla->direc_pag = recibido_w->reg_tamanio; //chequear
                tabla->offset_size = recibido_w->offset;

                dato_escribir = leer_memoria(NULL, recibido_w->lista_dfs, tabla, mem_socket, tam_pag, recibido_w->reg_tamanio, IO_READ);
                
                log_info(logger_io_extra, "Datos leídos de memoria: %s \n", dato_escribir);
                
                // 6. Escribir los datos en el archivo
                if (!FS_WRITE(recibido_w, dato_escribir, fcb)) {
                    log_error(logger_io_extra, "Error al escribir en el archivo %s \n", recibido_w->nombre_archivo);
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_params_inst_fs(recibido_w);
                    return 1;
                }
                
                // 7. Avisar a kernel que la escritura fue exitosa
                enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, IO_FS_WRITE);
                liberar_params_inst_fs(recibido_w);
                free(tabla); // REVISAR
                break;
           
            default:
                log_error(logger_io_extra, "Operacion no reconocida: %d \n", header);
                exit(EXIT_FAILURE);
        }
    }
}

// ------------------------------- IO STDIN --------------------------------------
int conexion_cliente_stdin(t_io_std* interfaz){

    int kernel_socket = interfaz->fd_io_kernel;

    while (true){

        int header = recibir_operacion(kernel_socket);
        switch(header){
            
            case CONFIRMACION: // DE KERNEL
                recibir_mensaje(kernel_socket, logger_io_extra);
            break; 

            case DIRECC_MEM:

                // 1. Recibir lista de direcciones y tamanio a escuchar de KERNEL
                df_size* recibido = recibir_df_size(kernel_socket);
                if(recibido == NULL) {
                    log_error(logger_io_extra, "Error al recibir datos de KERNEL \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_df_size(recibido);
                    return 1;
                } 

                log_info(logger_io, "PID: %d - Operacion: IO STDIN READ \n", recibido->PID);
                
                // 2. Cambiar el estado de la interfaz a leyendo
                log_info(logger_io_extra, "Interfaz %s espera a que escriban por consola \n", interfaz->nombre_interfaz);
                interfaz->estado = READING;

                // 3. Esperar a que el usuario escriba por consola
                char* input = consola_io_stdin(recibido->reg_tamanio);

                char* print_input = strdup(input);///

                print_input[recibido->reg_tamanio] = '\0';

                log_info(logger_io_extra, "\n Datos leídos por consola: %s \n", print_input);
                
                // 4. Cambiar el estado de la interfaz a no-leyendo
                interfaz->estado = NOT_READING;
                
                // 5. Mandarle el dato a memoria para escribir y recibir mensaje de confirmacion
                result = escribir_memoria(NULL, recibido->list_df_io, recibido->PID, interfaz->fd_io_mem, input, recibido->reg_tamanio, recibido->offset, tam_pag, IO_WRITE);    
                if(result == OK)
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, IO_STDIN_READ);
                else if (result == ERROR)
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);    

                liberar_df_size(recibido);
                free(input);
                free(print_input);
                
                break;
            case -1:
                log_error(logger_io_extra, "Cliente desconectado de %s... \n", "MEM_SERVER");
                return 1;
            // Si hay un error desconocido, terminar el programa
            default:
                log_error(logger_io_extra, "Algo anduvo mal en el server de %s. Cop: %d \n", "MEM_SERVER", header);
                return 1;
        }
    }
    return 1;
}

//-----------------------------IO_STDOUT-----------------------------

int conexion_cliente_stdout(t_io_std* interfaz){

    int kernel_socket = interfaz->fd_io_kernel;
    int mem_socket = interfaz->fd_io_mem;

    while (true){

        int header = recibir_operacion(kernel_socket);
        switch(header){
            case CONFIRMACION: // DE KERNEL
        
                recibir_mensaje(kernel_socket, logger_io_extra);
                
                break; 
            case DIRECC_MEM: 

                // 1. Recibir lista de direcciones y tamaño a imprimir de KERNEL
                df_size* recibido = recibir_df_size(kernel_socket);
                if(recibido == NULL) {
                    log_error(logger_io_extra, "Error al recibir datos de KERNEL \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_df_size(recibido);
                    return 1;
                } 

                log_info(logger_io, "PID: %d - Operacion: IO STDOUT WRITE \n", recibido->PID);

                t_tabla_r* tabla = malloc(sizeof(t_tabla_r));
                
                tabla->PID = 0;
                tabla->offset_size = recibido->offset; 

                // 2. Leer los datos desde memoria
                char* datos = leer_memoria(NULL, recibido->list_df_io, tabla, mem_socket, tam_pag, recibido->reg_tamanio, IO_READ);
                
                if (datos == NULL) {
                    log_error(logger_io_extra, "Error al leer datos de memoria \n");
                    enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, M_EXIT);
                    liberar_df_size(recibido);// Liberar memoria antes de salir
                    free(tabla);
                    return 1;
                }

                // 3. Cambiar el estado de la interfaz a imprimiendo
                log_info(logger_io_extra, "Interfaz %s está imprimiendo en pantalla \n", interfaz->nombre_interfaz);
                interfaz->estado = PRINTING;

                // 4. Imprimir los datos por pantalla
                log_warning(logger_io_extra, "Datos leídos desde memoria impresos por consola: %s \n", datos);

                // 5. Cambiar el estado de la interfaz a no-imprimiendo
                interfaz->estado = NOT_PRINTING;
                log_info(logger_io_extra, "Interfaz %s terminó de imprimir por pantalla \n", interfaz->nombre_interfaz);

                // 6. Informar al Kernel sobre la finalización de la operación
                enviar_mensaje(interfaz->nombre_interfaz, kernel_socket, T_IO_STDOUT);

                // Liberar memoria dinámica
                free(datos);
                free(tabla);
                liberar_df_size(recibido);
                break;

            case -1:
                log_error(logger_io_extra, "Cliente desconectado de %s... \n", "MEM_SERVER");
                return 1;
            // Si hay un error desconocido, terminar el programa
            default:
                log_error(logger_io_extra, "Algo anduvo mal en el server de %s. Cop: %d \n", "MEM_SERVER", header);
                return 1;
        }
    }
    return 1;
}