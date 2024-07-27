    #include "../include/receive.h"

int tamanio_pcb = sizeof(uint32_t) * 9 + sizeof(ESTADOS) + sizeof(uint8_t) * 4;

// ------------------------- Generales -------------------------
int recibir_operacion(int socket_cliente)
{
	int cod_op = 0;
    if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0){
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente) {
    // Inicializar size a 0 antes de recibir datos
    *size = 0;
    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    
    // Verificar que size es un valor positivo y razonable antes de alocar memoria
    if (*size <= 0) {
        return NULL;
    }

    void* buffer = calloc(1, *size); // Usando calloc en lugar de malloc
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "%s \n", buffer);
	free(buffer);
}

char* guardar_mensaje(int socket_cliente)
{
	int size, codigo_instruccion;
	// Pasamos el codop porque no nos importa
    recv(socket_cliente, &codigo_instruccion, sizeof(int), MSG_WAITALL);
	char* buffer = recibir_buffer(&size, socket_cliente);
	
	return buffer;
}

char* guardar_mensaje_fin_io(int socket_cliente)
{
	int size = 0;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}


t_list* recibir_paquete(int socket_cliente)
{
    int size;
    int desplazamiento = 0;
    void * buffer;
    t_list* valores = list_create();
    int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);

	memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
	while(desplazamiento < size){
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

// ------------------ Recepción de Proceso Memoria ----------------------

proceso_memoria* recibir_proceso(int socket_cliente){
    proceso_memoria* proceso = malloc(sizeof(proceso_memoria));
    int offset = 0;
    int size = 0;
    
    void* buffer = recibir_buffer(&size, socket_cliente);

    // Extraer el PID del stream del buffer
    memcpy(&(proceso->PID), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(proceso->length_path), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    proceso->path = malloc(proceso->length_path);
    memcpy(proceso->path, buffer + offset, proceso->length_path);

    free(buffer);
    return proceso;
}

t_pedir_instruccion* recibir_instruccion(int cpu_socket)
{
    t_pedir_instruccion* instruccion = malloc(sizeof(t_pedir_instruccion));
    int size;
	int offset = 0;

    void* buffer = recibir_buffer(&size, cpu_socket);

	memcpy (&instruccion->PID, buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy (&instruccion->PC, buffer + offset, sizeof(uint32_t));

    free(buffer);
    return instruccion;
}


//------------------ Recepción de PCB ----------------------
t_pcb* recibir_pcb(int conexion_cpu_dispatch){
    t_pcb* PCB = malloc(sizeof(t_pcb));
    PCB->reg = malloc(sizeof(t_registros_gral));
	int size = 0;
	int offset = 0;

    void* buffer = recv_pcb(&size, conexion_cpu_dispatch);

    // Deserializar el PID
    memcpy(&(PCB->PID), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar el valor q_restante
    memcpy(&(PCB->q_restante), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar el valor PC
    memcpy(&(PCB->PC), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

	// Deserializar el valor SI
	memcpy(&(PCB->SI), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Deserializar el valor DI
	memcpy(&(PCB->DI), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

    // Deserializar el estado del PCB
    memcpy(&(PCB->p_status), buffer + offset, sizeof(ESTADOS));
    offset += sizeof(ESTADOS);

    // Deserializar los registros generales del PCB
    memcpy(&PCB->reg->AX, buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&PCB->reg->BX, buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&PCB->reg->CX, buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&PCB->reg->DX, buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&PCB->reg->EAX, buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&PCB->reg->EBX, buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&PCB->reg->ECX, buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&PCB->reg->EDX, buffer + offset, sizeof(uint32_t));

    free(buffer);
    return PCB;
}

pcb_motivo* recibir_pcb_motivo(int conexion_cpu_dispatch){
	int size = 0;
	int offset = 0;

	pcb_motivo* paquete_pcb = malloc(sizeof(pcb_motivo));
	paquete_pcb->pcb = malloc(tamanio_pcb);
	paquete_pcb->pcb->reg = malloc(sizeof(t_registros_gral));

	void* buffer = recv_pcb(&size, conexion_cpu_dispatch);

	paquete_pcb->buffer_size = size - tamanio_pcb;

    // Deserializar el PID
    memcpy(&(paquete_pcb->pcb->PID), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar el valor q_restante
    memcpy(&(paquete_pcb->pcb->q_restante), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar el valor PC
    memcpy(&(paquete_pcb->pcb->PC), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

	// Deserializar el valor SI
	memcpy(&(paquete_pcb->pcb->SI), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Deserializar el valor DI
	memcpy(&(paquete_pcb->pcb->DI), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

    // Deserializar el estado del PCB
    memcpy(&(paquete_pcb->pcb->p_status), buffer + offset, sizeof(ESTADOS));
    offset += sizeof(ESTADOS);

    // Deserializar los registros generales del PCB
    memcpy(&(paquete_pcb->pcb->reg->AX), buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&(paquete_pcb->pcb->reg->BX), buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&(paquete_pcb->pcb->reg->CX), buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&(paquete_pcb->pcb->reg->DX), buffer + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&(paquete_pcb->pcb->reg->EAX), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(paquete_pcb->pcb->reg->EBX), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(paquete_pcb->pcb->reg->ECX), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(paquete_pcb->pcb->reg->EDX), buffer + offset, sizeof(uint32_t));

    free(buffer);
    return paquete_pcb;
}

//----------------------- Motivo desalojo--------------------------------
int recibir_nums(int conexion) {
    int num;
    int size;

    void* buffer = recibir_buffer(&size, conexion);
    memcpy(&num, buffer, sizeof(uint32_t));

    free(buffer);
    return num;
}

void *recv_pcb(int *size, int socket_cliente)
{   
    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);

    void* buffer = malloc(tamanio_pcb);
    recv(socket_cliente, buffer, tamanio_pcb, MSG_WAITALL);

    return buffer;
}

void* recv_motivo_sin_pcb(uint32_t size, int socket_cliente){
	void* buffer = malloc(size);
	recv(socket_cliente, buffer, size, MSG_WAITALL);
	return buffer;
}

//----------------------- Recepción de PCBs ----------------------------
t_io_gen *recibir_interfaz_gen(int conexion)
{
    int size = 0;
    int offset = 0;

    void *buffer = recibir_buffer(&size, conexion);

    t_io_gen *io_gen = malloc(sizeof(t_io_gen));
    if (!io_gen)
    {
        return NULL;
    }

    // Deserializar el header
    memcpy(&(io_gen->tipo_interfaz), buffer + offset, sizeof(header));
    offset += sizeof(header);

    // Deserializar la longitud del nombre de la interfaz
    memcpy(&(io_gen->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar el nombre de la interfaz
    io_gen->nombre_interfaz = malloc(io_gen->length_nombre_interfaz);
    memcpy(io_gen->nombre_interfaz, buffer + offset, io_gen->length_nombre_interfaz);
    offset += io_gen->length_nombre_interfaz;

    // Deserializar la cantidad de unidades de trabajo de la interfaz
    memcpy(&(io_gen->tiempo_unidad_trabajo), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(io_gen->fd_interfaz), buffer + offset, sizeof(uint32_t));

    free(buffer);
    return io_gen;
}

//----------------------- Recepción de PCBs ----------------------------
interfaz_a_enviar_gen* recibir_parametros_io_gen(int conexion, uint32_t size){
    interfaz_a_enviar_gen* interfaz = malloc(sizeof(interfaz_a_enviar_gen));
    int offset = 0;

    void* buffer = recv_motivo_sin_pcb(size, conexion);
    
    memcpy(&(interfaz->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    interfaz->nombre_interfaz = malloc(interfaz->length_nombre_interfaz);

    memcpy(interfaz->nombre_interfaz, buffer + offset, interfaz->length_nombre_interfaz);
    offset += interfaz->length_nombre_interfaz;

    memcpy(&(interfaz->unidades_trabajo), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    free(buffer);  
    return interfaz;
}

//----------------------- Recepción parametros interfaz ----------------------------
interfaz_a_enviar_std* recibir_parametros_interfaz_std(int conexion, uint32_t size) {
    interfaz_a_enviar_std* interfaz = malloc(sizeof(interfaz_a_enviar_std));
    int offset = 0;
    
    void* buffer = recv_motivo_sin_pcb(size, conexion);

    // Deserializar los parámetros de la interfaz
    memcpy(&(interfaz->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    interfaz->nombre_interfaz = malloc(interfaz->length_nombre_interfaz);
    memcpy(interfaz->nombre_interfaz, buffer + offset, interfaz->length_nombre_interfaz);
    offset += interfaz->length_nombre_interfaz;

    memcpy(&(interfaz->reg_tamanio), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(interfaz->offset), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(interfaz->list_df_size), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar la lista de parámetros
    interfaz->list_df = recibir_lista_parametros(buffer + offset, interfaz->list_df_size);

    free(buffer);
    return interfaz;
}

t_list* recibir_lista_parametros(void* stream, uint32_t size) {
    t_list* lista_recibida = list_create();
    // Deserializar la lista
    int offset = 0;
    for (int i = 0; i < size; i++){
        uint32_t* elemento_lista = malloc(sizeof(uint32_t));
        memcpy(elemento_lista, stream + offset, sizeof(uint32_t));
        list_add(lista_recibida, elemento_lista);
        offset += sizeof(uint32_t);
    }

    return lista_recibida;
}

char* recibir_recurso(int conexion, uint32_t size){
    int offset = 0;
    uint32_t length_nombre_recurso;
    void* buffer = recv_motivo_sin_pcb(size, conexion);
    // Deserializar la longitud del nombre del recurso
    memcpy(&(length_nombre_recurso), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    // Reservar memoria para el nombre del recurso
    char* nombre_recurso = malloc(length_nombre_recurso);
    // Copiar el nombre del recurso desde el buffer
    memcpy(nombre_recurso, buffer + offset, length_nombre_recurso);

    free(buffer);
    return nombre_recurso;
}

//----------------------- Recepción io stdin ----------------------------
t_io_std* recibir_io_std(int conexion){
	t_io_std* io_std = malloc(sizeof(t_io_std));
	
	int size = 0;
	int offset = 0;

	void* buffer = recibir_buffer(&size, conexion);

	// Deserializar el header
	memcpy(&(io_std->tipo_interfaz), buffer + offset, sizeof(header));
	offset += sizeof(header);

	// Deserializar la longitud del nombre de la interfaz
	memcpy(&(io_std->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Deserializar el nombre de la interfaz
	io_std->nombre_interfaz = malloc(io_std->length_nombre_interfaz);
	memcpy(io_std->nombre_interfaz, buffer + offset, io_std->length_nombre_interfaz);
	offset += io_std->length_nombre_interfaz;

	// Deserializar la cantidad de unidades de trabajo de la interfaz
	memcpy(&(io_std->fd_io_kernel), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(io_std->fd_io_mem), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	free(buffer);
	return io_std;
}

//----------------------- Recepción dialfs ----------------------------
t_io_df* recibir_interfaz_dialfs(int conexion){
	t_io_df* io_df = malloc(sizeof(t_io_df));
	int offset = 0;
    int size = 0;
	void* buffer = recibir_buffer(&size, conexion);

	memcpy(&(io_df->tipo_interfaz), buffer + offset, sizeof(header));
	offset += sizeof(header);

	memcpy(&(io_df->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	io_df->nombre_interfaz = malloc(io_df->length_nombre_interfaz);
	memcpy(io_df->nombre_interfaz, buffer + offset, io_df->length_nombre_interfaz);
	offset += io_df->length_nombre_interfaz;

	memcpy(&(io_df->fd_io_kernel), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(io_df->fd_io_mem), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(io_df->retraso_compactacion), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(io_df->tiempo_unidad_trabajo), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(io_df->estado), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(io_df->length_path), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    io_df->path = malloc(io_df->length_path);
    memcpy(io_df->path, buffer + offset, io_df->length_path);

    memcpy(&(io_df->block_size), buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&(io_df->block_count), buffer + offset, sizeof(int));

	free(buffer);
	return io_df;
}

//---------------------------Recibir UNIDS TRABAJO + PID-------------------------

io_gen_pid* recibir_io_gen_pid(int conexion){
    io_gen_pid* io_gen = malloc(sizeof(io_gen_pid));
	int offset = 0;
    int size = 0;
	void* buffer = recibir_buffer(&size, conexion);

    memcpy(&(io_gen->PID), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(io_gen->unidades_trabajo), buffer + offset, sizeof(uint32_t));

    free(buffer);
    return io_gen;
}

//---------------------------Recibir parámetros DIALFS-------------------------

parametros_fs* recibir_parametros_io_fs(int conexion){
    parametros_fs* io_df = malloc(sizeof(parametros_fs));
    
    int offset = 0;
    int size = 0;
    
    void* buffer = recibir_buffer(&size, conexion);   

    memcpy(&(io_df->length_nombre_archivo), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    io_df->nombre_archivo = malloc(io_df->length_nombre_archivo);
    memcpy(io_df->nombre_archivo, buffer + offset, io_df->length_nombre_archivo);
    offset += io_df->length_nombre_archivo;

    memcpy(&io_df->PID, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    memcpy(&io_df->reg_tamanio, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    free(buffer);
    return io_df;
}

//---------------------------Recibir parámetros DIALFS en Kernel-------------------------

interfaz_a_enviar_fs* recibir_parametros_kernel_fs(int conexion, uint32_t size_md){
    interfaz_a_enviar_fs* io_df = malloc(sizeof(interfaz_a_enviar_fs));
    int offset = 0;
    
    void* buffer = recv_motivo_sin_pcb(size_md, conexion);

    memcpy(&(io_df->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    io_df->nombre_interfaz = malloc(io_df->length_nombre_interfaz);
    memcpy(io_df->nombre_interfaz, buffer + offset, io_df->length_nombre_interfaz);
    offset += io_df->length_nombre_interfaz;

    memcpy(&(io_df->length_nombre_archivo), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    io_df->nombre_archivo = malloc(io_df->length_nombre_archivo);
    memcpy(io_df->nombre_archivo, buffer + offset, io_df->length_nombre_archivo);
    offset += io_df->length_nombre_archivo;

    memcpy(&io_df->reg_tamanio, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    free(buffer);
    return io_df;
}
//---------------------------Recibir parámetros FS_WRITE - FS_READ (CPU - KERNEL)----------------------
parametros_fs_rw* recibir_params_fs_rw (int conexion, uint32_t size_md){
    parametros_fs_rw* interfaz = malloc(sizeof(parametros_fs_rw));
    int offset = 0;
    
    void* buffer = recv_motivo_sin_pcb(size_md, conexion);

    memcpy(&(interfaz->length_nombre_interfaz), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    interfaz->nombre_interfaz = malloc(interfaz->length_nombre_interfaz);
    memcpy(interfaz->nombre_interfaz, buffer + offset, interfaz->length_nombre_interfaz);
    offset += interfaz->length_nombre_interfaz;

    memcpy(&(interfaz->length_nombre_archivo), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    interfaz->nombre_archivo = malloc(interfaz->length_nombre_archivo);
    memcpy(interfaz->nombre_archivo, buffer + offset, interfaz->length_nombre_archivo);
    offset += interfaz->length_nombre_archivo;

    memcpy(&interfaz->reg_tamanio, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&interfaz->ptr_arch, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    memcpy(&(interfaz->offset), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(interfaz->lista_dfs_size), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar la lista de parámetros
    interfaz->lista_dfs = recibir_lista_parametros(buffer + offset, interfaz->lista_dfs_size);

    free(buffer);
    return interfaz;
}
//---------------------------Recibir parámetros FS_WRITE - FS_READ (Kernel - IO) -------------------

params_inst_fs* recibir_params_inst_fs(int conexion){
    params_inst_fs* interfaz = malloc(sizeof(params_inst_fs));
    int offset = 0;
    int size = 0;
    
    void* buffer = recibir_buffer(&size, conexion);

    memcpy(&(interfaz->length_nombre_archivo), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    interfaz->nombre_archivo = malloc(interfaz->length_nombre_archivo);
    memcpy(interfaz->nombre_archivo, buffer + offset, interfaz->length_nombre_archivo);
    offset += interfaz->length_nombre_archivo;

    memcpy(&interfaz->PID, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&interfaz->reg_tamanio, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    memcpy(&interfaz->ptr_arch, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&interfaz->lista_dfs_size, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&interfaz->offset, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar la lista de parámetros
    interfaz->lista_dfs = recibir_lista_parametros(buffer + offset, interfaz->lista_dfs_size);

    free(buffer);
    return interfaz;
}
