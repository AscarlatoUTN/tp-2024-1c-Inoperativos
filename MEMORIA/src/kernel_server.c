#include "../include/kernel_server.h"

// Funcion para procesar la conexion con Kernel
void* procesar_conexion_kernel(void* void_args) {
    // Convertir los argumentos a la estructura correcta
    t_procesar_conexion_args* args = (t_procesar_conexion_args*)void_args;
    kernel_socket = args->mem_serv_socket;
    char* mem_serv_name = args->mem_serv_name;
    
    free (args);
    // Bucle infinito para procesar las operaciones
    while (true){
        
        // Recibir el codigo de operacion del cliente
        int cod_op = recibir_operacion(kernel_socket); 
        
        switch (cod_op) {
            case CREAR_PROCESO:
                /*
                if(procesos == NULL)
                    procesos = list_create();
                */
                t_proceso* proceso = malloc(sizeof(t_proceso));
                proceso_memoria* proceso_memoria = recibir_proceso(kernel_socket); // recibir el PID y Path desde KERNEL
                log_info(logger_memoria_extra, "PID recibido: %d \n", proceso_memoria->PID);
                
                // Obtener el PID y el Path del paquete
                proceso->pid = proceso_memoria->PID;
                
                char* path_absoluto = string_new();

                string_append(&path_absoluto, PATH_INSTRUCCIONES);
                string_append(&path_absoluto, proceso_memoria->path);

                // Abrir el archivo
                FILE* archivo = fopen(path_absoluto, "r");
                if (archivo == NULL){
                    log_error(logger_memoria_extra, "No se pudo abrir el archivo %s... \n", path_absoluto);
                    free(proceso);
                    free(path_absoluto);
                    free(proceso_memoria->path);
                    free(proceso_memoria);
                    send(kernel_socket, &ERROR, sizeof(int), 0);
                    break;
                }
                // Crear una lista para almacenar las instrucciones y la tabla de paginas
                proceso->instrucciones = list_create();
                proceso->pageTable = list_create();

                char* line = NULL;
                char* valor;
                size_t len = 0;
                ssize_t read;
                // Leer hasta el \n
                while ((read = getline(&line, &len, archivo)) != -1) {
                    // Cambiar el fin de linea '\n' por el fin de string '\0'
                    if (line[read - 1] == '\n') {
                        line[read - 1] = '\0';
                    }
                    valor = strdup(line);  // copia la linea del archivo
                    list_add(proceso->instrucciones, valor);  // agrega la linea a la lista de instruccion
                }
                log_info(logger_memoria, "PID: %d - Tamaño: 0 \n", proceso_memoria->PID);

                // Agregamos los procesos creados a una lista para almacenarlos en memoria
                // DESPUES CUANDO SE CREE LA ESTRUCTURA DE MEMORIA REAL, CAMBIAR
                pthread_mutex_lock (&mutex_lista_procesos);
                list_add(procesos, proceso);
                pthread_mutex_unlock (&mutex_lista_procesos);
                
                free(path_absoluto);
                free(line);
                free(proceso_memoria->path);
                free(proceso_memoria);
                fclose(archivo);
            
                
                // Enviar un mensaje de confirmacion al kernel
                send(kernel_socket, &RECIBIDO, sizeof(int), 0);
                
                break;

            case ELIMINAR_PROCESO:
                uint32_t PID = recibir_nums(kernel_socket);
                t_proceso* proceso_a_eliminar = encontrar_proceso_por_pid(PID);
                if (proceso_a_eliminar == NULL){
                    log_error(logger_memoria_extra, "No se encontro el proceso %d \n", PID);
                    send(kernel_socket, &ERROR, sizeof(int), 0);
                    break;
                }
                eliminar_proceso(proceso_a_eliminar);
                
                send(kernel_socket, &OK, sizeof(int), 0);
                break;

            case -1:
                log_error(logger_memoria_extra, "Cliente desconectado de %s... \n", mem_serv_name);
                return NULL;
            // Si hay un error desconocido, terminar el programa
            default:
                log_error(logger_memoria_extra, "Algo anduvo mal en el server de %s. Cop: %d \n", mem_serv_name, cod_op);
                return NULL;
        }

    }
    // Log de advertencia si el cliente se desconecta
    log_error(logger_memoria_extra, "El cliente se desconecto de %s server \n", mem_serv_name);
    // Cerrar el socket del cliente
    
    close(kernel_socket);
    pthread_exit(NULL);
    return 0;
}