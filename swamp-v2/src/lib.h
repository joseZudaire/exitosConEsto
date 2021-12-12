/*
 * lib.h
 *
 *  Created on: 24 sep. 2021
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

// Colores para el printf
#define _RED     "\x1b[31m"
#define _GREEN   "\x1b[32m"
#define _YELLOW  "\x1b[33m"
#define _BLUE    "\x1b[34m"
#define _MAGENTA "\x1b[35m"
#define _CYAN    "\x1b[36m"
#define _RESET   "\x1b[0m"

#define TRUE 1
#define FALSE 0

// Estructuras Definidas
typedef struct {
    char* ip; // Ip
    char* puerto; // puerto
    uint32_t tamanio_swamp;
    int tamanio_pagina;
    char** archivos_swamp;
    char* cant_archivos_swamp;
    int marcos_maximos;
    int retardo_swamp;
    int debug; // DEBUG
	t_config* configuracion;
} t_config_swamp;

t_log* logger;
t_config* config;
t_config_swamp* datosConfigSwamp;

// Funciones de Log y Config
void iniciar_logger();
t_config_swamp* read_and_log_config();
