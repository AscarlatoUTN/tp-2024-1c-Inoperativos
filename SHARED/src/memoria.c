#include "../include/memoria.h"

//------------ACCESO A MEMORIA-------------------------

char* leer_memoria(t_log* logger, t_list* list_df, t_tabla_r* tabla, int conexion_memoria, uint32_t tam_pagina, uint32_t tam_dato, header caso) {
    char* dato = malloc(tam_dato + 1); // +1 para el carácter nulo al final
    memset(dato, 0, tam_dato + 1);
    if (dato == NULL) {
        log_error(logger, "Error al asignar memoria para el dato leído de memoria.\n");
        return NULL;
    }
    
    int offset = 0;
    uint32_t desplazamiento = tabla->offset_size;
    int bytes_a_leer = 0;
    int cant_pags = list_size(list_df);
    
    uint32_t leer_primera = tam_pagina - desplazamiento;
    
    for (int i = 0; i < cant_pags; i++) {
        tabla->direc_pag = *(uint32_t*)list_get(list_df, i);

        if (i == 0) {
            bytes_a_leer = (tam_dato < leer_primera) ? tam_dato : leer_primera;
        } else {
            int bytes_restantes = tam_dato - offset;
            bytes_a_leer = (bytes_restantes < tam_pagina) ? bytes_restantes : tam_pagina;
        }

        tabla->offset_size = bytes_a_leer;
        
        
        peticion_lectura(caso, tabla, conexion_memoria);
        char* buffer = guardar_mensaje(conexion_memoria); // Recibo el dato leído de memoria
        if (strcmp("ERROR", buffer) == 0) {
            free(dato);
            free(buffer);
            return NULL;
        }

        if(logger != NULL) {
            if (bytes_a_leer > 1) {
                log_info(logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d \n", tabla->PID, tabla->direc_pag, *(uint32_t*)buffer);
            } else {
                log_info(logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d \n", tabla->PID, tabla->direc_pag, *(uint8_t*)buffer);
            }
        }
        
        memcpy(dato + offset, buffer, bytes_a_leer);
        offset += bytes_a_leer;
        
        free(buffer); // Liberar memoria del buffer
    }
    
    dato[tam_dato] = '\0'; // Agregar el carácter nulo al final del buffer
    
    return dato;
}

int escribir_memoria(t_log* logger, t_list* list_df, uint32_t PID, int conexion_memoria, void* dato, size_t tam_dato, uint32_t desplazamiento, uint32_t tam_pagina, header caso) {
    int offset = 0;
    int bytes_a_escribir = 0;
    int result = 0;
    
    int cant_pags = list_size(list_df);

    t_tabla_w* tabla_w = malloc(sizeof(t_tabla_w));
    tabla_w->PID = PID;
    tabla_w->offset = desplazamiento;
    
    uint32_t escribir_primera = tam_pagina - desplazamiento;

    for (int i = 0; i < cant_pags; i++) {
        
        tabla_w->df = *(uint32_t*)list_get(list_df, i);

        if(i == 0) {
            bytes_a_escribir = (tam_dato < escribir_primera) ? tam_dato : escribir_primera;
        } else {
            int bytes_restantes = tam_dato - offset;
            bytes_a_escribir = (bytes_restantes < tam_pagina) ? bytes_restantes : tam_pagina;
        }

        // Copiar datos al buffer de la tabla
        tabla_w->dato = calloc(bytes_a_escribir, sizeof(int));
       // memset(tabla_w->dato, 0, bytes_a_escribir);
        memcpy(tabla_w->dato, dato + offset, bytes_a_escribir);
        tabla_w->length_dato = bytes_a_escribir;

        peticion_escritura(caso, tabla_w, conexion_memoria);
        recv(conexion_memoria, &result, sizeof(int), MSG_WAITALL);
        if (result == -1) {
            free(tabla_w->dato); 
            free(tabla_w); 
            return -1;
        }
        if(logger != NULL) {
            if (bytes_a_escribir > 1) {
                log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %d \n", tabla_w->PID, tabla_w->df, *(uint32_t*) tabla_w->dato);
            } else {
                log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %d \n", tabla_w->PID, tabla_w->df, *(uint8_t*) tabla_w->dato);
            }
        }
        
        offset += bytes_a_escribir;
        free(tabla_w->dato); // Liberar el dato copiado después de la escritura
    }
    free(tabla_w);
    return 0;
}

// ------------------ Envío y Recepción para LECTURA y ESCRITURA de Memoria----------------------


// ------------------- LECTURA
void peticion_lectura(header header, t_tabla_r* obtener_marco, int conexion){
    t_paquete* paquete = crear_paquete(header);
    agregar_peticion_lectura(paquete, obtener_marco);   
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void agregar_peticion_lectura(t_paquete* paquete, t_tabla_r* tabla) {
    
    int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 3);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tabla->PID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &tabla->direc_pag, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiamos el dato al buffer del paquete
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(tabla->offset_size), sizeof(uint32_t));
    offset += sizeof (uint32_t);

    paquete->buffer->size += offset;
}

t_tabla_r* recibir_peticion_lectura(int conexion) {
    
    int size = 0;
    int offset = 0;

    void* buffer = recibir_buffer(&size, conexion);
	t_tabla_r* tabla = malloc(sizeof(t_tabla_r));

    memcpy(&(tabla->PID) , buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(tabla->direc_pag),  buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(tabla->offset_size), buffer + offset, sizeof(uint32_t));
    
    free(buffer);

    return tabla;
}


// ------------------- ESCRITURA
void peticion_escritura(header header, t_tabla_w* tabla_w, int conexion) {
    t_paquete* paquete = crear_paquete(header);
    agregar_peticion_escritura(paquete, tabla_w);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void agregar_peticion_escritura(t_paquete* paquete, t_tabla_w* tabla_w) {
    int offset = 0;

    // Aumentamos el tamaño del buffer del paquete para agregar los datos de la tabla_w
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 4 + tabla_w->length_dato);

    // Copiamos el PID al buffer del paquete
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &tabla_w->PID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiamos el df al buffer del paquete
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &tabla_w->df, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiamos el length_dato al buffer del paquete
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &tabla_w->length_dato, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiamos el dato al buffer del paquete
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, tabla_w->dato, tabla_w->length_dato);
    offset += tabla_w->length_dato;

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &tabla_w->offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Actualizamos el tamaño del buffer del paquete
    paquete->buffer->size += offset;
}


// ----------------------------- Petición de lectura -----------------------------

t_tabla_w* recibir_peticion_escritura(int conexion) {
    int size = 0;
    int offset = 0;

    // Recibir el buffer desde la conexión
    void* buffer = recibir_buffer(&size, conexion);
    t_tabla_w* tabla_w = malloc(sizeof(t_tabla_w));

    // Copiar el PID desde el buffer al struct tabla_w
    memcpy(&(tabla_w->PID), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiar el df desde el buffer al struct tabla_w
    memcpy(&(tabla_w->df), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiar el length_dato desde el buffer al struct tabla_w
    memcpy(&(tabla_w->length_dato), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Asignar memoria para el dato y copiarlo desde el buffer al struct tabla_w
    tabla_w->dato = malloc(tabla_w->length_dato);
    memcpy(tabla_w->dato, buffer + offset, tabla_w->length_dato);
    offset += tabla_w->length_dato;

    memcpy(&tabla_w->offset, buffer + offset, sizeof(uint32_t));

    // Liberar el buffer recibido
    free(buffer);
    return tabla_w;
}

//----------------------Hacer Pedido Marco--------------------------
void enviar_pedido_marco(header header, int conexion, t_tabla* marco){
	t_paquete* paquete = crear_paquete(header);
	agregar_pedido_marco(paquete, marco);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_pedido_marco(t_paquete* paquete, t_tabla* marco){
	
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 2);

	// Copiar el PID del PCB al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size, &(marco->PID), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(marco->marco_pag), sizeof(uint32_t));

	// Actualizar el tamaño del buffer para incluir los datos recién agregados
	paquete->buffer->size += sizeof(uint32_t) * 2;
}

t_tabla* recibir_pedido_marco(int conexion) {
    
    int size = 0;
    int offset = 0;

    void* buffer = recibir_buffer(&size, conexion);
    t_tabla* tabla = malloc(sizeof(t_tabla));

    memcpy(&(tabla->PID), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(tabla->marco_pag),  buffer + offset, sizeof(uint32_t));
    
    free (buffer);

    return tabla;
}


//----------------------Hacer Pedido Marco--------------------------
void enviar_pedido_resize(header header, int conexion, t_resize* resize){
	t_paquete* paquete = crear_paquete(header);
	agregar_resize(paquete, resize);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_resize(t_paquete* paquete, t_resize* resize){
	
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 3);
    
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(resize->PID), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(resize->tam_resize), sizeof(uint32_t));
	offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(resize->cant_pags), sizeof(uint32_t));
	
	// Actualizar el tamaño del buffer para incluir los datos recién agregados
	paquete->buffer->size += sizeof(uint32_t) * 3;
}

t_resize* recibir_resize(int conexion) {
    
    int size = 0;
    int offset = 0;

    void* buffer = recibir_buffer(&size, conexion);
    t_resize* resize = malloc(sizeof(t_resize));

    memcpy(&(resize->PID) , buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(resize->tam_resize),  buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(resize->cant_pags),  buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    free(buffer);
    return resize;
}

df_size* recibir_df_size(int conexion){
    df_size* io_df_size = malloc(sizeof(df_size));
    
    int size = 0;
    int offset = 0;

    void* buffer = recibir_buffer(&size, conexion);

    memcpy(&(io_df_size->reg_tamanio), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t); 
    
    memcpy(&(io_df_size->list_df_io_size), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar la lista de parámetros
    io_df_size->list_df_io = recibir_lista_parametros(buffer + offset, io_df_size->list_df_io_size);
    offset += sizeof(uint32_t) * io_df_size->list_df_io_size; 

    memcpy(&(io_df_size->offset), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t); 

    memcpy (&(io_df_size->PID), buffer + offset, sizeof(uint32_t));

    free(buffer);
    return io_df_size;
}