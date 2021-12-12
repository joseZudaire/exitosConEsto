#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include "matelib.h"

void* io_thread(void* mate_ref) {
	//pthread_exit si recibe 0

	printf("\nEntramos acá\n");

	mate_instance* mate_ref3 = malloc(sizeof(mate_instance));
	mate_init(mate_ref3,"matelib.config");
	//mate_pointer ptr = mate_memalloc(mate_ref, sizeof(char) * 8);
    mate_sem_wait((mate_instance*) mate_ref3, "SEM2");

    // Memory tooling usage
    //mate_pointer ptr = mate_memalloc(mate_ref3, sizeof(char) * 8);
    //mate_memwrite(mate_ref3, "PRINTER", ptr, sizeof(char) * 8);
    char *io_type = malloc(sizeof(char) * 8);
    //mate_memread(mate_ref3, ptr, io_type, sizeof(char) * 8);

    // IO Usage
    //mate_call_io(mate_ref, io_type, "I'm content to print...");

    // Freeing Memory
    // Closing Lib
    free(io_type);
    //mate_memfree(mate_ref3, ptr);

    mate_close(mate_ref3);

    return 0;
}

//matelib
int main(int argc, char *argv[])
{

	//sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

	mate_instance* mate_ref; //= malloc(sizeof(mate_instance));
	//mate_instance* mate_ref2 = malloc(sizeof(mate_instance));

	mate_init(mate_ref,"matelib.config");
	//mate_init(mate_ref2,"matelib.config2");

	mate_io_resource *io;

	/*io = malloc(sizeof(mate_io_resource));
	io->longitud = malloc(sizeof(int));
	*(io->longitud) = strlen("laguna") + 1;*/

	/*io->nombre = malloc(*(io->longitud)*sizeof(char));
	memcpy(io->nombre,"laguna",7*sizeof(char));*/

	//free(mate_memwrite(mate_ref, "hola1", 0, (strlen("hola1")+1)*sizeof(char)));

	printf("\nEstamos despues del memwrite\n");

	usleep(1000000);

	//free(mate_memwrite(mate_ref, "hola2", 0, (strlen("hola2")+1)*sizeof(char)));

	/*usleep(1000000);

	mate_sem_init(mate_ref, "sem1", 1);

	usleep(1000000);

	mate_sem_wait(mate_ref, "sem1");

	usleep(1000000);

	mate_sem_post(mate_ref, "sem1");

	usleep(1000000);

	mate_sem_destroy(mate_ref, "sem1");

	usleep(1000000);*/

	//free(mate_memalloc(mate_ref, 2));

	mate_pointer unPuntero = mate_memalloc(mate_ref, 42);

	printf("\nEl valor del puntero es: %i\n", unPuntero);

	mate_memwrite(mate_ref, "hola2", unPuntero, (strlen("hola2")+1));

	usleep(1000000);

	//free(mate_memfree(mate_ref, 0));

	usleep(1000000);

	//free(mate_memread(mate_ref, 0, "hola", (strlen("hola")+1)*sizeof(char)));

	usleep(1000000);
	void *msg = "hola que tal como va este es un mensaje";

	//pthread_t thread_id;
	//pthread_create(&thread_id, NULL, &io_thread, mate_ref);
	//pthread_detach(&thread_id);

	mate_call_io(mate_ref, (mate_io_resource) "holaaa", msg);

	printf("\nLlegamos acá despues de mandar io\n");

	//DESPUES DESCOMENTAR, POR ALGUNA RAZON ME TIRA SEGMENTATION FAULT
	/*free(io->nombre);
	free(io->longitud);
	free(io);*/

	printf("\nLlegamos acá despues de mandar io2\n");

	//mate_close(mate_ref2);

	mate_close(mate_ref);

	/*sem_t *sem_instance2 = malloc(sizeof(sem_t));
	if((sem_instance2 = sem_open("sem36", O_CREAT, 0, 2)) ==SEM_FAILED) {
		printf("\nerror2 inicializacion semaforo\n");
	}*/
	/*int *restrict valor = malloc(sizeof(int));
	printf("\nValor del semaforo: %i\n", sem_getvalue(sem_instance2, valor));
	sem_getvalue(sem_instance2, valor);
	printf("Semaforo valor: %i", *valor);*/

	/*printf("\n%i", sem_close(sem_instance2));
	sem_close(sem_instance2);
	printf("\n%i", sem_unlink("sem16"));
	sem_unlink("sem16");*/

	//mate_sem_init(mate_ref,"sem38",12);

	//free(valor);

	//printf("%s", sem_name(mate_ref->sem_instance));


	//mate_memread(mate_ref, 0, "hola", 2);

	/*
  // Lib instantiation
  mate_instance mate_ref;
  mate_init(&mate_ref, "./config.json");

  // Let's work with semaphores
  mate_sem_init(&mate_ref, "SEM1", 0);
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, &io_thread, &mate_ref);

  usleep(1000);
  printf("This message should go first...\n");
  mate_sem_post(&mate_ref, "SEM1");

  pthread_join(thread_id, NULL);

  mate_sem_destroy(&mate_ref, "SEM1");

  mate_close(&mate_ref);*/
  return 0;
}
