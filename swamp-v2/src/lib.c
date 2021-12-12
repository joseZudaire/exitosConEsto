#include "lib.h"

void iniciar_logger() {
	char* path = "swamp.log";
	char* nombrePrograma = "Swamp";

	logger = log_create(path , nombrePrograma, 0, LOG_LEVEL_INFO);
	log_info(logger, "El Logger del Swamp Fue Creado");

}

t_config_swamp* read_and_log_config() {
	t_config* archivo_Config = config_create("swamp.config");

	if(archivo_Config == NULL){
		log_error(logger, "No se encuentra la direccion del Archivo de Configuracion");
		exit(1);
	}
	datosConfigSwamp = malloc(sizeof(t_config_swamp));
    datosConfigSwamp->ip = config_get_string_value(archivo_Config, "IP");
    datosConfigSwamp->puerto = config_get_string_value(archivo_Config, "PUERTO");
	datosConfigSwamp->tamanio_swamp = config_get_int_value(archivo_Config, "TAMANIO_SWAP");
    datosConfigSwamp->tamanio_pagina = config_get_int_value(archivo_Config, "TAMANIO_PAGINA");
    datosConfigSwamp->archivos_swamp = config_get_array_value(archivo_Config, "ARCHIVOS_SWAP");
    datosConfigSwamp->cant_archivos_swamp = config_get_string_value(archivo_Config, "ARCHIVOS_SWAP");
    datosConfigSwamp->marcos_maximos = config_get_int_value(archivo_Config, "MARCOS_MAXIMOS");
    datosConfigSwamp->retardo_swamp = config_get_int_value(archivo_Config, "RETARDO_SWAP");
    datosConfigSwamp->debug = config_get_int_value(archivo_Config, "DEBUG");
	datosConfigSwamp->configuracion = archivo_Config;

    if (datosConfigSwamp->debug == TRUE) {
        printf("----------configurations---------->\n");
        printf("ip: %s\n",datosConfigSwamp->ip);
        printf("puerto: %s\n",datosConfigSwamp->puerto);
        printf("tamanio_swamp: %d\n",datosConfigSwamp->tamanio_swamp);
        printf("tamanio_pagina: %d\n",datosConfigSwamp->tamanio_pagina);
        printf("archivos_swamp: %s\n",datosConfigSwamp->archivos_swamp);
        printf("marcos_maximos: %d\n",datosConfigSwamp->marcos_maximos);
        printf("retardo_swamp: %d\n",datosConfigSwamp->retardo_swamp);
        printf("----------ends configurations---------->\n");
    }
	return datosConfigSwamp;
}
