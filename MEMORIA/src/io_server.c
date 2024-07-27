#include "../include/io_server.h"


void* procesar_conexion_io(void* void_args) {

    t_procesar_conexion_args* args = (t_procesar_conexion_args*)void_args;
    int io_socket = args->mem_serv_socket;
    char* mem_serv_name = args->mem_serv_name;
    free(args);
    char* value;

    while(1){
        int cod_op = recibir_operacion(io_socket); 
        usleep(RETARDO_RESPUESTA);// Retardo en microsegundos
        switch (cod_op) {
            case IO_WRITE:     
                t_tabla_w* tabla_w = recibir_peticion_escritura(io_socket);
                log_info(logger_memoria_extra, "Escritura de usuario...\n");
                log_info(logger_memoria,"PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño %d \n", tabla_w->PID, tabla_w->df, tabla_w->length_dato);
                
                value = acceso_usuario(tabla_w->df, ESCRITURA, tabla_w->dato, tabla_w->length_dato);
                  if(strcmp(value, "ERROR")==0) {
                    result = ERROR;
                } else if (strcmp(value, "OK") == 0) {
                    result = OK;
                }
                free(value);
                send(io_socket, &result, sizeof(int), 0); //Enviamos resultado de escritura
                liberar_tabla_w(tabla_w);
                
                break;
            case IO_READ:
                log_info(logger_memoria_extra, "Lectura de usuario...\n");
                t_tabla_r* tabla_r = recibir_peticion_lectura(io_socket);
                value = acceso_usuario(tabla_r->direc_pag, LECTURA, "0", tabla_r->offset_size);
                enviar_mensaje(value, io_socket, LECTURA);         
                log_warning(logger_memoria_extra, "Mensaje enviado \n");
                free(tabla_r);
                free (value);

                break;

            case -1:
                log_error(logger_memoria_extra, "Cliente desconectado de %s... \n", mem_serv_name);
                //terminar_programa(io_socket);
                // exit(EXIT_FAILURE);
                return NULL;
            
            // Si hay un error desconocido, terminar el programa
            default:
                log_error(logger_memoria_extra, "Algo anduvo mal en el server de %s. Cop: %d \n", mem_serv_name, cod_op);
                //terminar_programa(io_socket);
                //exit(EXIT_FAILURE);
                return NULL;

            }
        }
    log_error(logger_memoria_extra, "El cliente se desconecto de %s server \n", mem_serv_name);

    close(io_socket); // Cerrar el socket del cliente
    return 0;
}