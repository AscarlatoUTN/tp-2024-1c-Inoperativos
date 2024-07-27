#include "../include/send.h"

// aca ocurre toda la serializacion, deserializacion y send y recv

int SIZE_PCB = sizeof(uint32_t) * 9 + sizeof(ESTADOS) + sizeof(uint8_t) * 4;

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(header header_paquete)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = header_paquete;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;
	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_client)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_client, a_enviar, bytes, 0);
	free(a_enviar);
}


void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
} 


// ---------------------- Funciones de la Catedra ---------------------------
void enviar_mensaje(char* mensaje, int socket_cliente, header codigo_operacion){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	// Configuramos el paquete
	paquete->codigo_operacion = codigo_operacion;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);
	
	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);
	// Error al serializar
	if(a_enviar == NULL || bytes < 1)
	{
		printf("Error al serializar paquete.\n");
		// Retorna exit failure
		exit(EXIT_FAILURE);
	}

	// Enviamos el mensaje
	if(send(socket_cliente, a_enviar, bytes, 0) == -1)
	{
		printf("Error al enviar mensaje\n");
		exit(EXIT_FAILURE);
	}

	free(a_enviar); 
	eliminar_paquete(paquete);
}

void enviar_nums(header m_desalojo, int conexion, uint32_t num) {
    t_paquete* paquete = crear_paquete(m_desalojo);

    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &num, paquete->buffer->size);

    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

// ------------------ Envío de Proceso Memoria ----------------------
void agregar_a_proceso(t_paquete* paquete, proceso_memoria* proceso){

    int offset = 0;

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 2 + proceso->length_path);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &(proceso->PID), sizeof(uint32_t));
    offset += sizeof(uint32_t); 
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(proceso->length_path), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, proceso->path, proceso->length_path);
    
    paquete->buffer->size += sizeof(uint32_t) * 2 + proceso->length_path;

    free(proceso->path);
	free(proceso);
}

// -------------------------------- Envio y recepcion de PCB ---------------------------------------------

void enviar_pcb(int conexion, t_pcb* PCB, header header){
    t_paquete* pcb = crear_paquete(header);
    agregar_pcb(pcb, PCB);
    enviar_paquete(pcb, conexion);
	liberar_pcb(PCB);
    eliminar_paquete(pcb);
}

void agregar_pcb(t_paquete* paquete, t_pcb* PCB){
    int offset = 0;

    // Reasignar memoria para el stream del buffer, agregando espacio para los datos del PCB
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 9 + sizeof(ESTADOS) + sizeof(uint8_t)*4);

    // Copiar el PID del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size, &(PCB->PID), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiar el valor q_restante del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->q_restante), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiar el valor PC del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->PC), sizeof(uint32_t));
    offset += sizeof(uint32_t);

	//Copiar el valor del SI del PCB al stream del buffer
	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->SI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	//Copiar el valor del DI del PCB al stream del buffer
	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->DI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

    // Copiar el estado del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->p_status), sizeof(ESTADOS));
    offset += sizeof(ESTADOS);

    // Copiar los registros generales del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->AX), sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->BX), sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->CX), sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->DX), sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->EAX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->EBX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->ECX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(PCB->reg->EDX), sizeof(uint32_t));

    // Actualizar el tamaño del buffer para incluir los datos recién agregados
    paquete->buffer->size += sizeof(uint32_t) * 9 + sizeof(ESTADOS) + sizeof(uint8_t) * 4;
}

// -------------------Envío de PC--------------------------------------------

void pedir_instruccion(t_pedir_instruccion* instruccion, int conexion_memoria, header codigo_operacion) {
    t_paquete* paquete = crear_paquete(codigo_operacion);
    serializar_instruccion(paquete, instruccion);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void serializar_instruccion(t_paquete* paquete, t_pedir_instruccion* instruccion) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 2);
    
    int offset = 0;
    memcpy(paquete->buffer->stream  + offset, &instruccion->PID, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream  + offset, &instruccion->PC, sizeof(uint32_t));
	
    paquete->buffer->size += sizeof(uint32_t)*2;
}

// ------------------ Envío y Recepción de Interfaz Genérica ----------------------
void enviar_interfaz_gen(header header, int conexion, t_io_gen* io_gen){
	t_paquete* paquete = crear_paquete(header);
	agregar_interfaz_gen(paquete, io_gen);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_interfaz_gen(t_paquete* paquete, t_io_gen* io_gen){
	
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(header) + sizeof(uint32_t) * 3 + io_gen->length_nombre_interfaz);

	// Copiar el header al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size, &(io_gen->tipo_interfaz), sizeof(header));
	offset += sizeof(header);

	// Copiar la longitud del nombre de la interfaz al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_gen->length_nombre_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Copiar el nombre de la interfaz al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, io_gen->nombre_interfaz, io_gen->length_nombre_interfaz);
	offset += io_gen->length_nombre_interfaz;
	
	// Copiar la cantidad de unidades de trabajo de la interfaz al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_gen->tiempo_unidad_trabajo), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_gen->fd_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Actualizar el tamaño del buffer para incluir los datos recién agregados
	paquete->buffer->size += sizeof(header) + sizeof(uint32_t) * 3 + io_gen->length_nombre_interfaz;
}

//------------------- Envio Y Recepcion de Interfaz IO_GEN + PCB --------------------------------

void enviar_interfaz_io_gen(header m_header, interfaz_a_enviar_gen* interfaz, int conexion) {
    t_paquete* paquete = crear_paquete(m_header);
    agregar_interfaz_io_gen(paquete, interfaz);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}


void agregar_interfaz_io_gen (t_paquete* paquete, interfaz_a_enviar_gen* interfaz){

	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 11 + sizeof(ESTADOS) + sizeof (uint8_t) *4 + interfaz->length_nombre_interfaz);

    // Copiar PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size, &(interfaz->PCB->PID), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->q_restante), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->PC), sizeof(uint32_t));
    offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->SI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->DI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->p_status), sizeof(ESTADOS));
    offset += sizeof(ESTADOS);

	// Registros

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->AX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->BX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->CX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->DX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EAX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EBX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->ECX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EDX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, (interfaz->nombre_interfaz), interfaz->length_nombre_interfaz);
	offset += interfaz->length_nombre_interfaz;

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->unidades_trabajo), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	// Actualizo el valor de size del buffer
    paquete->buffer->size += sizeof(uint32_t) * 11 + sizeof(ESTADOS) + sizeof(uint8_t)*4 + interfaz->length_nombre_interfaz;
}

// ------------------ Envío y Recepción de Interfaz IO STDIN + PCB ----------------------
void enviar_parametros_interfaz_std(header m_header, interfaz_a_enviar_std* interfaz, int conexion) {
    t_paquete* paquete = crear_paquete(m_header);
    agregar_interfaz_io_std(paquete, interfaz);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void agregar_interfaz_io_std(t_paquete* paquete, interfaz_a_enviar_std* interfaz){
	
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 13 + sizeof(ESTADOS) + sizeof (uint8_t) * 4 + interfaz->length_nombre_interfaz + interfaz->list_df_size * sizeof(uint32_t));

    // Copiar PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size, &(interfaz->PCB->PID), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->q_restante), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->PC), sizeof(uint32_t));
    offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->SI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->DI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->p_status), sizeof(ESTADOS));
    offset += sizeof(ESTADOS);

    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->AX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->BX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->CX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->DX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EAX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EBX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->ECX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EDX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Copiar los parametros de la interfaz al stream del buffer

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, (interfaz->nombre_interfaz), interfaz->length_nombre_interfaz);
	offset += interfaz->length_nombre_interfaz;

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->reg_tamanio), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->list_df_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	for (int i = 0; i < interfaz->list_df_size; i++){
		memcpy (paquete->buffer->stream + paquete->buffer->size + offset, list_get(interfaz->list_df,i), sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}
	// Actualizo el valor de size del buffer
    paquete->buffer->size += offset;
}

// ------------------ Envío y Recepción de Interfaz STDIN ----------------------

void enviar_interfaz_std(header header, int conexion, t_io_std* io_stdin){
	t_paquete* paquete = crear_paquete(header);
	agregar_interfaz_std(paquete, io_stdin);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_interfaz_std(t_paquete* paquete, t_io_std* io_stdin){
	
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(header) + sizeof(uint32_t) * 3 + io_stdin->length_nombre_interfaz);

	// Copiar el header al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size, &(io_stdin->tipo_interfaz), sizeof(header));
	offset += sizeof(header);

	// Copiar la longitud del nombre de la interfaz al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_stdin->length_nombre_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Copiar el nombre de la interfaz al stream del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, io_stdin->nombre_interfaz, io_stdin->length_nombre_interfaz);
	offset += io_stdin->length_nombre_interfaz;
	
	// Copiar el fd de la conexión con kernel
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_stdin->fd_io_kernel), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Copiar el fd de la conexión con kernel
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_stdin->fd_io_mem), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Actualizar el tamaño del buffer para incluir los datos recién agregados
	paquete->buffer->size += sizeof(header) + sizeof(uint32_t) * 3 + io_stdin->length_nombre_interfaz;
}

//----------------------------Envio y Recepcion de Interfaz FS + PCB--------------------------
/*
interfaz_a_enviar_fs* recibir_parametros_interfaz_fs(int conexion, uint32_t size){
    interfaz_a_enviar_fs* interfaz = malloc(sizeof(interfaz_a_enviar_fs));
    int offset = 0;

    void* buffer = recv_motivo_sin_pcb(size, conexion);
    
	memcpy(&(interfaz->parametros->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	interfaz->parametros->nombre_interfaz = malloc(interfaz->parametros->length_nombre_interfaz);

	memcpy(interfaz->parametros->nombre_interfaz, buffer + offset, interfaz->parametros->length_nombre_interfaz);
	offset += interfaz->parametros->length_nombre_interfaz;

	memcpy(&(interfaz->parametros->unidades_trabajo), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(interfaz->parametros->reg_direccion), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(interfaz->parametros->reg_tamanio), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(interfaz->parametros->length_nombre_archivo), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	interfaz->parametros->nombre_archivo = malloc(interfaz->parametros->length_nombre_archivo);

	memcpy(interfaz->parametros->nombre_archivo, buffer + offset, interfaz->parametros->length_nombre_archivo);
	offset += interfaz->parametros->length_nombre_archivo;

	memcpy(&(interfaz->parametros->ptr_arch), buffer + offset, sizeof(uint32_t));
	
    free(buffer);  
    return interfaz;
}
*/

//-------------------------- WAIT y SIGNAL -------------------------------
void enviar_pcb_recurso(header m_header, enviar_pcb_rec* pcb_rec, int conexion){
	t_paquete* paquete = crear_paquete(m_header);
	agregar_pcb_recurso(paquete, pcb_rec);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_pcb_recurso(t_paquete* paquete, enviar_pcb_rec* pcb_rec){
    int offset = 0;

    // Reasignar memoria para el stream del buffer, agregando espacio para los datos del PCB
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 6 + sizeof(ESTADOS) + sizeof(t_registros_gral) + pcb_rec->length_nombre_recurso);

    // Copiar el PID del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size, &(pcb_rec->PCB->PID),sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiar el valor q_restante del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(pcb_rec->PCB->q_restante), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiar el valor PC del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(pcb_rec->PCB->PC), sizeof(uint32_t));
    offset += sizeof(uint32_t);

	//Copiar el valor del SI del PCB al stream del buffer
	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(pcb_rec->PCB->SI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	//Copiar el valor del DI del PCB al stream del buffer
	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(pcb_rec->PCB->DI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

    // Copiar el estado del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(pcb_rec->PCB->p_status), sizeof(ESTADOS));
    offset += sizeof(ESTADOS);

    // Copiar los registros generales del PCB al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + offset, pcb_rec->PCB->reg, sizeof(t_registros_gral));
	offset += sizeof (t_registros_gral);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(pcb_rec->length_nombre_recurso), sizeof (uint32_t));
	offset += sizeof (uint32_t);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, pcb_rec->nombre_recurso, pcb_rec->length_nombre_recurso);

    // Actualizar el tamaño del buffer para incluir los datos recién agregados
    paquete->buffer->size += sizeof(uint32_t) * 6 + sizeof(ESTADOS) + sizeof(t_registros_gral) + pcb_rec->length_nombre_recurso;
}

void agregar_lista(t_list* lista_a_enviar, t_paquete* paquete){
    if (!list_is_empty(lista_a_enviar)){
        for (int i=0; i<list_size(lista_a_enviar); i++){
            void* elem_lista = list_get(lista_a_enviar, i);
            agregar_a_paquete(paquete, elem_lista, sizeof(uint32_t));
        }
    } 
}

//------------------------Envio y recepcion estructura Df_size-----------------------------
void enviar_gen_pid(int conexion, io_gen_pid* io_gen){
	t_paquete* paquete = crear_paquete (UNIDS_TRABAJO);
	agregar_io_gen_pid(paquete, io_gen);
	enviar_paquete(paquete, conexion);
	eliminar_paquete (paquete);
}

void agregar_io_gen_pid(t_paquete* paquete, io_gen_pid* io_gen){
	int offset = 0;
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 2);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(io_gen->PID), sizeof(uint32_t));
	offset += sizeof (uint32_t);

	memcpy (paquete->buffer->stream + paquete->buffer->size + offset, &(io_gen->unidades_trabajo), sizeof(uint32_t));
	offset += sizeof (uint32_t);

	paquete->buffer->size += offset;
}

//------------------------Envio y recepcion estructura Df_size-----------------------------
void enviar_df_size(header header, int conexion, df_size* io_df_size){
    t_paquete* paquete = crear_paquete(header);
    agregar_df_size(paquete, io_df_size);   
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void agregar_df_size(t_paquete* paquete, df_size* io_df_size){
	
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 4 + io_df_size->list_df_io_size * sizeof(uint32_t));

	// Copiar el registro tamaño
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df_size->reg_tamanio), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df_size->list_df_io_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	for (int i = 0; i < io_df_size->list_df_io_size; i++){
		memcpy (paquete->buffer->stream + paquete->buffer->size + offset, list_get(io_df_size->list_df_io, i), sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df_size->offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df_size->PID), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	// Actualizar el tamaño del buffer para incluir los datos recién agregados
	paquete->buffer->size += sizeof(uint32_t) * 4 + io_df_size->list_df_io_size * sizeof(uint32_t);
}

// ------------------ Envío y Recepción de Interfaz DIALFS ----------------------

void enviar_interfaz_dialfs(header header, int conexion, t_io_df* io_df){
	t_paquete* paquete = crear_paquete(header);
	agregar_interfaz_dialfs(paquete, io_df);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_interfaz_dialfs(t_paquete* paquete, t_io_df* io_df){
	
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(header) + sizeof(uint32_t) * 9 + io_df->length_path + io_df->length_nombre_interfaz);

	memcpy(paquete->buffer->stream + paquete->buffer->size, &(io_df->tipo_interfaz), sizeof(header));
	offset += sizeof(header);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->length_nombre_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, io_df->nombre_interfaz, io_df->length_nombre_interfaz);
	offset += io_df->length_nombre_interfaz;

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->fd_io_kernel), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->fd_io_mem), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->retraso_compactacion), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->tiempo_unidad_trabajo), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->estado), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Agregar length_path
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->length_path), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Agregar path
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, io_df->path, io_df->length_path);
	offset += io_df->length_path;

	// Agregar block_size
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->block_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Agregar block_count
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(io_df->block_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	paquete->buffer->size += offset;
}


//-----------------------------Envio y recepcion de estructura Df_size--------------------------------

void enviar_interfaz_fs(header header, interfaz_a_enviar_fs* interfaz, int conexion){
    t_paquete* paquete = crear_paquete(header);
	agregar_interfaz_fs(paquete, interfaz);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_interfaz_fs(t_paquete* paquete, interfaz_a_enviar_fs* interfaz){

	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 13 + sizeof(ESTADOS) + interfaz->length_nombre_interfaz + interfaz->length_nombre_archivo);

	// Copiar PCB 
	memcpy(paquete->buffer->stream + paquete->buffer->size, &(interfaz->PCB->PID), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->q_restante), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->PC), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->SI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->DI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->p_status), sizeof(ESTADOS));
	offset += sizeof(ESTADOS);

	//Copiar registros
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->AX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->BX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->CX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->DX), sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EAX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EBX), sizeof(uint32_t));	
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->ECX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EDX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Copiar los parametros de la interfaz 
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset,interfaz->nombre_interfaz, interfaz->length_nombre_interfaz);
	offset += interfaz->length_nombre_interfaz;
	
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_archivo), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset,interfaz->nombre_archivo, interfaz->length_nombre_archivo);
	offset += interfaz->length_nombre_archivo;

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->reg_tamanio), sizeof (uint32_t));
	offset += sizeof(uint32_t);
	
	paquete->buffer->size += offset;
}

//---------------------Enviar nombre interfaz y archivo-------------------------

void enviar_parametros_fs(header header, parametros_fs* interfaz, int conexion){
    t_paquete* paquete = crear_paquete(header);
	agregar_parametros_fs(paquete, interfaz);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_parametros_fs(t_paquete* paquete, parametros_fs* interfaz){

	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 3 + interfaz->length_nombre_archivo);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_archivo), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, interfaz->nombre_archivo, interfaz->length_nombre_archivo);
	offset += interfaz->length_nombre_archivo;

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PID), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->reg_tamanio), sizeof (uint32_t));
	offset += sizeof(uint32_t);

	paquete->buffer->size += offset;
}


//---------------------Enviar nombre interfaz y archivo-------------------------

void enviar_param_fs_rw(header header, parametros_fs_rw* interfaz, int conexion){
    t_paquete* paquete = crear_paquete(header);
	agregar_parametros_fs_rw(paquete, interfaz);
	//liberar_pcb (interfaz->PCB);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_parametros_fs_rw(t_paquete* paquete, parametros_fs_rw* interfaz){

	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint8_t) * 4 + sizeof(uint32_t) * 16 + sizeof(ESTADOS) + sizeof (uint32_t) * interfaz->lista_dfs_size + interfaz->length_nombre_interfaz + interfaz->length_nombre_archivo);

	// Copiar PCB 
	memcpy(paquete->buffer->stream + paquete->buffer->size, &(interfaz->PCB->PID), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->q_restante), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->PC), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->SI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->DI), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->p_status), sizeof(ESTADOS));
	offset += sizeof(ESTADOS);

	//Copiar registros
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->AX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->BX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->CX), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->DX), sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EAX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EBX), sizeof(uint32_t));	
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->ECX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PCB->reg->EDX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	//Parametros de la interfaz
	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_interfaz), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, interfaz->nombre_interfaz, interfaz->length_nombre_interfaz);
	offset += interfaz->length_nombre_interfaz;

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_archivo), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, interfaz->nombre_archivo, interfaz->length_nombre_archivo);
	offset += interfaz->length_nombre_archivo;

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->reg_tamanio), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->ptr_arch), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->lista_dfs_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	for (int i = 0; i < interfaz->lista_dfs_size; i++){
		memcpy (paquete->buffer->stream + paquete->buffer->size + offset, list_get(interfaz->lista_dfs,i), sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}

	paquete->buffer->size += offset;
}

void enviar_params_inst(header header, params_inst_fs* interfaz, int conexion){
	t_paquete* paquete = crear_paquete(header);
	agregar_parametros_inst_mem(paquete, interfaz);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_parametros_inst_mem(t_paquete* paquete, params_inst_fs* interfaz){
	int offset = 0;

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t) * 6 + interfaz->length_nombre_archivo + interfaz->lista_dfs_size * sizeof(uint32_t));

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->length_nombre_archivo), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, interfaz->nombre_archivo, interfaz->length_nombre_archivo);
	offset += interfaz->length_nombre_archivo;

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->PID), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->reg_tamanio), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->ptr_arch), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->lista_dfs_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + paquete->buffer->size + offset, &(interfaz->offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	for (int i = 0; i < interfaz->lista_dfs_size; i++){
		memcpy (paquete->buffer->stream + paquete->buffer->size + offset, list_get(interfaz->lista_dfs,i), sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}
	paquete->buffer->size += offset;
}