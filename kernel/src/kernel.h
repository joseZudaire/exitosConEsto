#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <semaphore.h>
#include <commons/config.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include "lib.h"
#include "utils.h"

//#include "planificadores.h"

int ejecutar;

int uso = 0;

typedef char *mate_sem_name;

/*typedef enum
{
	MEMORIA = 0,
	KERNEL = 1
}codigo;*/

//Deadlock
t_list* procesosEnDeadlock;

int planificacion = 0;
int conexionMemoria;

//Colas de Estado
t_queue* processNew;
t_list* processReady;
t_list* processSuspendedBlocked;
t_queue* processSuspendedReady;
t_queue* processExit;
t_queue* colaIO;
t_list* processBlocked;
t_list* cpusExec;
t_list* dispositivosIO;
t_list* semaforosKernel;

//Semaforos
sem_t puedeEntrarAlSistema;
sem_t procesoNuevo;
sem_t cantidadCarpinchosEnReady;
sem_t planificar;
sem_t planifMedianoPlazo;
sem_t ejecutarIO;
sem_t semFinalizarProceso;
sem_t suspendidoAReady;

//MUTEX
pthread_mutex_t m_colaNew;
pthread_mutex_t m_colaReady;
pthread_mutex_t m_colablocked;
pthread_mutex_t m_colaSuspendedBlocked;
pthread_mutex_t m_colaSuspendedReady;
pthread_mutex_t m_colaExit;
pthread_mutex_t m_listaCPUs;
pthread_mutex_t m_ioQueue;
pthread_mutex_t m_semaforosKernel;

int PIDAnterior;

//*void* recibirOperaciones(int );

int desbloquearProcesoPorSemaforo(semaforo* semaforo);

void terminarProceso(int unPID);

mate_instance* asignarEspacio();

void sacarRecursos(int unPID);

void chequearDeadlock();

int comprobarDeadlock(int PID);

void reasignarProceso(int unPID, char* unSemaforo);

void reasignarSemaforo(char* unSemaforo, int PID);

semaforo* conseguirSemaforo(char* semNombre);

t_pcb* conseguirPcb(int PID);

void iterator(t_log* logger, char* value);

void deserealizar(void *magic, int socket_cliente);

mate_instance* deserializar_instancia(void *magic, int socket_cliente);

int ejecutarCarpincho(t_cpu *cpu);

//void* serializar_instancia(t_cpu *carpincho);
void* serializar_instancia(t_cpu *lib_ref);

mate_instance* deserializar_mensaje(void *magic, int desplazamiento, int socket_cliente);

void crearCPUS();

void hacerAlgo();

void planificadorLargoPlazo();

void planificadorMedianoPlazo();

t_pcb* elDeMenorSJF();

void calcularEstimacion(t_pcb*);

void planificarPorHRRN();

t_pcb* elMayorRR();

int calculoRR(t_pcb*);

void planificarPorSJFSinDesalojo();

t_cpu* buscarCPULibre();

void asignarCPU(t_pcb*);

void crearIO();

void planificadorIO();

void encolarIO(t_pcb*, char*);

void crearSemaforoKernel(char*, int);

void eliminarSemaforoKernel(char*);

void sumarSem(char*, t_pcb*);

int restarSem(char*, t_pcb*);

void finalizacionCarpincho(int unCarpincho);

#endif /* SERVER_H_ */
