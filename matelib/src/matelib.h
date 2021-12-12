#ifndef MATELIB_H_INCLUDED
#define MATELIB_H_INCLUDED

#include <stdint.h>
#include "utils.h"
#include <semaphore.h>
#include <commons/config.h>
#include <errno.h>

//-------------------Type Definitions----------------------/

t_log* mate_logger;

typedef char *mate_sem_name;

typedef int32_t mate_pointer;

/*typedef enum
{
	MEMORIA = 0,
	KERNEL = 1
}codigo;*/

typedef enum
{
	NEW = 0,
	READY = 1,
	BLOCKED = 2
}state;

typedef enum
{
	PROCESS = 1
}codOperacion;

typedef enum
{
	MEMALLOC = 0,
	MEMFREE = 1,
	MEMREAD = 2,
	MEMWRITE = 3
}operacion;

typedef struct mate_io_resourc1
{
	void *nombre;
	int *longitud;
} mate_io_resourc1;

typedef char *mate_io_resource;

typedef struct mate_memory
{
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

typedef struct carpincho
{
    semaforo* sema;
	pid_t *PID;
    state *estado;
    mate_io_resourc1 *nombreIO;
    mate_memory *memoria;
    type *tipo;
	char* ip;
	char* puerto;
	int* socketConexion;
} carpincho;

typedef struct mate_instance
{
	void *group_info;
} mate_instance;

// TODO: Docstrings

void asignar_puerto_e_IP(char *unaConfig, mate_instance *lib_ref);

//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config);

int mate_close(mate_instance *lib_ref);

//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value);

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem);

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg);

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size);

int mate_memfree(mate_instance *lib_ref, mate_pointer addr);

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size);

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size);

void* serializar_instancia(mate_instance *lib_ref);

//void serializar_mensaje(mate_instance *lib_ref, char* mensaje);

t_log* mate_iniciar_logger(void);

#endif
