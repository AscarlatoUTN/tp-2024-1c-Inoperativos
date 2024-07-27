#include "../include/traduccion.h" 

int tlb_size;
static uint32_t posicion = 0; // no se si la tengo que definir aca
static uint32_t contador_global;
tlb_entry* tlb;

//------------------------------MMU-------------------------------------
uint32_t traducir_direccion(uint32_t PID, int num_pag, int desplazamiento) {
    t_tabla* tabla = malloc(sizeof(t_tabla));
    if (tabla == NULL) {
        log_error(logger_cpu_extra, "Error al asignar memoria para la tabla \n");
        return UINT32_MAX;
    }

    tabla->marco_pag = num_pag;
    tabla->PID = PID;
    
    uint32_t frame = UINT32_MAX;
    if(CANTIDAD_ENTRADAS_TLB > 0)
        frame = buscar_tlb(PID, num_pag);

    if (frame == UINT32_MAX) { // TLB MISS
        log_info(logger_cpu, "PID: %d - TLB MISS - Pagina: %d \n", PID, num_pag);
        enviar_pedido_marco(OBTENER_MARCO, conexion_memoria, tabla);
        recv(conexion_memoria, &frame, sizeof(uint32_t), MSG_WAITALL);
        
        if (frame == UINT32_MAX) {
            log_error(logger_cpu_extra, "No se encontró el proceso o la página \n");
            free(tabla); 
            return UINT32_MAX;
        }

        log_info(logger_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d \n", PID, num_pag, frame);
       
        if(CANTIDAD_ENTRADAS_TLB > 0)
            actualizar_tlb(PID, num_pag, frame);

    }else { //TLB HIT
        log_info(logger_cpu, "PID: %d - TLB HIT - Pagina: %d \n", PID, num_pag);
        log_info(logger_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d \n", PID, num_pag, frame);
    }
    
    free(tabla); 
    return frame * tam_pagina + desplazamiento; //DF
}

//-------------------------------- TLB ---------------------------------------------

void init_tlb(){
	tlb_size = CANTIDAD_ENTRADAS_TLB;
	if(tlb_size == 0) {
		log_warning(logger_cpu_extra, "TLB deshabilitada \n");
		return;
	}

	tlb = (tlb_entry*) malloc(sizeof(tlb_entry) * tlb_size);
    if (tlb == NULL) {
        log_error(logger_cpu_extra, "Error al asignar memoria para la TLB \n");
        return;
    }

    for (int i = 0; i < tlb_size; i++) {
        tlb[i].PID = 0;
        tlb[i].last_used = 0;
    }

    if(strcmp(ALGORITMO_TLB, "LRU") == 0)
        contador_global = 0;
}

//FIFO
void reemplazar_pag_fifo(uint32_t PID, int pagina, uint32_t marco) {
    tlb[posicion].PID = PID;
    tlb[posicion].pag = pagina;
    tlb[posicion].marco = marco;
    posicion = (posicion + 1) % tlb_size;
}

//LRU

void reemplazar_pag_lru(uint32_t PID, int pagina, uint32_t marco) {
    int lru_index = 0;
    
    for (int i = 1; i < tlb_size; i++) {
        if (tlb[i].PID == 0) {
            lru_index = i;
            break;
        }
        else if (tlb[i].last_used < tlb[lru_index].last_used) {
            lru_index = i;
        }
    }
    tlb[lru_index].PID = PID;
    tlb[lru_index].pag = pagina;
    tlb[lru_index].marco = marco;
    tlb[lru_index].last_used = contador_global++;
}

uint32_t buscar_tlb(uint32_t PID, int pagina) {
    for (int i = 0; i < tlb_size; i++) {
        if (tlb[i].PID == PID && tlb[i].pag == pagina) {
            if(strcmp(ALGORITMO_TLB, "LRU") == 0)
                tlb[i].last_used = contador_global++;
            // log_warning(logger_cpu, "TLB hit \n");
            return tlb[i].marco;
        }
    }
    // log_warning(logger_cpu, "TLB miss \n");
    return UINT32_MAX; 
}

void actualizar_tlb(uint32_t PID, int pagina, uint32_t marco) {
    if (strcmp(ALGORITMO_TLB, "FIFO") == 0) {
        reemplazar_pag_fifo(PID, pagina, marco);
    } else if (strcmp(ALGORITMO_TLB, "LRU") == 0){
        reemplazar_pag_lru(PID, pagina, marco);
    } else {
        log_error(logger_cpu_extra, "Algoritmo de reemplazo de TLB no válido \n");
        exit(EXIT_FAILURE);
    }
}