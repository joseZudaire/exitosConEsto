#include "lib.h"

void iniciar_logger() {
	char* path = "kernel.log";
	char* nombrePrograma = "Kernel";

	logger = log_create(path , nombrePrograma, 0, LOG_LEVEL_INFO);
	log_info(logger, "El Logger del Kernel Fue Creado");

}

void read_and_log_config() {
	t_config* archivo_Config = config_create("kernel.config");

	if(archivo_Config == NULL){
		log_error(logger, "No se encuentra la direccion del Archivo de Configuracion");
		exit(1);
	}

	datosConfigKernel = malloc(sizeof(t_config_kernel));
	datosConfigKernel->puerto_escucha = config_get_string_value(archivo_Config, "PUERTO_ESCUCHA");
    datosConfigKernel->ip_memoria = config_get_string_value(archivo_Config, "IP_MEMORIA");
    datosConfigKernel->puerto_memoria = config_get_string_value(archivo_Config, "PUERTO_MEMORIA");
	datosConfigKernel->algoritmo_planificacion = config_get_string_value(archivo_Config, "ALGORITMO_PLANIFICACION");
	datosConfigKernel->alfa = atof(
			config_get_string_value(archivo_Config, "ALFA"));
	datosConfigKernel->estimacion_inicial = config_get_int_value(archivo_Config,
			"ESTIMACION_INICIAL");
	datosConfigKernel->dispositivos_io = config_get_array_value(archivo_Config,
			"DISPOSITIVOS_IO");
	datosConfigKernel->duraciones_io = config_get_array_value(archivo_Config,
			"DURACIONES_IO");
    //datosConfigKernel->retardo_cpu = config_get_int_value(archivo_Config, "RETARDO_CPU");
    datosConfigKernel->grado_multiprogramacion = config_get_int_value(archivo_Config, "GRADO_MULTIPROGRAMACION");
    datosConfigKernel->grado_multiprocesamiento = config_get_int_value(archivo_Config, "GRADO_MULTIPROCESAMIENTO");
    datosConfigKernel->debug = config_get_int_value(archivo_Config, "DEBUG");
    datosConfigKernel->tiempo_deadlock = config_get_int_value(archivo_Config, "TIEMPO_DEADLOCK");
	datosConfigKernel->configuracion = archivo_Config;

    if (datosConfigKernel->debug == TRUE) {
        printf("----------configurations---------->\n");
        printf("ip_memoria: %s\n",datosConfigKernel->ip_memoria);
        printf("puerto_memoria: %s\n",datosConfigKernel->puerto_memoria);
        printf("puerto_escucha: %s\n",datosConfigKernel->puerto_escucha);
        printf("algoritmo_planificacion: %s\n", datosConfigKernel->algoritmo_planificacion);
        printf("dispositivos_io: %s\n", datosConfigKernel->dispositivos_io);
        printf("duraciones_io: %s\n", datosConfigKernel->duraciones_io);
        //printf("retardo_cpu: %d\n", datosConfigKernel->retardo_cpu);
        printf("grado_multiprogramacion: %d\n", datosConfigKernel->grado_multiprogramacion);
        printf("grado_multiprocesamiento: %d\n", datosConfigKernel->grado_multiprocesamiento);
        printf("grado_multiprocesamiento: %d\n", datosConfigKernel->tiempo_deadlock);
        printf("----------ends configurations---------->\n");
    }

	//return datosConfigKernel;
}
