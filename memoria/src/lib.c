#include "lib.h"

t_log* iniciar_logger(int debug)
{
	char* path = "memoria.log";
	char* nombrePrograma = "Memoria";

	if (debug == TRUE)
		 logger = log_create(path , nombrePrograma, 1, LOG_LEVEL_DEBUG);
	 else
		 logger = log_create(path , nombrePrograma, 1, LOG_LEVEL_INFO);

	log_info(logger, "El Logger de memoria fue creado");

	return logger;
}

t_config_memoria* read_and_log_config()
{
	t_config* archivo_Config = config_create("./memoria.config");

    datosConfigMemoria = malloc(sizeof(t_config_memoria));
	datosConfigMemoria->ip = config_get_string_value(archivo_Config, "IP");
	datosConfigMemoria->puerto = config_get_string_value(archivo_Config, "PUERTO");
    datosConfigMemoria->ip_swap = config_get_string_value(archivo_Config, "IP_SWAP");
	datosConfigMemoria->puerto_swap = config_get_string_value(archivo_Config, "PUERTO_SWAP");
    datosConfigMemoria->tipo_asignacion = config_get_string_value(archivo_Config, "TIPO_ASIGNACION");
    datosConfigMemoria->tamanio = config_get_int_value(archivo_Config, "TAMANIO");
    datosConfigMemoria->tamanio_pagina = config_get_int_value(archivo_Config, "TAMANIO_PAGINA");
    datosConfigMemoria->algoritmo_reemplazo_mmu = config_get_string_value(archivo_Config, "ALGORITMO_REEMPLAZO_MMU");
    datosConfigMemoria->cantida_entradas_tlb = config_get_int_value(archivo_Config, "CANTIDAD_ENTRADAS_TLB");
    datosConfigMemoria->algoritmo_reemplazo_tlb = config_get_string_value(archivo_Config, "ALGORITMO_REEMPLAZO_TLB");
    datosConfigMemoria->retardo_acierto_tlb = config_get_int_value(archivo_Config, "RETARDO_ACIERTO_TLB");
    datosConfigMemoria->retardo_fallo_tlb = config_get_int_value(archivo_Config, "RETARDO_FALLO_TLB");
    datosConfigMemoria->path_dump_tlb = config_get_string_value(archivo_Config, "PATH_DUMP_TLB");
    datosConfigMemoria->debug = config_get_int_value(archivo_Config, "DEBUG");
    datosConfigMemoria->marcos_por_carpincho = config_get_int_value(archivo_Config, "MARCOS_POR_CARPINCHO");
	datosConfigMemoria->configuracion = archivo_Config;

    if (datosConfigMemoria->debug == TRUE)
    {
        datosConfigMemoria->path_dump_tlb = config_get_string_value(archivo_Config, "PATH_DUMP_TLB_DEBUG");
        printf("----------Configuration---------->\n");
        printf("ip: %s\n",datosConfigMemoria->ip);
        printf("puerto: %s\n",datosConfigMemoria->puerto);
        printf("ip_swap: %s\n",datosConfigMemoria->ip_swap);
        printf("puerto_swap: %s\n",datosConfigMemoria->puerto_swap);
        printf("tamanio: %d\n",datosConfigMemoria->tamanio);
        printf("tamanio pag: %d\n",datosConfigMemoria->tamanio_pagina);
        printf("tipo_asignacion: %s\n",datosConfigMemoria->tipo_asignacion);
        printf("algoritmo_reemplazo_mmu: %s\n",datosConfigMemoria->algoritmo_reemplazo_mmu);
        printf("cantida_entradas_tlb: %d\n",datosConfigMemoria->cantida_entradas_tlb);
        printf("algoritmo_reemplazo_tlb: %s\n",datosConfigMemoria->algoritmo_reemplazo_tlb);
        printf("retardo_acierto_tlb: %d\n",datosConfigMemoria->retardo_acierto_tlb);
        printf("retardo_fallo_tlb: %d\n",datosConfigMemoria->retardo_fallo_tlb);
        printf("path dump tlb: %s\n",datosConfigMemoria->path_dump_tlb);
        printf("----------end configuration---------->\n");
    }

	return datosConfigMemoria;
}
