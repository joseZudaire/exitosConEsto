//#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <string.h>


#define IP "0.0.0.0"
#define PUERTO "4444"

typedef int32_t mate_pointer;

typedef enum {
	NEW = 0, READY = 1, BLOCKED = 2, RUNNING = 3, SUSPENDED = 4, FINISHED = 5
} state;

typedef enum {
	MESSAGE = 0, PROCESS = 1
} codOperacion;

typedef enum {
	MEMALLOC = 0, MEMFREE = 1, MEMREAD = 2, MEMWRITE = 3
} operacion;

typedef struct mate_io_resource {
	void *nombre;
	int *longitud;
} mate_io_resource;

typedef struct mate_memory {
	int *size;
	mate_pointer *addr;
	operacion *tipoOp;
} mate_memory;

typedef struct mate_inner_structure {
	//void *memory;
	//int *size;
	sem_t *sem_instance;
} mate_inner_structure;

typedef struct mate_instance {
	mate_inner_structure *group_info;
	pid_t *PID;
	int *socket;
	state *estado;
	mate_io_resource *nombreIO;
	mate_memory *memoria;
} mate_instance;

t_log* logger;

typedef struct {
	int* socket;
	int PID;
	state status;
} t_pcb;

void* recibir_buffer(int*, int);
//int iniciar_servidor();
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
t_pcb* create_pcb(int);




//#endif /* UTILS_H_ */
