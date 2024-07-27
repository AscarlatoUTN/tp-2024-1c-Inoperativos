#include "../include/server_multih.h"


handshake HANDSHAKE;
t_pcb * pcb;
int espacios;
int INTERRUPT_EXIT = 2;
uint32_t PID_EXEC = 0;

// Función para procesar la conexión de interrupción
void* establecer_conexion_interrupt(void* void_args){
    
    t_procesar_conexion_args* args = (t_procesar_conexion_args*)void_args; // Convertir args a la estructura correcta
    char* cpu_serv_name = args->cpu_serv_name; // Extraer nombre del servidor de los args
    free(args);
    
    while (1){
        // Recibir el código de operación del cliente
        int cod_op = recibir_operacion(conexion_kernel_interrupt); 
        switch (cod_op) {
        case INTR: // Interruption flag
            uint32_t PID_INTR = recibir_nums(conexion_kernel_interrupt);
            
            pthread_mutex_lock(&mutex_pid_exec);
            if (PID_INTR == PID_EXEC)
            {                   
                flag_interrupcion(&mutex_interrupcion, INTERRUPT_EXIT, &hubo_interrupcion);
                log_warning(logger_cpu_extra, "Interrupción de exit recibida. \n");
                send (conexion_kernel_interrupt, &OK, sizeof (int),0);
            }
            if (PID_INTR != PID_EXEC)
                send (conexion_kernel_interrupt, &ERROR, sizeof (int), 0);
            pthread_mutex_unlock(&mutex_pid_exec);
            break;
        case FIN_Q:
            uint32_t PID_FIN_Q = recibir_nums(conexion_kernel_interrupt);
            pthread_mutex_lock(&mutex_pid_exec);
            if (PID_FIN_Q == PID_EXEC)
            {    
                flag_interrupcion(&mutex_interrupcion, INTERRUPT, &hubo_interrupcion);
                log_warning(logger_cpu_extra, "Interrupción de fin de quantum recibida. \n");
            }
            pthread_mutex_unlock(&mutex_pid_exec);
            break;
        case -1:
            log_error(logger_cpu_extra, "Cliente desconectado de %s... \n", cpu_serv_name);
            exit(EXIT_FAILURE);
            break;
        default:
            log_error(logger_cpu_extra, "Algo anduvo mal en el server de %s. Cop: %d \n", cpu_serv_name, cod_op);
            exit(EXIT_FAILURE);
        }

    }
    log_warning(logger_cpu_extra, "El cliente se desconecto de %s server \n", cpu_serv_name);
    close(conexion_kernel_interrupt); // Cerrar el socket del cliente
    return 0;
}

// Función para establecer la conexión
void* establecer_conexion_dispatch(char* cpu_serv_name){     
    while (1){
        int cod_op = recibir_operacion(conexion_kernel_dispatch); // Recibir el código de operación del cliente
        switch (cod_op) {
            case CTXT_EXEC:
                
                //--------------------Recibir Contexto de Ejecución
                t_pcb* PCB = recibir_pcb(conexion_kernel_dispatch);

                if (PCB == NULL) {
                    log_error(logger_cpu_extra, "Error al recibir PCB de %s \n", cpu_serv_name);
                    break;
                }
                pthread_mutex_lock(&mutex_pid_exec);
                PID_EXEC = PCB->PID;
                pthread_mutex_unlock(&mutex_pid_exec);
                
                log_info(logger_cpu_extra, "Recibi el PCB de PID: %d \n", PCB->PID);

                t_pcb* PCB_actualizado = pedir_instrucciones(PCB);

                // --------------------Devolver Contexto de Ejecución Actualizado
        
                // Enviar el PCB actualizado a kernel segun motivo
                pthread_mutex_lock(&mutex_interrupcion);
                if (hubo_interrupcion == INTERRUPT) {
                    pthread_mutex_unlock(&mutex_interrupcion);
                    
                    enviar_pcb(conexion_kernel_dispatch, PCB_actualizado, M_FIN_QUANTUM);

                } else if (hubo_interrupcion == INTERRUPT_EXIT) {
                    pthread_mutex_unlock(&mutex_interrupcion);
                    //Enviar PCB
                    enviar_pcb(conexion_kernel_dispatch, PCB_actualizado, M_PROC_INTERRUPT);
                    
                } else {
                    pthread_mutex_unlock(&mutex_interrupcion);
                }

                flag_interrupcion(&mutex_interrupcion, NOT_INTERRUPT, &hubo_interrupcion);
                pthread_mutex_lock (&mutex_pid_exec);
                PID_EXEC = 0;
                pthread_mutex_unlock(&mutex_pid_exec);

                break;

            case -1:
                log_error(logger_cpu_extra, "Cliente desconectado de %s... \n", cpu_serv_name);
                exit (EXIT_FAILURE);
                break;
            
            // Si hay un error desconocido, terminar el programa
            default:
                log_error(logger_cpu_extra, "Algo anduvo mal en el server de %s. Cop: %d \n", cpu_serv_name, cod_op);
                exit (EXIT_FAILURE);
        }
    }
    log_warning(logger_cpu_extra, "El cliente se desconecto de %s server \n", cpu_serv_name);
    close(conexion_kernel_dispatch); // Cerrar el socket del cliente
    return 0;
}

int manejo_interrupts(char* cpu_serv_name){
    // Esperar a que el cliente se conecte
    conexion_kernel_interrupt = esperar_cliente(logger_cpu_extra, cpu_serv_name, cpu_interrupt_socket);
    
    // Si el cliente se conecta correctamente, crear un nuevo hilo para procesar la conexión
    if (conexion_kernel_interrupt!=-1){
        pthread_t thread;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->cpu_serv_name = cpu_serv_name;

        recv(conexion_kernel_interrupt, &HANDSHAKE, sizeof(handshake), MSG_WAITALL);
        if (HANDSHAKE == HANDSHAKE_KERNEL_CPU) {
            result = OK;
            log_info(logger_cpu_extra, "Handshake Kernel->CPU OK\n");
            send(conexion_kernel_interrupt, &result, sizeof(int32_t), 0);
            pthread_create(&thread,NULL,(void*) establecer_conexion_interrupt,(void*) args);
            pthread_detach(thread);
        } else {
            result = ERROR;
            log_error(logger_cpu_extra, "Handshake Kernel->CPU ERROR\n");
            send(conexion_kernel_interrupt, &result, sizeof(int32_t), 0);
        }
        return 1;
    }
    return 0;
}


int server_escuchar(char* cpu_serv_name){
    // Esperar a que el cliente se conecte
    conexion_kernel_dispatch = esperar_cliente (logger_cpu_extra, cpu_serv_name, cpu_dispatch_socket);
    
    // Si el cliente se conecta correctamente, crear un nuevo hilo para procesar la conexión
    if (conexion_kernel_dispatch!=-1){
        recv(conexion_kernel_dispatch, &HANDSHAKE, sizeof(handshake), MSG_WAITALL);
        if (HANDSHAKE == HANDSHAKE_KERNEL_CPU){
            log_info(logger_cpu_extra, "Handshake Kernel->CPU OK \n");
            result = OK;
            send(conexion_kernel_dispatch, &result, sizeof(int32_t), 0);
            establecer_conexion_dispatch(cpu_serv_name);
        } else {
            log_error(logger_cpu_extra, "Handshake inxistente \n");
            result = ERROR; 
            send(conexion_kernel_dispatch, &result, sizeof(int32_t), 0);
            close(conexion_kernel_dispatch);
        }
        return 1;
    }
    return 0;
}
