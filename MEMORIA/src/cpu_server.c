#include "../include/cpu_server.h"

// Funcion para procesar la conexion con CPU
void* procesar_conexion_cpu(void* void_args) {
    // Convertir los argumentos a la estructura correcta
    t_procesar_conexion_args* args = (t_procesar_conexion_args*)void_args;
    cpu_socket = args->mem_serv_socket;
    char* mem_serv_name = args->mem_serv_name;
    
    free(args);
    while (true){

        // Recibir el codigo de operacion del cliente
        int cod_op = recibir_operacion(cpu_socket);
        
        usleep(RETARDO_RESPUESTA); //Retardo en microsegundos
        
        switch (cod_op){
            case SGTE_INSTRUCC:
                log_info(logger_memoria_extra, "%s me está pidiendo una instruccion...\n", mem_serv_name);
                
                // TEMPORAL, HASTA CREAR ESTRUCTURA DE MEMORIA
                //Recibir el PID y PC de CPU
                t_pedir_instruccion* instruccion = recibir_instruccion(cpu_socket);

                // Buscar en la lista de instrucciones la instruccion que corresponde al PID y devolver la instruccion de la posicion PC
                for(int i = 0; i < list_size(procesos); i++){
                    pthread_mutex_lock (&mutex_lista_procesos);
                    t_proceso* instruccion_memoria = list_get(procesos, i);
                    pthread_mutex_unlock (&mutex_lista_procesos);
                    if(instruccion_memoria->pid == instruccion->PID){
                        // Obtener la siguiente instruccion
                        char* instruccion_a_enviar = list_get(instruccion_memoria->instrucciones, instruccion->PC);
                        enviar_mensaje(instruccion_a_enviar, cpu_socket, SGTE_INSTRUCC); 
                        
                        
                        // Loggeamos la instruccion enviada del proceso correspondiente
                        log_info(logger_memoria_extra, "Instruccion del proceso %d enviada a %s... \n", instruccion_memoria->pid, mem_serv_name);
                        break;
                    }
                }
                free(instruccion);
                
                break;
            case OBTENER_MARCO:
                t_tabla* tabla_frame = recibir_pedido_marco(cpu_socket);
                uint32_t frame = acceder_tabla_paginas(tabla_frame->PID, tabla_frame->marco_pag);
                free(tabla_frame); ///
                send(cpu_socket, &frame, sizeof(uint32_t), 0);

                break;
            case AJUSTAR_TAMANIO_PROCESO:
                log_info(logger_memoria_extra, "Ajustando tamaño de proceso...\n");
                t_resize* resize = recibir_resize(cpu_socket);
                int result = modificar_tabla_paginas(resize->cant_pags,resize->PID,resize->tam_resize);
                free(resize); ///
                send(cpu_socket, &result, sizeof(int), 0);
                break;
                
            case LECTURA:
                t_tabla_r* tabla_lectura = recibir_peticion_lectura(cpu_socket);

                log_info(logger_memoria,"PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño %d \n", tabla_lectura->PID, tabla_lectura->direc_pag, tabla_lectura->offset_size);

                char* valor = acceso_usuario(tabla_lectura->direc_pag, LECTURA, "0", tabla_lectura->offset_size);
                enviar_mensaje(valor, cpu_socket, LECTURA);
                free(tabla_lectura);
                free(valor);

                break;
            
            case ESCRITURA:
                t_tabla_w* tabla_w = recibir_peticion_escritura(cpu_socket);
                log_info(logger_memoria_extra, "Escritura de usuario...\n");
                log_info(logger_memoria,"PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño %d \n", tabla_w->PID, tabla_w->df, tabla_w->length_dato);
                
                char* value = acceso_usuario(tabla_w->df, ESCRITURA, tabla_w->dato, tabla_w->length_dato);
                
                if(strcmp(value, "ERROR")==0) {
                    result = ERROR;
                } else if (strcmp(value, "OK") == 0) {
                    result = OK;
                }

                send(cpu_socket, &result, sizeof(int), 0); //Enviamos resultado de escritura
                liberar_tabla_w(tabla_w);
                free(value);
                
                break;

            case -1:
                log_error(logger_memoria_extra, "Cliente desconectado de %s...\n", mem_serv_name);
                terminar_programa();
                exit(EXIT_FAILURE);
                break;
            // Si hay un error desconocido, terminar el programa
            
            default:
                log_error(logger_memoria_extra, "Hubo un error en el server de %s \n", mem_serv_name);
                log_warning(logger_memoria_extra, "Cop: %d \n", cod_op);
                terminar_programa();
                exit(EXIT_FAILURE);
        }
    }
    // Log de advertencia si el cliente se desconecta
    log_warning(logger_memoria_extra, "El cliente se desconecto de %s server \n", mem_serv_name);
    // Cerrar el socket del cliente

    close(cpu_socket);
    pthread_exit(NULL);
    return 0;
}