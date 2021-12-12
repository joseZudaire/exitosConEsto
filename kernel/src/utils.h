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
#include <errno.h>
#include <signal.h>

#define IP "0.0.0.0"
#define PUERTO "4444"

typedef int32_t mate_pointer;

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef enum {
	NEW = 0, READY = 1, BLOCKED = 2, EXEC = 3, SUSPENDED = 4, FINISHED = 5
} state;

typedef enum {
	PROCESS = 1
} codOperacion;

typedef enum {
	MEMALLOC = 0, MEMFREE = 1, MEMREAD = 2, MEMWRITE = 3, PROCESS_ENDED = 4
} operacion;

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
	int PID;
	int* valor;
}semaforo;

typedef struct mate_instance {
	semaforo* sema;
	pid_t *PID;
	int *socket;
	state *estado;
	mate_io_resource *nombreIO;
	mate_memory *memoria;
	type *tipo;
} mate_instance;

t_log* logger;

typedef struct {
	int* socket;
	int PID;

	//recurso asignado Semaforo (los nombrados)
	char* semaforoRequerido;
	int IO_finished;

	state status;
	char* timestamp_ready_in;
	char* timestamp_exec_in; //procesoCPU cuando entró
	char* timestamp_exec_out; //procesoCPU cuando sale
	float proximaEstimacion;
	float previaEstimacion;
	float rafagaRealCPU;
} t_pcb;

typedef struct {
	char* nombre;
	int duracion_io;
} t_io;

typedef struct {
	int PID;
	char* nombre;
} t_io_req;

typedef struct {
	int valor;
	char* nombre;
	int PID;
} t_sem_kernel;

typedef enum {
	FREE = 0, BUSY = 1
} statusCPU;

typedef struct {
	statusCPU estado;
	t_pcb* pcbCarpincho;
	sem_t* semaforoCPU;
	mate_instance* carpincho;
} t_cpu;

void* recibir_buffer(int*, int);
int iniciar_servidor(char *ip, char* puerto);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
int crear_conexion(char*, char*);
void copiarCarpincho(mate_instance* nuevoCarpincho, mate_instance* viejoCarpincho);
t_pcb* create_pcb(int , mate_instance*);


//#endif /* UTILS_H_ */
