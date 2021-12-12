#ifndef CLIENTSIDE_CLIENTUTILS_H_
#define CLIENTSIDE_CLIENTUTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include "server.h"
#include "lib.h"

// Enumeradores
typedef enum {
	LEER=9,
	ESCRIBIR=8,
	CREAR=7,
	ALGO=6
} op_code_swap;

int socket_swamp;

int crear_conexion_swamp(char *, char*);
void enviar_paquete_swamp(t_buffer* buffer, int codigo_operacion);
t_buffer* serializar_escritura_swap(int pid_carpincho, int pagina, char* msg);
int enviar_escritura_swap(int pid_carpincho, int pagina, char* msg);
int enviar_tipo_asignacion();
int lectura_swamp(int pid_carpincho, int pagina, int marco);
int enviar_consulta_puede_escribir_en_swap(int socket, int carpinchoID, int cantPag);

//t_buffer* serializar_cuatro_parametros(char*, uint32_t, uint32_t, uint32_t);


#endif
