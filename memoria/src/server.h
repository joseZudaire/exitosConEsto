#ifndef SERVER_H_
#define SERVER_H_
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "lib.h"


typedef int32_t mate_pointer;

typedef enum
{
	MENSAJES2 = 0,
	PAQUETE = 1
}op_code;

typedef enum
{
	PUEDE_ESCRIBIR = 1,
	NO_PUEDE_ESCRIBIR = -1
}swamp_puede_escribir_resultado;

typedef enum {
	NEW = 0, READY = 1, BLOCKED = 2, EXEC = 3, SUSPENDED = 4, FINISHED = 5
} state;

typedef enum {
	MEMALLOC = 0, MEMFREE = 1, MEMREAD = 2, MEMWRITE = 3
} operacion;

typedef enum {
	PROCESS = 1
} codOperacion;

typedef struct mate_io_resource {
	void *nombre;
	int *longitud;
} mate_io_resource;

typedef struct mate_memory {
	int *size;
	mate_pointer *addr;
	operacion *tipoOp;
	void *valor;
} mate_memory;

typedef enum

{
	MEMORIA = 0,
	IO = 1,
	SEMAFORO = 2,
	INICIALIZACION = 3,
	FINALIZACION = 4
}type;

typedef enum
{
	INIT = 0,
	WAIT = 1,
	POST = 2,
	DESTROY = 3
}operation;

typedef struct semaforo
{
	operation* operacion;
	char* nombre_sem;
	int* longitud;
	int* valor;
}semaforo;

typedef enum {
	MATE_FREE_FAULT=-5,
	MATE_READ_FAULT=-6,
	MATE_WRITE_FAULT=-7,
	MATE_WRITE_OK = 7,
	MATE_READ_OK = 6,
	MATE_FREE_OK = 5,
	MATE_ALLOC_OK = 4,
	MATE_ALLOC_FAULT = -4,
	MATE_END_OK = 9,
	MATE_END_FAULT = -9
} err_code;

typedef struct{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct{
	uint8_t codigo_operacion;
	t_buffer* buffer;
} t_paquete;


typedef struct mate_instance {
	semaforo* sema;
	pid_t *PID;
	int *socket;
	state *estado;
	mate_io_resource *nombreIO;
	mate_memory *memoria;
	type *tipo;
} mate_instance;

// Funciones
void iniciar_servidor();
void server_aceptar_clientes(int);
void server_procesar_mensaje(uint8_t, int);
int server_bind_listen(char*, char*);
mate_instance* deserializar_instancia(void *magic, int socket_cliente);
void* recibirOperaciones(int cliente_fd);
int esperar_cliente(int socket_servidor);
int recibir_operacion(int cliente_fd);

int recibir_cod_op(int socket);
void deserializar_mensaje(t_paquete* paquete, int socket);

#endif
