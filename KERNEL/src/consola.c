#include "../include/consola.h"

// Comandos Ingresados por Consola
char* commands[] = {
    "INICIAR_PROCESO",
    "FINALIZAR_PROCESO",
    "EJECUTAR_SCRIPT",
    "INICIAR_PLANIFICACION",
    "DETENER_PLANIFICACION",
    "PROCESO_ESTADO",
    "MULTIPROGRAMACION",
    NULL
};

// Linkea los comandos de consola con las funciones que ejecutan
t_console consoleMap[] = {
    {iniciar_proceso, "INICIAR_PROCESO"},
    {finalizar_proceso, "FINALIZAR_PROCESO"},
    {ejecutar_script, "EJECUTAR_SCRIPT"},
    {iniciar_planificacion, "INICIAR_PLANIFICACION"},
    {detener_planificacion, "DETENER_PLANIFICACION"},
    {proceso_estado, "PROCESO_ESTADO"},
    {multiprogramacion, "MULTIPROGRAMACION"},
    {NULL, NULL}
};


void consola() {
    char* input;

    while(true) {
        input = readline("> ");
        if (input == NULL) {
            log_error(logger_kernel_extra, "Comando nulo \n");

            continue; // Si el comando es NULL, se saltea el resto del bucle
        }

        char* comando = strdup(input); // Copia el comando para pasarlo al hilo
        if (comando == NULL) {
            log_error(logger_kernel_extra, "Error al duplicar el comando\n");
            free(input);
            continue;
        }

        pthread_t hilo_consola;
        pthread_create(&hilo_consola, NULL, (void*)procesar_comando, (void*)comando);
        pthread_detach(hilo_consola);
        free(input);
    }
}


// -------------------- EJECUTAR SCRIPT

void ejecutar_script(char* path_relativo){

    if(path_relativo == NULL || strcmp(path_relativo, "")==0){
        log_error(logger_kernel_extra, "Falta ingresar el path del script \n");
        return;
    }

    char* path_absoluto = strdup(PATH_INSTRUCCIONES);
    string_append(&path_absoluto, path_relativo);

    FILE* archivo = fopen(path_absoluto, "r"); // Abrir el archivo
    if (archivo == NULL)
    {
        log_error(logger_kernel_extra, "No se pudo abrir el archivo %s... \n", path_absoluto);
        free(path_absoluto);
        return;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, archivo)) != -1)
    { // Leer hasta el \n
        if (line[read - 1] == '\n')
        { // Cambiar el fin de linea '\n' por el fin de string '\0'
            line[read - 1] = '\0';
        }
        char* input = strdup(line); // Copiar la linea del archivo
        procesar_comando((void*)input);
    }

    fclose(archivo);
    free(line);
    free(path_absoluto);
}

void procesar_comando(void* comando) {
    char* input = (char*)comando;
    char** split_console = string_n_split(input, 2, " ");

    if (split_console == NULL) {
        log_error(logger_kernel_extra, "Error en string_n_split\n");
        free(input);
        return;
    }

    if (split_console[0] != NULL){
    // Compara el comando con cada uno de los comandos creados en cmds[]
        for(uint8_t i = 0; consoleMap[i].consoleName != NULL; i++) {

            // Compara el primer elemento del array split_console con el nombre de la consola en la posición i
            if(strcmp(split_console[0], consoleMap[i].consoleName) == 0) {

                // Si son iguales, ejecuta la función correspondiente con los argumentos split_console[1] y conexion
                consoleMap[i].func(split_console[1]);

                string_array_destroy(split_console);

                free (input);
                // Vuelve a consola, termina toda esta funcion
                return;
            }
        }
    }
    log_error(logger_kernel_extra, "El comando ingresado es incorrecto\n");

    string_array_destroy(split_console);

    free(input);
}

// -------------------- PROCESO ESTADO

void proceso_estado() {
    estado_procesos_t estados[] = {
        {"New", cola_new},
        {"Ready", cola_ready},
        {"Auxiliar VRR", cola_aux_vrr},
    };

    int num_estados = (sizeof(estados) / sizeof(estados[0]));

    for (int i = 0; i < num_estados; i++) {
        listar_procesos(estados[i].nombre_estado, estados[i].lista_procesos);
    }

    accion_int_rec(IMPRIMIR, false);

    pthread_mutex_lock(&mutex_pid);
    log_info(logger_kernel, "EXEC: %d", PID_EXEC);
    pthread_mutex_unlock(&mutex_pid);
}

// -------------------

void multiprogramacion(char* grado_nuevo){

    if (grado_nuevo == NULL || strcmp(grado_nuevo, "") == 0){
        log_error(logger_kernel_extra, "Falta ingresar el nuevo grado de multiprogramación");
        return;
    }
    
    // Verificar si grado_nuevo es mayor al grado actual de multiprogramación
    int nuevo_grado = atoi(grado_nuevo);
    if (nuevo_grado == GRADO_MULTIPROGRAMACION) {
        log_info(logger_kernel_extra, "Grado de Multiprogramación no ha sido modificado \n");
        return;
    }

    int diferencia = nuevo_grado - GRADO_MULTIPROGRAMACION;
    if (diferencia > 0) {
        for (int i = 0; i < diferencia; i++) {
            sem_post(&sem_multiprogramacion);
        }
    } else {
        for (int i = 0; i < -diferencia; i++) {
            pthread_t hilo_multiprogramacion;
            pthread_create(&hilo_multiprogramacion, NULL, (void*)bajar_multiprogramacion, NULL);
            pthread_detach(hilo_multiprogramacion);
        }
    }

    log_info(logger_kernel_extra, "Grado de Multiprogramación actualizado a %d \n", nuevo_grado);
    GRADO_MULTIPROGRAMACION = nuevo_grado;
}

void bajar_multiprogramacion(){
    sem_wait(&sem_multiprogramacion);
}
