#include "../include/ops_dialfs.h"

t_bitarray* bitmap;
bool SET = true;
bool CLEAN = false;
void* ptr_mmap_bitmap;
void* ptr_mmap_mem;
size_t bitmap_size = 0;
int FREE_BLOCK_COUNT = 0;
// ----------------------------------------- INICIALIZACION ------------------------------------
// ---------------- Crear Archivos Bitmap y Bloques
void* crear_archivo_bitmap() {
    bitmap_size = ceil((double)BLOCK_COUNT / 8);
    ptr_mmap_bitmap = crear_y_mapear_archivo("bitmap.dat", bitmap_size); // puntero al archivo mapeado en memoria
    if (ptr_mmap_bitmap == NULL) {
        return NULL;
    }

    
    bitmap = bitarray_create_with_mode(ptr_mmap_bitmap, bitmap_size, LSB_FIRST);
    return bitmap;
}

void* crear_archivo_bloques() {
    size_t bloques_size = BLOCK_COUNT * BLOCK_SIZE;
    return crear_y_mapear_archivo("bloques.dat", bloques_size);
}

// ---------------- Funciones Auxiliares
// Construir nombre de archivo
char* construir_ruta_archivo(char* filename) {
    char* path = malloc(strlen(PATH_BASE_DIALFS) + strlen(filename) + 2);
    strcpy(path, PATH_BASE_DIALFS);
    strcat(path, "/");
    strcat(path, filename);
    return path;
}

// Abrir o crear un archivo y ajustar su tamaño
int abrir_archivo(char* path, size_t size) {
    int fd = open(path, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd == -1) {
        log_error(logger_io_extra, "Error al abrir/crear el archivo %s\n", path);
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        log_error(logger_io_extra, "Error al obtener información del archivo %s\n", path);
        close(fd);
        return -1;
    }

    if (st.st_size < size) {
        if (ftruncate(fd, size) == -1) {
            log_error(logger_io_extra, "Error al cambiar el tamaño del archivo %s\n", path);
            close(fd);
            return -1;
        }
    }
    return fd;
}

// Mapear un archivo a memoria
void* mapear_archivo(int fd, size_t size, char* path) {
    ptr_mmap_mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr_mmap_mem == MAP_FAILED) {
        log_error(logger_io_extra, "Error al mapear el archivo %s en memoria\n", path);
        return NULL;
    }
    return ptr_mmap_mem;
}

// Función principal que utiliza las anteriores para crear y mapear un archivo
void* crear_y_mapear_archivo(char* filename, size_t size) {
    char* path = construir_ruta_archivo(filename);
    int fd = abrir_archivo(path, size);
    if (fd == -1) {
        free(path);
        return NULL;
    }

    ptr_mmap_mem = mapear_archivo(fd, size, path);
    if (ptr_mmap_mem == NULL) {
        free(path);
        return NULL;
    }

    free(path);
    return ptr_mmap_mem;
}

void buscar_directorio() {
    DIR *dir; // Puntero al directorio
    struct dirent *entrada; // Estructura para cada entrada en el directorio

    // Intenta abrir el directorio
    char* path = strdup(PATH_BASE_DIALFS);
    string_append(&path, "/");
    dir = opendir(path);
    if (dir == NULL) {
        // No se pudo abrir el directorio
        return;
    }

    // Itera sobre cada entrada en el directorio
    while ((entrada = readdir(dir)) != NULL) {
        // Verifica si el archivo termina en .txt
        char *ext = strstr(entrada->d_name, ".txt");
        if (ext == NULL || *(ext + 4) != '\0') {
            // Si no es un archivo .txt, continúa con la siguiente entrada
            continue;
        }
        t_fcb* fcb = malloc (sizeof(t_fcb));
        fcb->nombre_archivo = strdup(entrada->d_name);
        fcb->arch_metadata = config_create(construir_ruta_archivo(fcb->nombre_archivo));
        fcb->bloque_base = config_get_int_value(fcb->arch_metadata, "BLOQUE_INICIAL");
        fcb->tamanio = config_get_int_value(fcb->arch_metadata, "TAMANIO_ARCHIVO");
        pthread_mutex_lock(&mutex_lista_fcbs);
        list_add (lista_fcbs, fcb);
        pthread_mutex_unlock(&mutex_lista_fcbs);
    }
    closedir(dir);
}

// ------------------------------------ OPERACIONES ---------------------------------------
uint32_t actualizar_espacio_disco(char* nombre_archivo, header accion)
{
    switch (accion){
        case IO_FS_CREATE:
            uint32_t bloque_base = buscar_fst_bloque_libre(); // Encontrar 1er bloque libre en el bitmap
            if(status_bloque(bloque_base, SET, 0) == UINT32_MAX) // Marcar el bloque como ocupado
                break;
                
            return bloque_base;
            
        case IO_FS_DELETE:

            t_fcb* fcb_d = sacar_fcb_lista(nombre_archivo);
            if (fcb_d == NULL) break;

            int cant_bloques = calcular_bloques(fcb_d->tamanio);
            if(deallocate_blocks(fcb_d->bloque_base, cant_bloques) == UINT32_MAX) {
                liberar_fcb(fcb_d);
                break;
            }
                 
            liberar_fcb(fcb_d);
            return 0;

        default:
            break;
    }
    return UINT32_MAX;
}



//------------------------ FS_CREATE y FS_DELETE
void FS_CREATE(char* path, char* nombre_archivo, uint32_t bloque_base) {

    t_fcb* fcb_c = inicializar_fcb(nombre_archivo);
    if (fcb_c == NULL) exit(EXIT_FAILURE);

    char* path_completo = construir_ruta_archivo(nombre_archivo);
    fcb_c->arch_metadata = crear_archivo_metadata(path_completo);
    free(path_completo);
    if (fcb_c->arch_metadata == NULL) {
        liberar_fcb(fcb_c);
        exit(EXIT_FAILURE);
    }

    escribir_metadata(fcb_c, bloque_base, 0);

    pthread_mutex_lock(&mutex_lista_fcbs);
    list_add(lista_fcbs, fcb_c);
    pthread_mutex_unlock(&mutex_lista_fcbs);
    //free(path_metadata);va o no va? no se
}

bool FS_DELETE(char* path, char* nombre_archivo){
    char* path_completo = construir_ruta_archivo(nombre_archivo);
    bool success = (remove(path_completo) == 0);
    if (success) {
        log_info(logger_io_extra, "Archivo %s eliminado con éxito.\n", path_completo);
    } else {
        log_error(logger_io_extra, "Error al eliminar el archivo %s\n", path_completo);
    }
    free(path_completo);
    return success;
}

bool FS_WRITE(params_inst_fs* recibido_w, void* dato_escribir, t_fcb* fcb){ 
    // Obtener posición inicial y calcular cantidad de bloques a escribir
    int byte_inicial = fcb->bloque_base * BLOCK_SIZE + recibido_w->ptr_arch;
    return escribir_arch_bloques(dato_escribir, recibido_w->reg_tamanio, byte_inicial);
}

char* FS_READ(char* nombre_archivo, int tamanio_leer, int ptr_arch){
    
    t_fcb* fcb = get_fcb_lista(nombre_archivo); // Buscar fcb del archivo
    if (fcb == NULL) {
        log_error(logger_io_extra, "No se encontró el archivo %s en la lista de archivos \n", nombre_archivo);
        return NULL;
    }
    
    // Verificar que esté dentro de los límites
    if (ptr_arch < 0 || ptr_arch + tamanio_leer > fcb->tamanio) {
        log_error(logger_io_extra, "Lectura inválida en el archivo %s \n", nombre_archivo);
        return NULL;
    }

    char* dato_leido = malloc(tamanio_leer);
    if (dato_leido == NULL) {
        log_error(logger_io_extra, "Error al reservar memoria para lectura \n");
        return NULL;
    }
    
    // Obtener posición inicial y calcular cantidad de bloques a leer
    int byte_init_leer = fcb->bloque_base * BLOCK_SIZE + ptr_arch;

    // Leer los datos desde los bloques hasta que se haya leído el tamaño solicitado
    pthread_mutex_lock(&mutex_archivo_bloques);
    memcpy(dato_leido, ptr_mmap_mem + byte_init_leer, tamanio_leer);
    pthread_mutex_unlock(&mutex_archivo_bloques);
    
    dato_leido [tamanio_leer] = '\0'; // Agregar fin de cadena
    
    return dato_leido;
}

// Crear y abrir archivo de metadata
t_config* crear_archivo_metadata(char* path_completo) {
    int fd = open(path_completo, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd == -1) {
        log_error(logger_io_extra, "Error al abrir/crear el archivo %s\n", path_completo);
        return NULL;
    }
    close(fd);
    return config_create(path_completo);
}

void escribir_metadata(t_fcb* fcb, uint32_t bloque_base, int tamanio) {

    fcb->tamanio = tamanio; // En bytes
    fcb->bloque_base = bloque_base; // Marcar bloque_base como base.

    char bloque_base_config[32];
    char tamanio_config[32];
    snprintf(bloque_base_config, sizeof(bloque_base_config), "%u", bloque_base);
    snprintf(tamanio_config, sizeof(tamanio_config), "%d", tamanio);

    config_set_value(fcb->arch_metadata, "BLOQUE_INICIAL", bloque_base_config);
    config_set_value(fcb->arch_metadata, "TAMANIO_ARCHIVO", tamanio_config);
    config_save(fcb->arch_metadata);
}

// Función para verificar si un archivo ya existe en un directorio (agarré la del commit porque no estaba acá)
bool archivo_duplicado(char* nombre_archivo) {
    t_fcb* fcb = get_fcb_lista(nombre_archivo);
    bool result = fcb == NULL ? false : true; 
    return result;
}

//------------------------ FS_TRUNCATE
bool FS_TRUNCATE(parametros_fs* recibido) {
    t_fcb* fcb_d = get_fcb_lista(recibido->nombre_archivo);
    if (!fcb_d) {
        log_error(logger_io_extra, "No se encontró el archivo %s \n", recibido->nombre_archivo);
        return false;
    }

    uint32_t cant_bloques_actuales = calcular_bloques(fcb_d->tamanio);
    uint32_t cant_bloques_nuevos = calcular_bloques(recibido->reg_tamanio); 

    if(cant_bloques_nuevos > BLOCK_COUNT) {
        log_error(logger_io_extra, "Out of Disk. EL tamaño del archivo %s es mayor al del FS \n", recibido->nombre_archivo);
        return false; 
    } 
    
    if(cant_bloques_actuales == cant_bloques_nuevos) {
        if (fcb_d->tamanio != recibido->reg_tamanio)
            escribir_metadata(fcb_d, fcb_d->bloque_base, recibido->reg_tamanio);

        log_info(logger_io_extra, "ARCHIVO %s: no es necesario truncar tamaño \n", recibido->nombre_archivo);
        return true;   
    } 
    
    if(cant_bloques_nuevos > cant_bloques_actuales) {
        uint32_t nuevos_bloques = cant_bloques_nuevos - cant_bloques_actuales;
        return aumentar_archivo(fcb_d, cant_bloques_actuales, nuevos_bloques, recibido->reg_tamanio, recibido->PID);
    } else {
        uint32_t nuevos_bloques = cant_bloques_actuales - cant_bloques_nuevos;
        return disminuir_archivo(fcb_d, cant_bloques_actuales, nuevos_bloques, recibido->reg_tamanio);
    }
}

uint32_t calcular_bloques(uint32_t tamanio) {
    return (uint32_t)fmax(ceil((double)tamanio / BLOCK_SIZE), 1);
}

bool disminuir_archivo(t_fcb* fcb, uint32_t cant_bloques_actuales, uint32_t bloques_eliminar, uint32_t tamanio_nuevo){
    
    int bloque_init_borrar = cant_bloques_actuales - bloques_eliminar;
    if (deallocate_blocks(bloque_init_borrar, bloques_eliminar) == UINT32_MAX) // Liberar bloques del bitmap, y reescribir el archivo bloques.dat a 0
        return false;
        
    escribir_metadata(fcb, fcb->bloque_base, tamanio_nuevo);
    return true;
}

bool aumentar_archivo(t_fcb* fcb, uint32_t cant_bloques_actuales, uint32_t bloques_aumentar, uint32_t tamanio_nuevo, uint32_t PID) {

    pthread_mutex_lock(&mutex_block_count); 
    if(FREE_BLOCK_COUNT <= bloques_aumentar) {
        log_error(logger_io_extra, "ARCHIVO %s - Tamanio a ampliar: %d mayor a cantidad de espacio libre: %d \n", fcb->nombre_archivo, tamanio_nuevo, FREE_BLOCK_COUNT);
        pthread_mutex_unlock(&mutex_block_count);
        return false;
    }
    pthread_mutex_unlock(&mutex_block_count);
    
    // Primer bloque desde el cual ampliar
    uint32_t bloque_init_ampliar = espacio_contiguo_ady(fcb->bloque_base, bloques_aumentar, cant_bloques_actuales);
    if(bloque_init_ampliar == UINT32_MAX) {
        log_warning(logger_io_extra, "No hay espacio contiguo adyacente suficiente \n");

        usleep(RETRASO_COMPACTACION);
        bloque_init_ampliar = espacio_contiguo_disp(fcb->bloque_base, bloques_aumentar, cant_bloques_actuales, fcb->nombre_archivo);

        if(bloque_init_ampliar == UINT32_MAX){
            log_info(logger_io, "PID: %d - Inicio Compactación. \n", PID);
            bloque_init_ampliar = compactar_fs(fcb->nombre_archivo, fcb->tamanio);
            log_info(logger_io, "PID: %d - Fin Compactación. \n", PID);    
        }
    }
    // Aumentar el tamaño del archivo
    if (allocate_blocks(bloque_init_ampliar, bloques_aumentar) == UINT32_MAX) /// Está dentro de espacio_contiguo_disp
        return false;
        
    escribir_metadata(fcb, bloque_init_ampliar - cant_bloques_actuales, tamanio_nuevo);
    return true;
}

uint32_t espacio_contiguo_ady(uint32_t bloque_inicial, uint32_t bloques_ampliar, uint32_t cant_bloques_actuales){

    uint32_t final_archivo = bloque_inicial + cant_bloques_actuales; // Primer bloque que no contenga el archivo
    uint32_t bloques_contiguos = 0; 
    for (int i = final_archivo; i < bloques_ampliar + final_archivo; i++){
        if(bitarray_test_bit(bitmap,i))
            break;
        bloques_contiguos++;
    }
    return (bloques_contiguos < bloques_ampliar) ? UINT32_MAX : final_archivo;
}

uint32_t espacio_contiguo_disp(uint32_t bloque_arch_inicial, uint32_t bloques_ampliar, uint32_t cant_bloques_actuales, char* path){

    uint32_t bloques_contiguos = 0;
    uint32_t bloque_init_ampliar = 0;
    uint32_t nuevo_bloque_base = 0;
    uint32_t tamano_archivo = bloques_ampliar + cant_bloques_actuales;
    
    // Buscar un espacio contiguo lo suficientemente grande para TODO el archivo
   for (int bloque_actual = 0; bloque_actual < BLOCK_COUNT; bloque_actual++){
        if(bitarray_test_bit(bitmap,bloque_actual)){
            bloques_contiguos = 0; // Si está ocupado el bloque, anular los bloques contiguos hasta el momento            
        }else {
            bloques_contiguos++; // Si no está ocupado, aumentar la cant de bloques contiguos
            
            // Comprobar si el espacio hallado ya es suficientemente grande para el archivo
            if (bloques_contiguos >= tamano_archivo) {
                nuevo_bloque_base = bloque_actual - tamano_archivo + 1;
		bloque_init_ampliar = bloque_actual - bloques_ampliar + 1;
                break;
            }            
        }
    }

    if (bloques_contiguos < tamano_archivo) {
        log_warning(logger_io_extra, "No hay bloques contiguos suficientes disponibles en el FS\n");
        return UINT32_MAX;
    }
    
    // Si hay suficientes bloques, movemos el archivo a ese lugar
    // 1. Tomo el bloque inicial y a partir de ahi cambio los bits necesarios a ocupado
    
    if (allocate_blocks(nuevo_bloque_base, tamano_archivo) == UINT32_MAX)
        return UINT32_MAX;

    // 2. Copiar datos a buffer y liberar bloques antiguos
    uint32_t tamanio_archivo_actual = BLOCK_SIZE * cant_bloques_actuales;    //  En bytes. No importa el offset en el ultimo bloque, igual lo ocupa completo
    
    void* buffer = malloc(tamanio_archivo_actual);
    if (buffer == NULL) {
        log_error(logger_io_extra, "Error al reservar memoria para buffer de archivo\n");
        return UINT32_MAX;
    }

    int byte_inicio = bloque_arch_inicial * BLOCK_SIZE;
    int new_init_byte = nuevo_bloque_base * BLOCK_SIZE;

    pthread_mutex_lock(&mutex_archivo_bloques);
    memcpy(buffer, ptr_mmap_mem + byte_inicio, tamanio_archivo_actual); // Copio el contenido de bloques.dat del archivo al buffer
    pthread_mutex_unlock(&mutex_archivo_bloques);
    
    // Marco los actuales bloques del archivo como libres en el bitmap
    if(deallocate_blocks(bloque_arch_inicial, cant_bloques_actuales) == UINT32_MAX){
        free(buffer);
        return UINT32_MAX;
    }

    // 3. Colocar el contenido del archivo en su nuevo espacio en bloques.dat
    if(!escribir_arch_bloques(buffer, tamanio_archivo_actual, new_init_byte)) {
        free(buffer);
        return UINT32_MAX;
    }
        
    free(buffer);
    return bloque_init_ampliar; // result + cant_bloques_actuales + 1 (lo que estaba antes)
}

bool escribir_arch_bloques(void* buffer, int tamanio, int byte_inicial){

    // Escribir los datos en bloques hasta que se haya escrito el tamaño solicitado
    pthread_mutex_lock(&mutex_archivo_bloques);
    memcpy(ptr_mmap_mem + byte_inicial, buffer, tamanio); 

    if (msync(ptr_mmap_mem, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC) == -1){
        log_warning (logger_io_extra, "Error al sincronizar el bloques.dat \n");    
        pthread_mutex_unlock(&mutex_archivo_bloques);
        free(buffer); // no se si hay que hacer esto
        return false;
    } 
    pthread_mutex_unlock(&mutex_archivo_bloques);
    return true;
}

//-------------------------------------------------COMPACTAR--------------------------------------------
uint32_t compactar_fs(char* nombre_archivo, int tamanio_arch) {

    int offset = 0;
    int buffer_general_size = BLOCK_SIZE * BLOCK_COUNT - tamanio_arch;
    
    void* buffer_general = malloc(buffer_general_size); //Resto de bloques
    memset(buffer_general, 0, buffer_general_size); //Limpiamos todo el buffer

    void* buffer_especial = malloc(tamanio_arch); //Bloque que queremos truncar
    memset(buffer_especial, 0, tamanio_arch); //Limpiamos todo el buffer

    
    if (buffer_general == NULL || buffer_especial == NULL) {
        log_error(logger_io_extra, "Error al asignar memoria para los buffers auxiliares de compactación \n");
        free(buffer_general);
        free(buffer_especial);
        return UINT32_MAX;
    }
       
    for (int i = 0; i < list_size(lista_fcbs); i++) {
        t_fcb* archivo = list_get(lista_fcbs, i);
        if (archivo == NULL) {
            log_error(logger_io_extra, "Error al obtener archivo de la lista de archivos metadata \n");
            free(buffer_general);
            free(buffer_especial);
            return UINT32_MAX;
        }

        bool es_archivo_especial = strcmp(archivo->nombre_archivo, nombre_archivo) == 0;
        void* buffer = es_archivo_especial ? buffer_especial : buffer_general + offset * BLOCK_SIZE;

        pthread_mutex_lock(&mutex_archivo_bloques);
        memcpy(buffer, ptr_mmap_mem + archivo->bloque_base * BLOCK_SIZE, archivo->tamanio);
        pthread_mutex_unlock(&mutex_archivo_bloques);
        
        int bloques_archivo = calcular_bloques(archivo->tamanio);
        
        if (!es_archivo_especial) {
            int nuevo_bloque_base = offset;
            offset += bloques_archivo;
            escribir_metadata(archivo, nuevo_bloque_base, archivo->tamanio);
        }
    }

    deallocate_blocks(0, BLOCK_COUNT); // Colocar todo el bitmap en cero
    
    // Calcular bloque inicial del archivo que queremos modificar (el pasado por parámetro)
    int bloque_nuevo_inicial = offset;
    
    // Actualizar el bitmap y escribir los nuevos bloques del archivo a ampliar
    if (allocate_blocks(0, offset + calcular_bloques(tamanio_arch)) != UINT32_MAX){
        if(escribir_arch_bloques(buffer_general, buffer_general_size, 0)){
            if(!escribir_arch_bloques(buffer_especial, tamanio_arch, BLOCK_SIZE * bloque_nuevo_inicial)){
                free(buffer_general);
                free(buffer_especial);
                return UINT32_MAX;
            }
        }
    }
    
    free(buffer_general);
    free(buffer_especial);

    return bloque_nuevo_inicial + calcular_bloques(tamanio_arch); // antes habia un + 1
}

//-------------------------------BITMAP--------------------------------------------

// Verificar si un bloque está ocupado
bool bloque_ocupado(int nro_bloque) {
    if (nro_bloque >= 0 && nro_bloque < bitarray_get_max_bit(bitmap)) {
        return bitarray_test_bit(bitmap, nro_bloque); // Devuelve el valor del bit de la posición indicada
    }
    return false; // Fuera de los límites o no ocupado
}

uint32_t buscar_fst_bloque_libre(){ // Le sacamos el puntero porque era inutil
    for (int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        if (!bitarray_test_bit(bitmap, i)){
            return i;
        }
    }
    return UINT32_MAX; 
}

// Función auxiliar para asignar bloques
uint32_t allocate_blocks(uint32_t bloque_inicial, uint32_t cant_bloques) {
    pthread_mutex_lock(&mutex_block_count); 
    if(FREE_BLOCK_COUNT == 0) {
        pthread_mutex_unlock(&mutex_block_count);
        log_warning (logger_io_extra, "No hay bloques disponibles en el FS \n");
        return UINT32_MAX; // No hay bloques disponibles
    }
    pthread_mutex_unlock(&mutex_block_count); 

    for (int i = bloque_inicial; i < cant_bloques + bloque_inicial; i++) {
         if (status_bloque(i, SET, 0) == UINT32_MAX)
            return UINT32_MAX;
    }
    
    return 0; 
}
                                   
uint32_t deallocate_blocks(uint32_t nro_bloque, uint32_t cant_bloques) {
    for (int i = nro_bloque; i < cant_bloques + nro_bloque; i++) {
        uint32_t pos_mem = i * BLOCK_SIZE;
        if (status_bloque(i, CLEAN, pos_mem) == UINT32_MAX)
            return UINT32_MAX;
    }
    return 0;
}


// Cambiar el estado de un marco (ocupar o limpiar)
uint32_t status_bloque(uint32_t nro_bloque, bool condicion, uint32_t pos_mem) {
    if(nro_bloque == UINT32_MAX)
        return UINT32_MAX;

    pthread_mutex_lock(&mutex_bitmap); // Mutex para el bitarray
    pthread_mutex_lock(&mutex_block_count); // Mutex para la variable global
    pthread_mutex_lock(&mutex_archivo_bloques); // Mutex para bloques.dat
    if (condicion){
        bitarray_set_bit(bitmap, nro_bloque);
        FREE_BLOCK_COUNT--;
    } else {
        bitarray_clean_bit(bitmap, nro_bloque);
        memset(ptr_mmap_mem + pos_mem, 0, BLOCK_SIZE); 
        FREE_BLOCK_COUNT++;
    }
    pthread_mutex_unlock(&mutex_block_count);
    pthread_mutex_unlock(&mutex_archivo_bloques);

    if (msync(ptr_mmap_bitmap, bitmap_size, MS_SYNC) == -1){    
        pthread_mutex_unlock(&mutex_bitmap);
        return UINT32_MAX;
    } 
    pthread_mutex_unlock(&mutex_bitmap);

    pthread_mutex_lock(&mutex_archivo_bloques);
    if (msync(ptr_mmap_mem, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC) == -1){    
        pthread_mutex_unlock(&mutex_archivo_bloques);
        return UINT32_MAX;
    } 
    pthread_mutex_unlock(&mutex_archivo_bloques);

    return 0;
}

//--------------------------------FCB------------------------------------
t_fcb* inicializar_fcb(char* nombre_archivo) {
    t_fcb* fcb = malloc(sizeof(t_fcb));
    if (fcb == NULL) {
        log_error(logger_io_extra, "Error al reservar memoria para FCB\n");
        return NULL;
    }
    fcb->nombre_archivo = strdup(nombre_archivo);
    if (fcb->nombre_archivo == NULL) {
        log_error(logger_io_extra, "Error al reservar memoria para nombre_archivo\n");
        liberar_fcb(fcb);
        return NULL;
    }
    return fcb;
}


t_fcb* sacar_fcb_lista(char* nombre_archivo){
    pthread_mutex_lock(&mutex_lista_fcbs);
    for (int i = 0; i < list_size(lista_fcbs); i++){
		t_fcb* fcb_aux = list_get(lista_fcbs, i);
		if (strcmp(fcb_aux->nombre_archivo, nombre_archivo) == 0){
			list_remove(lista_fcbs, i);
            pthread_mutex_unlock(&mutex_lista_fcbs);
			return fcb_aux;
		}
	}
    pthread_mutex_unlock(&mutex_lista_fcbs);
    log_error(logger_io_extra, "No se encontró el archivo %s en la cola \n", nombre_archivo);
    return NULL;
}

t_fcb* get_fcb_lista(char* nombre_archivo){
    pthread_mutex_lock(&mutex_lista_fcbs);
    for (int i = 0; i < list_size(lista_fcbs); i++){
		t_fcb* fcb_aux = list_get(lista_fcbs, i);
		if (strcmp(fcb_aux->nombre_archivo, nombre_archivo) == 0){
            pthread_mutex_unlock(&mutex_lista_fcbs);
			return fcb_aux;
		}
	}
    pthread_mutex_unlock(&mutex_lista_fcbs);
    log_warning(logger_io_extra, "No se encontró el archivo %s en la cola \n", nombre_archivo);
    return NULL;
}

void liberar_fcb(t_fcb* fcb) {
    if (fcb) {
        free(fcb->nombre_archivo);
        config_destroy(fcb->arch_metadata);
        free(fcb);
    }
}
