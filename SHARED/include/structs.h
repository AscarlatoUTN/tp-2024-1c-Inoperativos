#ifndef STRUCTS_H
#define STRUCTS_H

#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

typedef enum{
	// CPU --> MEMORIA
	SGTE_INSTRUCC,
	FRAME_TLB_MISS,
    ACCEDER_TABLA_PAGINAS,
    AJUSTAR_TAMANIO_PROCESO,
    OBTENER_MARCO,
    LECTURA,
    ESCRITURA,
    
	// KERNEL --> CPU
	CTXT_EXEC,

	// KERNEL --> MEMORIA
	CREAR_PROCESO,
	ELIMINAR_PROCESO,

	// IO --> KERNEL
	UNIDS_TRABAJO,
	DIRECC_MEM,
	INTERFAZ,
	INIT,

	// IO --> MEMORIA
	IO_STDIN_READ,
    IO_STDOUT_WRITE,
	IO_FS_CREATE,
    IO_FS_READ,
	IO_FS_WRITE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,

	// Motivos de Desalojo
	M_FIN_QUANTUM,
    M_WAIT,
    M_SIGNAL,
    M_IO_GEN,
    M_IO_STDIN,
    M_IO_STDOUT,
    M_IO_FS_CREATE,
    M_IO_FS_TRUNCATE,
    M_IO_FS_DELETE,
    M_IO_FS_READ,
    M_IO_FS_WRITE,
    M_EXIT,
    M_OOM,
    M_PROC_INTERRUPT,
    
	// Tipos de Interfaz
    T_IO_GEN,
    T_IO_STDIN,
    T_IO_STDOUT,
    T_IO_DIALFS,

    // Conexion dispatch-io
    CONFIRMACION,
    CONFIRMACION_KERNEL,
    CONFIRMACION_MEM,

    //Interrupcion
    INTR,
    FIN_Q,

    //Acceso memoria
    IO_READ,
    IO_WRITE
} header;

typedef enum{
    HANDSHAKE_IO_KERNEL,
    HANDSHAKE_KERNEL_CPU,
    HANDSHAKE_CPU_MEMORIA,
    HANDSHAKE_KERNEL_MEMORIA,
    HANDSHAKE_IO_STDIN_MEMORIA,
    HANDSHAKE_IO_STDOUT_MEMORIA,
    HANDSHAKE_IO_DIALFS_MEMORIA,
    HANDSHAKE_IO_GEN_KERNEL,
    HANDSHAKE_IO_STDIN_KERNEL,
    HANDSHAKE_IO_STDOUT_KERNEL,
    HANDSHAKE_IO_DIALFS_KERNEL
} handshake;

// -------------------------------- Estructuras de Paquetes --------------------------
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	header codigo_operacion;
	t_buffer* buffer;
} t_paquete;


typedef struct
{
    uint32_t PID;
    uint32_t PC;
} t_pedir_instruccion;

// -------------------------------- Estructuras de Procesos --------------------------

typedef enum{
    NEW,
    READY,
    EXEC,
    BLOCK,
    EXIT
} ESTADOS;


typedef struct {
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
} t_registros_gral;

typedef struct t_pcb t_pcb;

struct t_pcb {
    uint32_t PID;               // ID del Proceso
    uint32_t q_restante;        // Unidad de tiempo utilizada por VRR
    uint32_t PC;                // Program Counter, numero de la proxima instruccion a ejecutar
    uint32_t SI;                // Registro SI
    uint32_t DI;                // Registro DI
    ESTADOS p_status;           // Estado del Proceso (New/Ready/Exec/Block/Exit)  
    t_registros_gral* reg;      // Struct que contiene los registros generales de la C
};

typedef struct
{
    t_pcb* pcb;
    uint32_t buffer_size;
} pcb_motivo;

// -------------------------------- Estructuras de Manejo de Recursos --------------------------
typedef struct {
    char* nombre;
    uint16_t instancias;
} recurso_t;

typedef struct {
    uint32_t PID;
    t_list* recs_asoc;
} pid_rec;

// -------------------------------- Estructuras de CPU --------------------------
typedef struct {
    t_pcb* PCB;
    uint32_t length_nombre_interfaz;
    char* nombre_interfaz;
    uint32_t unidades_trabajo;
} interfaz_a_enviar_gen;

typedef struct {
    t_pcb* PCB;
    uint32_t length_nombre_interfaz;
    char* nombre_interfaz;
    uint32_t reg_tamanio;
    uint32_t list_df_size;
    uint32_t offset;
    t_list* list_df;
} interfaz_a_enviar_std;

typedef struct {
    t_pcb* PCB;
    uint32_t length_nombre_interfaz;
    char* nombre_interfaz;
    uint32_t length_nombre_archivo;
    char* nombre_archivo;
    uint32_t reg_tamanio;
} interfaz_a_enviar_fs;

typedef struct {
    uint32_t length_nombre_archivo;
    char* nombre_archivo;
    uint32_t reg_tamanio;
    uint32_t PID;
} parametros_fs;

typedef struct { // CPU --> KERNEL
    t_pcb* PCB;
    uint32_t length_nombre_interfaz;
    char* nombre_interfaz;
    uint32_t length_nombre_archivo;
    char* nombre_archivo;
    t_list* lista_dfs;
    uint32_t lista_dfs_size;
    uint32_t reg_tamanio;
    uint32_t ptr_arch;
    uint32_t offset;
} parametros_fs_rw;

typedef struct { // KERNEL --> IO
    uint32_t length_nombre_archivo;
    char* nombre_archivo;
    uint32_t PID;
    t_list* lista_dfs;
    uint32_t lista_dfs_size;
    uint32_t reg_tamanio;
    uint32_t ptr_arch;
    uint32_t offset;
} params_inst_fs;

typedef struct{
    t_pcb* PCB;
    uint32_t length_nombre_recurso;
    char* nombre_recurso;
} enviar_pcb_rec;

typedef struct{
    uint32_t PID;
    uint32_t pag;
    uint32_t marco;
	uint32_t last_used;
} tlb_entry;

// -------------------------------- Estructuras de I/O --------------------------

typedef enum {
    SLEEPING,
    WORKING,
    READING,
    NOT_READING,
    PRINTING,
    NOT_PRINTING
} ESTADO;

typedef struct {
	header tipo_interfaz;
    uint32_t length_nombre_interfaz;
	char* nombre_interfaz;
    uint32_t tiempo_unidad_trabajo;
	int fd_interfaz;
    ESTADO estado;
} t_io_gen;

typedef struct {
	header tipo_interfaz;
    uint32_t length_nombre_interfaz;
	char* nombre_interfaz;
	uint32_t fd_io_kernel;
    uint32_t fd_io_mem;
    ESTADO estado;
} t_io_std;

typedef struct {
    header tipo_interfaz;
    uint32_t length_nombre_interfaz;
	char* nombre_interfaz;
    uint32_t length_path;
    char* path;
    uint32_t tiempo_unidad_trabajo;
    uint32_t retraso_compactacion;
    uint32_t fd_io_kernel;
    uint32_t fd_io_mem;
    int block_size;
    int block_count;
    ESTADO estado;
} t_io_df;

typedef struct {
    char* nombre_archivo;
    int bloque_base;
    int tamanio; // En Bytes
    t_config* arch_metadata;
} t_fcb;

typedef struct {
    t_list* list_df_io;
    uint32_t list_df_io_size;
    uint32_t reg_tamanio;   
    uint32_t PID; 
    uint32_t offset;
} df_size;

//Estructuras de hilos
typedef struct {
    int kernel_socket;
    char* io_serv_name;
    t_io_gen* interfaz;
} t_hilo_io_gen_args;

typedef struct {
    int kernel_socket;
    int mem_socket;
    char* io_serv_kernel;
    char* io_serv_mem;
    t_io_std* interfaz;
} t_hilo_io_std_args;

// -------------------------------- Estructuras de Kernel --------------------------

typedef struct {
    uint32_t PID;
    uint32_t unidades_trabajo;
}io_gen_pid;

typedef struct {
    char* nombre_estado;
    t_list* lista_procesos;
} estado_procesos_t;

// -------------------------------- Estructuras de Memoria --------------------------
typedef struct {
   uint32_t PID;
   char* path;
   uint32_t length_path;
} proceso_memoria;

typedef struct {
    char* ip;
    char* puerto;
    int conexion;
    t_log* logger;
    t_config* config;
} t_conexion_args;

// Struct instruccion + PID
typedef struct {
    uint32_t pid;
    t_list* instrucciones;
    t_list* pageTable;
} t_proceso;

typedef struct {
    uint32_t PID;
    uint32_t direc_pag;
    uint32_t offset_size;
} t_tabla_r;

typedef struct {
    uint32_t PID;
    uint32_t df;
    uint32_t length_dato;
    void* dato;
    uint32_t offset;
}t_tabla_w;

typedef struct {
    uint32_t PID;
    uint32_t marco_pag;
}t_tabla;

typedef struct {
    uint32_t PID;
    uint32_t tam_resize;
    uint32_t cant_pags;
}t_resize;


// -------------------------------- Funciones -------------------------------------

void signal_bin (sem_t*);
void flag_interrupcion(pthread_mutex_t*, int32_t, int*);
void liberar_df_size(df_size*);
void liberar_params_inst_fs (params_inst_fs*);
void liberar_pcb(t_pcb*);
char* estado_pcb (ESTADOS);
void liberar_tabla_w(t_tabla_w*);

#endif