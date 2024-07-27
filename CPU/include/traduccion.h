#ifndef TRADUCCION_H
#define TRADUCCION_H

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include "../../SHARED/include/send.h"
#include "../../SHARED/include/receive.h"
#include "../../SHARED/include/memoria.h"
#include "../../SHARED/include/serv_sockets.h"
#include "../../SHARED/include/structs.h"
#include "../../MEMORIA/include/memoria_real.h"

#include "init.h"

uint32_t traducir_direccion(uint32_t, int, int);
void init_tlb();
void reemplazar_pag_fifo(uint32_t, int, uint32_t);
void reemplazar_pag_lru(uint32_t, int, uint32_t);
uint32_t buscar_tlb(uint32_t, int);
void actualizar_tlb(uint32_t, int, uint32_t);


#endif