#ifndef LIB_H_
#define LIB_H_

#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

// Colores para el printf
#define _RED     "\x1b[31m"
#define _GREEN   "\x1b[32m"
#define _YELLOW  "\x1b[33m"
#define _BLUE    "\x1b[34m"
#define _MAGENTA "\x1b[35m"
#define _CYAN    "\x1b[36m"
#define _RESET   "\x1b[0m"

// Booleans
#define TRUE 1
#define FALSE 0

// Estructuras Definidas
typedef struct
{
    char* ip; // Ip memoria
    char* puerto; // puerto memoria
    char* ip_swap; // Ip memoria swap
    char* puerto_swap; // puerto memoria swap
	uint32_t tamanio; // TAMANO_MEMORIA
	uint32_t tamanio_pagina; // TAMANO_PAGINA
    char* tipo_asignacion; // TIPO ASIGNACION
    char* algoritmo_reemplazo_mmu; // ALGORITMO_REEMPLAZO MMU
    int cantida_entradas_tlb; // Cantidad de entradas tlb
	char* algoritmo_reemplazo_tlb; // ALGORITMO_REEMPLAZO TLB
    int retardo_acierto_tlb; // RETARDO ACIERtO TLB
    int retardo_fallo_tlb; // RETARDO FALLO TLB
	char* log_file; // LOG_FILE
    char* path_dump_tlb;
    int debug; // DEBUG
    uint32_t marcos_por_carpincho;
	t_config* configuracion;
} t_config_memoria;

t_log* logger;
t_config_memoria* datosConfigMemoria;

t_log* iniciar_logger(int debug);

t_config_memoria* read_and_log_config();


//TESTS
/*
    void test_tlb();
    void test_LRU_GLOBAL();
    void test_MCLOCK_GLOBAL();
    void test_LRU_FIJA();
    void test_MCLOCK_FIJA();
    void test_obtener_todas_las_paginas_con_presencia_en_cache();
    void test_paginacion();
    void test_mem_virtual();
*/
#endif
