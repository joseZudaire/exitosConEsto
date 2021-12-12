#include "kernel.h"

/*int main(void) {
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor();
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	/*t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			//list_iterate(lista, (void*) iterator);
			for (int i=0; i<list_size(lista);i++)
				iterator(logger,list_get(lista,i));
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,
					"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;*/
	/*printf("to do bien");
	return 0;

}*/

int main() {
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	read_and_log_config();

	conexionMemoria = crear_conexion(datosConfigKernel->ip_memoria, datosConfigKernel->puerto_memoria);
	if(conexionMemoria == -1) {
		log_info(logger, "Conexión fallida con Memoria");
	}

	//Creo Listas y Colas
	processNew = queue_create();
	processReady = list_create();
	processSuspendedBlocked = list_create();
	processSuspendedReady = queue_create();
	processBlocked = list_create();
	processExit = queue_create();
	cpusExec = list_create();
	dispositivosIO = list_create();
	colaIO = queue_create();
	semaforosKernel = list_create();

	pthread_t hiloPMP;
	pthread_t hiloPLP;
	pthread_t hiloPCP;
	pthread_t hiloPIO;

	//Creo semaforos
	sem_init(&puedeEntrarAlSistema, 0,
			datosConfigKernel->grado_multiprogramacion);

	sem_init(&procesoNuevo, 0, 0);
	sem_init(&cantidadCarpinchosEnReady, 0, 0);
	sem_init(&planificar, 0, 1);
	sem_init(&planifMedianoPlazo, 0, 0);
	sem_init(&ejecutarIO, 0, 0);
	sem_init(&semFinalizarProceso, 0, 0);
	sem_init(&suspendidoAReady, 0, 0);

	//Creo MUTEX
	pthread_mutex_init(&m_colaNew, NULL);
	pthread_mutex_init(&m_colaReady, NULL);
	pthread_mutex_init(&m_colablocked, NULL);
	pthread_mutex_init(&m_colaSuspendedBlocked, NULL);
	pthread_mutex_init(&m_colaSuspendedReady, NULL);
	pthread_mutex_init(&m_colaExit, NULL);
	pthread_mutex_init(&m_listaCPUs, NULL);
	pthread_mutex_init(&m_ioQueue, NULL);
	pthread_mutex_init(&m_semaforosKernel, NULL);

	//Crear Hilos
	pthread_create(&hiloPLP, NULL, (void*) &planificadorLargoPlazo, NULL);
	pthread_detach(hiloPLP);

	pthread_create(&hiloPMP, NULL, (void*) &planificadorMedianoPlazo, NULL);
	pthread_detach(hiloPMP);

	if (strcmp(datosConfigKernel->algoritmo_planificacion, "HRRN") == 0) {
		pthread_create(&hiloPCP, NULL, (void*) &planificarPorHRRN, NULL);
		pthread_detach(hiloPCP);
	} else if (strcmp(datosConfigKernel->algoritmo_planificacion, "SJF") == 0) {
		pthread_create(&hiloPCP, NULL, (void*) &planificarPorSJFSinDesalojo,
				NULL);
		pthread_detach(hiloPCP);
	}

	pthread_create(&hiloPIO, NULL, (void*) &planificadorIO, NULL);
	pthread_detach(hiloPIO);


	//CREO CPUS
	crearCPUS(cpusExec);
	int server_fd = iniciar_servidor(datosConfigKernel->ip_memoria,datosConfigKernel->puerto_escucha);
	log_info(logger, "Servidor listo para recibir al cliente");
	crearIO();
	void* algo;
	int tamanio;
	int cliente_fd = 0;

	pthread_t hiloLlegada;
	pthread_t hiloDeadlock;

	pthread_create(&hiloDeadlock, NULL, (void*) &chequearDeadlock, NULL);
	pthread_detach(hiloDeadlock);

	while (1) {
		cliente_fd = esperar_cliente(server_fd);

		//Encolar en lista de carpinchos y despues cuando se asigna a una CPU se queda escuchado
		//el send del mate_init se hace recien cuando se asigna el carpincho a la CPU
		//idem para llamada de IO, porque es bloqueante

		//printf("\nLlega aca\n");

		//printf("\nEntramos una vez\n");
		int cod_op;
		void* buffer;

		if ((cliente_fd != 0) && (cliente_fd != -1)) {
			cod_op = recibir_operacion(cliente_fd);

			if(cod_op == 1) {

				mate_instance* carpincho = deserializar_instancia(buffer,cliente_fd);
				log_info(logger, "PID %i: Llegó un carpincho", *(carpincho->PID));
				planificacion = 0;
				t_pcb* clientPCB = create_pcb(cliente_fd, carpincho);
				pthread_mutex_lock(&m_colaNew);
				queue_push(processNew, clientPCB);
				pthread_mutex_unlock(&m_colaNew);
				sem_post(&procesoNuevo);
			}


		}
	}

	return 0;
}

void chequearDeadlock() {

	log_info(logger, "-------------------------------------------------------------");
	log_info(logger, "-------------------------------------------------------------");
	log_info(logger, "Servidor ejecutando algoritmo de detección y recuperación");
	log_info(logger, "-------------------------------------------------------------");
	log_info(logger, "-------------------------------------------------------------");
	log_info(logger, "-------------------------------------------------------------");

	//usleep(10000000);

	while(1) {
		usleep(datosConfigKernel->tiempo_deadlock*1000);

		if(processBlocked->elements_count != 0) {
			procesosEnDeadlock = list_create();

			for(int i = 0; i < (processBlocked->elements_count-1); i++) {
				t_pcb* unPcb = list_get(processBlocked, i);

				int resultado = comprobarDeadlock(unPcb->PID);

				if(resultado != 0) {
					terminarProceso(resultado);
				}
			}

			list_destroy(procesosEnDeadlock);
		}
	}
}

void terminarProceso(int unPID) {
	for(int i = 0; i < (processBlocked->elements_count-1); i++){
		t_pcb* unPcb = list_get(processBlocked, i);

		if(unPcb->PID == unPID) {
			sacarRecursos(unPID);
			free(unPcb);
			pthread_mutex_lock(&m_colablocked);
			list_remove(processBlocked, i);
			pthread_mutex_unlock(&m_colablocked);
		}
	}
}

void sacarRecursos(int unPID){
	for(int i = 0; i < semaforosKernel->elements_count; i++){

		pthread_mutex_lock(&m_semaforosKernel);
		semaforo* unSem = list_get(semaforosKernel, i);
		pthread_mutex_unlock(&m_semaforosKernel);

		if(unSem->PID == unPID) {
			unSem->PID = -1;
			pthread_mutex_lock(&m_semaforosKernel);
			list_replace(semaforosKernel, unSem, i);
			pthread_mutex_unlock(&m_semaforosKernel);
		}
	}

}

void pasarDeBloqueadoAReady(int unPID){
	for(int i = 0; i < processBlocked->elements_count; i++){
		t_pcb* unPcb = list_get(processBlocked, i);

		if(unPcb->PID == unPID) {
			pthread_mutex_lock(&m_colablocked);
			list_remove(processBlocked, i);
			pthread_mutex_unlock(&m_colablocked);

			pthread_mutex_lock(&m_colaReady);
			list_add(processReady, (void*) unPcb);
			pthread_mutex_unlock(&m_colaReady);
		}
	}

}

int comprobarDeadlock(int PID){

	printf("\nEl PID es: %i\n", PID);
	t_pcb* unPcb = conseguirPcb(PID);

	//usleep(1000000);

	if(unPcb != NULL) {

		printf("\n1\n");
		semaforo* unSemaforo = conseguirSemaforo(unPcb->semaforoRequerido);
		printf("\n2\n");

		if(unSemaforo != NULL) {

			printf("\n3\n");
			printf("\nPCB PID: %i\n", unPcb->PID);
			printf("\nSemaforo PID: %i\n", unSemaforo->PID);
			if(unPcb->PID == unSemaforo->PID) {
				printf("\n4\n");
				//esta bien asignado, por lo que no deberia estar bloqueado
				//reasignar el semaforo anterior

				if(procesosEnDeadlock->elements_count != 0){
					printf("\n5\n");
					t_pcb* pcbAnterior = list_get(procesosEnDeadlock, (procesosEnDeadlock->elements_count-1));
					reasignarSemaforo(pcbAnterior->semaforoRequerido, -1);
				}

				pasarDeBloqueadoAReady(unPcb->PID);
				//DEBERÍA PASARSE A READY

				return 0;
			}

			bool loRequiere(int PID){
				printf("\n6\n");
				return unSemaforo->PID == PID;
			}

			if(list_any_satisfy(procesosEnDeadlock, (void*) loRequiere)) {
				printf("\n7\n");

				//hay un deadlock, porque estamos en un ciclo
				for(int i = 0; i < procesosEnDeadlock->elements_count; i++) {
					printf("\n8\n");
					int iterador = list_get(procesosEnDeadlock,i);

					if(iterador == unSemaforo->PID) {
						printf("\n9\n");
						list_take_and_remove(procesosEnDeadlock,i);

						bool esMenor(void* unPID, void* otroPID){
							printf("\n10\n");
							return (int) unPID > (int) otroPID;
						}

						t_list* ordenados = list_create();
						ordenados = list_sorted(procesosEnDeadlock, esMenor);

						int PIDaFinalizar = list_get(ordenados, 0);
						list_destroy(ordenados);

						return PIDaFinalizar;
						//finalizar proceso con PIDaFinalizar

					}
				}
			}

			list_add(procesosEnDeadlock, unPcb->PID);

			PIDAnterior = unPcb->PID;
			comprobarDeadlock(unSemaforo->PID);
			//conseguirSemDePID(unPcb->PID);

		}
		else{
			printf("\n11\n");
			//no hay semaforo con ese nombre
			//reasignar semaforo de proceso a null
			//y reasignar semaforo anterior a proceso anterior

			reasignarProceso(unPcb->PID, NULL);

			return 0;
		}
	}
	else {
		printf("\n12\n");

		/*for (int i = 0; i < procesosEnDeadlock->elements_count; i++) {
			//printf("\nEl PID\n", procesosEnDeadlock->)
		}*/
		//no hay procesos con ese PID
		//encontrar semaforo anterior a ese

		if((procesosEnDeadlock->elements_count != 0)&&(procesosEnDeadlock->elements_count != 1)){
			printf("\n13\n");
			printf("\nLa cantidad de elementos en procesos en deadlock es: %i\n", procesosEnDeadlock->elements_count);
			//t_pcb* otroPcb = list_get(procesosEnDeadlock, (procesosEnDeadlock->elements_count-1));
			int indice = (procesosEnDeadlock->elements_count) - 1;
			printf("\nIndice: %i\n", indice);
			fflush(stdout);
			t_pcb* otroPcb = list_get(procesosEnDeadlock, indice);
			printf("\nOtroPcb->PID: %i\n", otroPcb->PID);

			reasignarSemaforo(otroPcb->semaforoRequerido, otroPcb->PID);
		}

		//usleep(1000000000);

		return 0;
	}

	/*for(int i = 0; i < processBlocked->elements_count; i++) {
		t_pcb unPcb = list_get(processBlocked, i);

		if(semNombre != NULL) {
			if(strcmp(unPcb->semaforoRequerido,semNombre) == 0){




				if(unPcb->semaforoRequerido->PID == unPcb->PID) {
					//está to do bien
					return 0;
				}

				bool loRequiere(int PID){
					return unPcb->semaforoRequerido->PID == PID;
				}

				if(list_any_satisfy(procesosEnDeadlock, (void*) loRequiere)) {

					//hay un deadlock
					for(int i = 0; i < procesosEnDeadlock->elements_count; i++) {
						int iterador = list_get(procesosEnDeadlock,i);

						if(iterador == unPcb->semaforoRequerido->PID) {
							list_take_and_remove(procesosEnDeadlock,i-1);

							bool esMenor(void* unPID, void* otroPID){
								return (int) unPID > (int) otroPID;
							}

							t_list* ordenados = list_create();
							ordenados = list_sorted(procesosEnDeadlock, esMenor);

							int PIDaFinalizar = list_get(ordenados, 0);
							list_destroy(ordenados);

							return PIDaFinalizar;

							//finalizar proceso con PIDaFinalizar

						}
					}
				}

				//meter por las dudas si un proceso tiene un semaforo y viceversa

				if(unPcb->semaforoRequerido->PID == -1) {
					//reasignar semaforo


					return 0;
				}

				list_add(procesosEnCola, unPcb->PID);

				//se hace un return acá?
				return conseguirSemDePID(unPcb->PID);
			}

			if(i == (processBlocked->elements_count-1)) {
				return 0;
			}
		}
		else{
			return 0;
		}
	}*/
	return -1;
}

void reasignarProceso(int unPID, char* unSemaforo) {
	for(int i = 0; i < processBlocked->elements_count; i++){

		pthread_mutex_lock(&m_colablocked);
		t_pcb* unPcb = list_get(processBlocked, i);
		pthread_mutex_unlock(&m_colablocked);

		if(unPcb->PID == unPID) {
			if(unSemaforo != NULL) {
				memcpy(unPcb->semaforoRequerido,unSemaforo,strlen(unSemaforo)+1);
				pthread_mutex_lock(&m_colablocked);
				list_replace(processBlocked, unPcb, i);
				pthread_mutex_unlock(&m_colablocked);
			}
			else {
				unPcb->semaforoRequerido = NULL;
				pthread_mutex_lock(&m_colablocked);
				list_replace(processBlocked, unPcb, i);
				pthread_mutex_unlock(&m_colablocked);
			}

		}
	}
}

void reasignarSemaforo(char* unSemaforo, int PID) {

	for(int i = 0; i < semaforosKernel->elements_count; i++){

		pthread_mutex_lock(&m_semaforosKernel);
		semaforo* unSem = list_get(semaforosKernel, i);
		pthread_mutex_unlock(&m_semaforosKernel);

		if(unSem->nombre_sem == unSemaforo) {
			unSem->PID = PID;
			pthread_mutex_lock(&m_semaforosKernel);
			list_replace(semaforosKernel, unSem, i);
			pthread_mutex_unlock(&m_semaforosKernel);
		}
	}
}

semaforo* conseguirSemaforo(char* semNombre){
	for(int i = 0; i < (semaforosKernel->elements_count-1); i++){
		pthread_mutex_lock(&m_semaforosKernel);
		semaforo* unSem = list_get(semaforosKernel, i);
		pthread_mutex_unlock(&m_semaforosKernel);

		if(strcmp(unSem->nombre_sem,semNombre) == 0) {
			return unSem;
		}
	}
	return NULL;
}

t_pcb* conseguirPcb(int PID){
	for(int i = 0; i < (processBlocked->elements_count-1); i++){

		pthread_mutex_lock(&m_colablocked);
		t_pcb* unPcb = list_get(processBlocked, i);
		pthread_mutex_unlock(&m_colablocked);

		if(unPcb->PID == PID) {
			return unPcb;
		}
	}
	return NULL;
}

/*void verUnSemaforoQueEstaAsignadoAEsePID(int PID_Proceso){
	for(int i = 0; i < semaforos->elements_count; i++) {
		semaforo* unSem = list_get(semaforos, i);

		if(unSem->PID == PID_Proceso){
			conseguirPIDdeSem(unSem->nombre_sem);
		}
		if(i == (semaforos->elements_count-1)){
			//reasignar proceso
			conseguirPIDdeSem(NULL);
		}
	}
}*/

/*int chequearLoop(char* semNombre, int PID){
	while(1) {

		for(int i = 0; i < semaforos->elements_count; i++) {
			semaforo* unSem = list_get(semaforos, i);

			if((strcmp(unSem->nombre_sem, semNombre) == 0) && (unSem->PID!=-1) && (contadorDeadlock <= processBlocked->elements_count)){
				chequearLoop(unSem->nombre_sem, PID);
			}
		}

	}
}*/


/*t_cpu* asignarTCPU(mate_instance* carpincho, int socket) {

	t_cpu* cpuCarpincho = malloc(sizeof(t_cpu));
	cpuCarpincho->estado = FREE;
	copiarCarpincho(cpuCarpincho->carpincho, carpincho);

	cpuCarpincho->pcbCarpincho = malloc(sizeof(t_pcb));
	cpuCarpincho->pcbCarpincho->PID = *(carpincho->PID);
	cpuCarpincho->pcbCarpincho->status = malloc(sizeof(state));
	*(cpuCarpincho->pcbCarpincho->status) = NEW;
	cpuCarpincho->pcbCarpincho->socket = malloc(sizeof(int));
	memcpy(cpuCarpincho->pcbCarpincho->socket, carpincho->socket, sizeof(int));

	//crear el semaforo

	if(*(cpuCarpincho->carpincho->tipo) == INICIALIZACION) {
		printf("\nIniciado un carpincho\n");
		int resultado = 0;
		send(socket, &resultado, sizeof(int), 0);

		return NULL;
	}

	return cpuCarpincho;
}
*/


void copiarCarpincho(mate_instance* nuevoCarpincho, mate_instance* viejoCarpincho) {

	nuevoCarpincho = malloc(sizeof(mate_instance));
	nuevoCarpincho->PID = malloc(sizeof(pid_t));
	memcpy(nuevoCarpincho->PID, viejoCarpincho->PID, sizeof(int));

	nuevoCarpincho->tipo = malloc(sizeof(type));
	memcpy(nuevoCarpincho->tipo, viejoCarpincho->tipo, sizeof(type));

	nuevoCarpincho->sema = malloc(sizeof(semaforo));
	nuevoCarpincho->sema->longitud = malloc(sizeof(int));
	memcpy(nuevoCarpincho->sema->longitud, viejoCarpincho->sema->longitud, sizeof(int));

	nuevoCarpincho->sema->nombre_sem = malloc(*(nuevoCarpincho->sema->longitud));
	memcpy(nuevoCarpincho->sema->nombre_sem, viejoCarpincho->sema->nombre_sem, *(nuevoCarpincho->sema->longitud));

	nuevoCarpincho->sema->operacion = malloc(sizeof(operation));
	memcpy(nuevoCarpincho->sema->operacion, viejoCarpincho->sema->operacion, sizeof(operation));

	nuevoCarpincho->estado = malloc(sizeof(state));
	memcpy(nuevoCarpincho->estado, viejoCarpincho->estado, sizeof(state));

	nuevoCarpincho->memoria = malloc(sizeof(mate_memory));
	nuevoCarpincho->memoria->size = malloc(sizeof(int));
	memcpy(nuevoCarpincho->memoria->size, viejoCarpincho->memoria->size, sizeof(int));

	nuevoCarpincho->memoria->addr = malloc(sizeof(mate_pointer));
	memcpy(nuevoCarpincho->memoria->addr, viejoCarpincho->memoria->addr, sizeof(mate_pointer));

	nuevoCarpincho->memoria->tipoOp = malloc(sizeof(operacion));
	memcpy(nuevoCarpincho->memoria->tipoOp, viejoCarpincho->memoria->tipoOp, sizeof(operacion));

	nuevoCarpincho->memoria->valor = malloc(*(nuevoCarpincho->memoria->size));
	memcpy(nuevoCarpincho->memoria->valor, viejoCarpincho->memoria->valor, *(nuevoCarpincho->memoria->size));

	nuevoCarpincho->nombreIO = malloc(sizeof(mate_io_resource));
	nuevoCarpincho->nombreIO->longitud = malloc(sizeof(int));
	memcpy(nuevoCarpincho->nombreIO->longitud, viejoCarpincho->nombreIO->longitud, sizeof(int));

	nuevoCarpincho->nombreIO->nombre = malloc(*(nuevoCarpincho->nombreIO->longitud));
	memcpy(nuevoCarpincho->nombreIO->nombre, viejoCarpincho->nombreIO->nombre, *(nuevoCarpincho->nombreIO->longitud));

}

mate_instance* deserializar_instancia(void *magic, int socket_cliente) {

	//printf("\nEntramos a deserealizar instancia\n");

	mate_instance* nuevaInstancia = malloc(sizeof(mate_instance));

	nuevaInstancia->PID = malloc(sizeof(pid_t));
	nuevaInstancia->tipo = malloc(sizeof(type));
	nuevaInstancia->sema = malloc(sizeof(semaforo));
	nuevaInstancia->sema->longitud = malloc(sizeof(int));
	nuevaInstancia->sema->operacion = malloc(sizeof(operation));
	nuevaInstancia->sema->valor = malloc(sizeof(int));
	nuevaInstancia->estado = malloc(sizeof(state));
	nuevaInstancia->memoria = malloc(sizeof(mate_memory));
	nuevaInstancia->memoria->size = malloc(sizeof(int));
	nuevaInstancia->memoria->addr = malloc(sizeof(mate_pointer));
	nuevaInstancia->memoria->tipoOp = malloc(sizeof(operacion));
	nuevaInstancia->nombreIO = malloc(sizeof(mate_io_resource));
	nuevaInstancia->nombreIO->longitud = malloc(sizeof(int));
	nuevaInstancia->socket = malloc(sizeof(int));

	*(nuevaInstancia->socket) = socket_cliente;

	recv(socket_cliente, nuevaInstancia->tipo, sizeof(type), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->sema->operacion, sizeof(operation), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->sema->longitud, sizeof(int), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->sema->valor, sizeof(int), MSG_WAITALL);

	nuevaInstancia->sema->nombre_sem = malloc(*(nuevaInstancia->sema->longitud));

	recv(socket_cliente, nuevaInstancia->sema->nombre_sem, *(nuevaInstancia->sema->longitud), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->PID, sizeof(pid_t), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->estado, sizeof(state), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->memoria->size, sizeof(int), MSG_WAITALL);

	nuevaInstancia->memoria->valor = malloc(*(nuevaInstancia->memoria->size));

	recv(socket_cliente, nuevaInstancia->memoria->valor, *(nuevaInstancia->memoria->size), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->memoria->addr, sizeof(mate_pointer), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->memoria->tipoOp, sizeof(operacion), MSG_WAITALL);
	recv(socket_cliente, nuevaInstancia->nombreIO->longitud, sizeof(int), MSG_WAITALL);

	nuevaInstancia->nombreIO->nombre = malloc(*(nuevaInstancia->nombreIO->longitud));

	recv(socket_cliente, nuevaInstancia->nombreIO->nombre, *(nuevaInstancia->nombreIO->longitud), MSG_WAITALL);

	printf("\nEl nombre de IO es: %s\n", nuevaInstancia->nombreIO->nombre);
	//printf("\nEl nombre de IO es: %s\n", nuevaInstancia->nombreIO->nombre);

	//printf("\nTerminamos de deserealizar instancia\n");

	if(*(nuevaInstancia->tipo) == INICIALIZACION){
		int desplazamiento = sizeof(int);
		//printf("\nEntramos aca\n");
		int resultado = 0;
		send(socket_cliente, &resultado, desplazamiento, 0);

	}


	/*void* algo1 = malloc(sizeof(int));
	int algo12 = 0;

	memcpy(algo1, &algo12, sizeof(int));

	usleep(1000000);

	printf("\nEl resultado de mandar el retorno fue: %i\n", send(socket_cliente, algo1, sizeof(int), 0));*/

	/*
	 * PARA PROBAR CÓMO LLEGAN LOS PAQUETES
	 *
	 */

	/*log_info(logger, "Tipo: %i", *(nuevaInstancia->tipo));
	log_info(logger, "Largo sem: %i", *(nuevaInstancia->sema->longitud));
	log_info(logger, "PID: %i", *(nuevaInstancia->PID));
	log_info(logger, "Estado: %i", *(nuevaInstancia->estado));
	log_info(logger, "Memoria Size: %i", *(nuevaInstancia->memoria->size));
	log_info(logger, "Memoria Valor: %s", nuevaInstancia->memoria->valor);
	log_info(logger, "Tipo Operacion: %i", *(nuevaInstancia->memoria->tipoOp));
	log_info(logger, "Largo: %i", *(nuevaInstancia->nombreIO->longitud));
	log_info(logger, "Nombre: %s", nuevaInstancia->nombreIO->nombre);
	log_info(logger, "Largo nuevamente: %i", strlen(nuevaInstancia->nombreIO->nombre));
	log_info(logger, "Estado: %i", *(nuevaInstancia->estado));
	log_info(logger, "PID: %i", *(nuevaInstancia->PID));
	 log_info(logger, "Tipo: %i", *(nuevaInstancia->tipo));*/


	/*
	 * POR SI NECESITAN USAR ALGO DE SEMÁFOROS
	 *
	 *nuevaInstancia->sema->sem_instance = malloc(sizeof(sem_t));
	log_info(logger, "%s", nuevaInstancia->sema->nombre_sem);
	if((nuevaInstancia->sema->sem_instance = sem_open(nuevaInstancia->sema->nombre_sem, O_RDWR)) ==SEM_FAILED) {
		printf("\Error en la inicialización del semáforo\n");
	}
	nuevaInstancia->sema->sem_instance = sem_open(nuevaInstancia->sema->nombre_sem,0,0,12);
	int *restrict semaforo = malloc(sizeof(int));
	printf("\n%i\n",sem_getvalue(nuevaInstancia->sema->sem_instance, semaforo));
	log_info(logger, "%i", *semaforo);*/

	return nuevaInstancia;
}

void operacionesSemaforo(t_cpu* cpu) {
	semaforo* unSemaforo = malloc(sizeof(semaforo));
	unSemaforo->longitud = malloc(sizeof(int));
	unSemaforo->operacion = malloc(sizeof(operation));
	unSemaforo->valor = malloc(sizeof(int));

	*(unSemaforo->longitud) = *(cpu->carpincho->sema->longitud);
	unSemaforo->nombre_sem = malloc(sizeof(char)*(*(unSemaforo->longitud)+1));
	memcpy(unSemaforo->nombre_sem, cpu->carpincho->sema->nombre_sem, (*(unSemaforo->longitud) + 1));
	*(unSemaforo->valor) = *(cpu->carpincho->sema->valor);
	int resultado;
	cpu->pcbCarpincho->semaforoRequerido = malloc(
			(strlen(cpu->carpincho->sema->nombre_sem) + 1) * sizeof(char));
	memcpy(cpu->pcbCarpincho->semaforoRequerido,
			cpu->carpincho->sema->nombre_sem,
			strlen(cpu->carpincho->sema->nombre_sem) + 1);
	switch (*(cpu->carpincho->sema->operacion)) {
	case INIT:
		crearSemaforoKernel(cpu->carpincho->sema->nombre_sem,
				*(cpu->carpincho->sema->valor));
		break;
	case WAIT:
		resultado = restarSem(cpu->carpincho->sema->nombre_sem,
				cpu->pcbCarpincho);
		if (resultado == -1) {
			cpu->pcbCarpincho->status = BLOCKED;
			cpu->pcbCarpincho->timestamp_exec_out = temporal_get_string_time(
					"%MS");
			cpu->pcbCarpincho->previaEstimacion =
					cpu->pcbCarpincho->proximaEstimacion;
			cpu->pcbCarpincho->rafagaRealCPU = (atof(
					cpu->pcbCarpincho->timestamp_exec_out)
					- atof(cpu->pcbCarpincho->timestamp_exec_in)) / 1000;
			pthread_mutex_lock(&m_colablocked);
			list_add(processBlocked, cpu->pcbCarpincho);
			pthread_mutex_unlock(&m_colablocked);
			ejecutar = 1;
		}
		break;
	case POST:
		sumarSem(cpu->carpincho->sema->nombre_sem, cpu->pcbCarpincho);
		desbloquearProcesoPorSemaforo(cpu->carpincho->sema);
		break;
	case DESTROY:
		eliminarSemaforoKernel(cpu->carpincho->sema->nombre_sem);
		break;
	default:
		log_error(logger, "Codigo invalido de operacion de semaforo");
		break;
	}

	int desplazamiento = sizeof(int);
	int valorAMandar = 0;
	send(*(cpu->carpincho->socket), &valorAMandar, desplazamiento, 0);
	//printf("\nDespues del send del semaforo\n");

}

int restarSem(char* unNombreSem, t_pcb* pcb) {
	int position = verificarExisteSem(unNombreSem);
	if (position != -1) {
		t_sem_kernel* semaforo = list_get(semaforosKernel, position);
		if (semaforo->valor >= 1) {
			pthread_mutex_lock(&m_semaforosKernel);
			semaforo->valor --;
			pthread_mutex_unlock(&m_semaforosKernel);
			log_info(logger, "PID %i: Realizado sem_wait al semaforo %s",
					pcb->PID, unNombreSem);
			return 0;
		} else {
			log_error(logger,
					"PID %i: No se puede realizar sem_wait al semaforo %s",
					pcb->PID, unNombreSem);
			return -1;
		}
	} else {
		log_error(logger, "El semaforo %s no existe", unNombreSem);
		return -2;
	}
}



void sumarSem(char* unNombreSem, t_pcb* pcb) {
	int position = verificarExisteSem(unNombreSem);
	if (position != -1) {
		t_sem_kernel* unSem = list_get(semaforosKernel, position);
		pthread_mutex_lock(&m_semaforosKernel);
		unSem->valor++;
		pthread_mutex_unlock(&m_semaforosKernel);
		log_info(logger, "PID %i: Realizado sem_post al semaforo %s", pcb->PID,
				unNombreSem);
	} else {
		log_error(logger, "PID %i: El semaforo %s no existe", pcb->PID,
				unNombreSem);
	}
}

int verificarExisteSem(char* nombreSem){
	//printf("\nEntramos para verificar\n");
	for(int i = 0; i < semaforosKernel->elements_count; i++){
		semaforo* unSem = list_get(semaforosKernel, i);
		if(strcmp(unSem->nombre_sem,nombreSem) == 0) {
			return i;
		}
	}
	return -1;
}

//int ejecutarCarpincho(mate_instance* carpincho, int cliente_fd){//t_cpu *cpu) {
int ejecutarCarpincho(t_cpu *cpu) {
	log_info(logger, "CPU Creada");

	ejecutar = 0;

	while (1) {
		sem_wait(cpu->semaforoCPU);
		log_info(logger, "Iniciando Ejecucion PID %d", cpu->pcbCarpincho->PID);
		planificacion = 0;

		//quedar escuchando
		while (cpu->pcbCarpincho->status == EXEC) {
			//while((*(cpu->carpincho->estado) != BLOCKED)&&(*(cpu->carpincho->estado) != SUSPENDED)){
			//agarrar socket cpu->unCarpincho->socket y mandas OK
			// deserializar carpincho a partir de cpu->unCarpincho->socket

			//printf("\nTamanio de la cola de READY: %i\n", list_size(processReady));

			if(cpu->pcbCarpincho->IO_finished){
				//printf("\nEntramos al if para mandar resultado IO\n");
				int desplazamiento = sizeof(int);
				int resultado = 0;
				send(cpu->pcbCarpincho->socket, &resultado, desplazamiento, 0);
				cpu->pcbCarpincho->IO_finished = 0;
			}

			//mandar el OK al init, semaforo o call i/o
			int resultado = 0;
			int desplazamiento = sizeof(int);

			//printf("\nLlegamos acá\n");

			uso = 1;
			int cod_op = recibir_operacion(cpu->pcbCarpincho->socket);
			uso = 0;
			//printf("\nRecibimos un cod operacion\n");

			void* magico;
			cpu->carpincho = malloc(sizeof(mate_instance));
			cpu->carpincho = deserializar_instancia(magico, cpu->pcbCarpincho->socket);
			//mate_instance* unaInstancia = deserializar_instancia(magico, cpu->pcbCarpincho->socket);

			//printf("\nValor del tipo: %i\n", *(cpu->carpincho->tipo));
			//printf("\nValor del PID: %i\n", *(cpu->carpincho->PID));

			//printf("\nDespués de deseralizar\n");

			int32_t* valor2 = malloc(sizeof(int32_t));

			switch (*(cpu->carpincho->tipo)) {
			case MEMORIA:
				log_info(logger, "PID %i: Me llegó una operación de Memoria.", *(cpu->carpincho->PID));

				//DESCOMENTAR DESPUES
				void* buffer_memoria = serializar_instancia(cpu);

				int tamanio;

				/*
				* HAY EJEMPLOS EN LAS OPERACIONES PARA QUE NO ME ROMPA MATELIB
				* BORRAR DESPUÉS
				*/

				/*
				int valor = 0;
				void* buffer_memoria;

				switch(*(cpu->carpincho->memoria->tipoOp)) {
				case MEMALLOC:
					send(cpu->pcbCarpincho->socket, &valor, sizeof(mate_pointer), 0);

					//Si no se prueba con Memoria, comentar


					//send(socket, buffer_memoria, sizeof(mate_pointer), 0);
					break;
				case MEMFREE:
					send(cpu->pcbCarpincho->socket, &valor, sizeof(int), 0);
					//send(socket, buffer_memoria, sizeof(mate_pointer), 0);
					break;
				case MEMWRITE:
					//printf("\nEntramos al Memwrite\n");
					 *valor2 = 0;
					send(cpu->pcbCarpincho->socket, valor2, sizeof(mate_pointer), 0);
					//printf("\nDespues del send\n");
					//send(socket, buffer_memoria, sizeof(mate_pointer), 0);
					break;
				case MEMREAD:
					//memcpy(&tamanio, buffer_memoria, sizeof(int));

					//----------------ejemplo---------------
					buffer_memoria = malloc(((strlen("hola que tal como va este es jose") + 1)*sizeof(char)) + sizeof(int));
					int tamanioTotal = strlen("hola que tal como va este es jose") + 1;
					int tamanioTotal2 = tamanioTotal + sizeof(int);
					memcpy(buffer_memoria, &tamanioTotal, sizeof(int));
					char* mensaje = "hola que tal como va este es jose";
					memcpy(buffer_memoria + sizeof(int), mensaje, tamanioTotal);

					//----------------ejemplo---------------
					//send(socket, buffer_memoria, tamanio+sizeof(int), 0);

					send(cpu->pcbCarpincho->socket, buffer_memoria, tamanioTotal2, 0);

					break;
				default:
					log_info(logger, "PID %i: Tipo de operación de memoria no reconocido", *(cpu->carpincho->PID));
				}*/
				break;
			case IO:
					log_info(logger, "PID %i: Me llegó una operación de IO.", *(cpu->carpincho->PID));
					cpu->pcbCarpincho->status = BLOCKED;
					cpu->pcbCarpincho->timestamp_exec_out = temporal_get_string_time("%MS");
					cpu->pcbCarpincho->previaEstimacion =
							cpu->pcbCarpincho->proximaEstimacion;
					cpu->pcbCarpincho->rafagaRealCPU = (atof(
							cpu->pcbCarpincho->timestamp_exec_out)
							- atof(cpu->pcbCarpincho->timestamp_exec_in)) / 1000;
				encolarIO(cpu->pcbCarpincho, cpu->carpincho->nombreIO->nombre);
					pthread_mutex_lock(&m_colablocked);
					list_add(processBlocked, cpu->pcbCarpincho);
					pthread_mutex_unlock(&m_colablocked);

					cpu->pcbCarpincho->IO_finished = 1;

					//también por alguna razón no me está entrando al if que tengo más abajo
					ejecutar = 1;
					//send(*(cpu->pcbCarpincho->socket), &resultado, sizeof(int), 0);
					break;
			case SEMAFORO:
				log_info(logger, "PID %i: Me llegó una operación de Semáforo.", *(cpu->carpincho->PID));
				operacionesSemaforo(cpu);
				//send(*(cpu->pcbCarpincho->socket), &resultado, sizeof(int), 0);
				break;

			case FINALIZACION:
				//printf("\nFinalizando el carpincho PID %i\n", *(cpu->carpincho->PID));
				resultado = 0;
				send(cpu->pcbCarpincho->socket, &resultado, sizeof(int), 0);
				close(cpu->pcbCarpincho->socket);
				cpu->pcbCarpincho->status = FINISHED;
				pthread_mutex_lock(&m_colaExit);
				queue_push(processExit, cpu->pcbCarpincho);
				pthread_mutex_unlock(&m_colaExit);
				sem_post(&semFinalizarProceso);
				//TODO: no habria que mandar msj a memoria
				//TODO: sacar de la cola
				break;
			default:
				log_info(logger, "PID %i: La operación es inválida", *(cpu->carpincho->PID));
				resultado = -1;
				send(*(cpu->pcbCarpincho->socket), &resultado, sizeof(int), 0);
				close(*(cpu->pcbCarpincho->socket));
				cpu->pcbCarpincho->status = FINISHED;
				pthread_mutex_lock(&m_colaExit);
				queue_push(processExit, cpu->pcbCarpincho);
				pthread_mutex_unlock(&m_colaExit);
				sem_post(&semFinalizarProceso);
				break;
			}

			/*if((*(cpu->carpincho->estado) == BLOCKED)||(*(cpu->carpincho->estado) == SUSPENDED)){

			 }*/



			/*cpu->estado = FREE;
			cpu->pcbCarpincho = NULL;
			sem_post(&planifMedianoPlazo);
			log_info(logger, "CPU en estado FREE");*/

		}
		sem_post(&planifMedianoPlazo);
		sem_post(&planificar);
		planificacion = 0;
		cpu->estado = FREE;
		//cpu->pcbCarpincho = NULL;
		log_info(logger, "CPU en estado FREE");
		//free(cpu->carpincho);

	}
}

void* serializar_instancia(t_cpu *lib_ref) {

	//TODO: esto habría que corregirlo.
				//lib_ref->puerto);

	int tamanio = *(lib_ref->carpincho->nombreIO->longitud) + *(lib_ref->carpincho->memoria->size) +
			sizeof(pid_t) + 6*sizeof(int) + sizeof(state) + sizeof(mate_pointer) + sizeof(operacion) + sizeof(codOperacion) +
			sizeof(type) + sizeof(operation) + *(lib_ref->carpincho->sema->longitud);

	codOperacion tipoMensaje = PROCESS;

	void * magic = malloc(tamanio);
	int desplazamiento = 0;

	memcpy(magic+desplazamiento,&tipoMensaje,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->carpincho->tipo,sizeof(type));
	desplazamiento+= sizeof(type);
	memcpy(magic+desplazamiento,lib_ref->carpincho->sema->operacion,sizeof(operation));
	desplazamiento+= sizeof(operation);
	memcpy(magic+desplazamiento,lib_ref->carpincho->sema->longitud,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->carpincho->sema->valor,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->carpincho->sema->nombre_sem,*(lib_ref->carpincho->sema->longitud));
	desplazamiento+= *(lib_ref->carpincho->sema->longitud);

	memcpy(magic+desplazamiento,lib_ref->carpincho->PID,sizeof(pid_t));
	desplazamiento+= sizeof(pid_t);
	memcpy(magic+desplazamiento,lib_ref->carpincho->estado,sizeof(state));
	desplazamiento+= sizeof(int);

	memcpy(magic+desplazamiento,lib_ref->carpincho->memoria->size,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->carpincho->memoria->valor,*(lib_ref->carpincho->memoria->size));
	desplazamiento+= *(lib_ref->carpincho->memoria->size);
	memcpy(magic+desplazamiento,lib_ref->carpincho->memoria->addr,sizeof(mate_pointer));
	desplazamiento+= sizeof(mate_pointer);
	memcpy(magic+desplazamiento,lib_ref->carpincho->memoria->tipoOp,sizeof(operacion));
	desplazamiento+= sizeof(operacion);

	memcpy(magic+desplazamiento,lib_ref->carpincho->nombreIO->longitud,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->carpincho->nombreIO->nombre,*(lib_ref->carpincho->nombreIO->longitud));
	desplazamiento+= *(lib_ref->carpincho->nombreIO->longitud);

	log_info(logger, "PID %i: Se mandó un paquete", *(lib_ref->carpincho->PID));

	send(conexionMemoria, magic, desplazamiento, 0);

	void* result;
	int unTamanio;

	//if(*(lib_ref->carpincho->estado))

	switch (*(lib_ref->carpincho->memoria->tipoOp)) {
	case MEMALLOC:
		result = malloc(sizeof(mate_pointer));
		recv(conexionMemoria, result, sizeof(mate_pointer), MSG_WAITALL);
		int unNumero;
		memcpy(&unNumero, result, sizeof(int));
		printf("\nEl puntero de MEMALLOC es: %i\n", unNumero);
		send(lib_ref->pcbCarpincho->socket, result, sizeof(mate_pointer), 0);
		break;
	case MEMFREE:
		result = malloc(sizeof(int));
		recv(conexionMemoria, result, sizeof(int), MSG_WAITALL);
		send(lib_ref->pcbCarpincho->socket, result, sizeof(int), 0);
		break;
	case MEMREAD:
		printf("\nAntes del primer recv\n");
		recv(conexionMemoria, &unTamanio, sizeof(int), MSG_WAITALL);
		result = malloc(unTamanio+sizeof(int));
		memcpy(result, &unTamanio, sizeof(int));
		int desplaza = sizeof(int);
		printf("\nAntes del segundo recv\n");
		recv(conexionMemoria, result+desplaza, unTamanio, MSG_WAITALL);
		printf("\nAntes del send\n");
		send(lib_ref->pcbCarpincho->socket, result, unTamanio+sizeof(int), 0);
		printf("\nDespues del send\n");
		printf("El string: %s", result);
		printf("La longitud es: %i", strlen(result));
		break;
	case MEMWRITE:
		result = malloc(sizeof(mate_pointer));
		recv(conexionMemoria, result, sizeof(mate_pointer), MSG_WAITALL);
		send(lib_ref->pcbCarpincho->socket, result, sizeof(mate_pointer), 0);
		break;
	default:
		result = malloc(sizeof(int));
		recv(conexionMemoria, result, sizeof(int), MSG_WAITALL);
		send(lib_ref->pcbCarpincho->socket, result, sizeof(mate_pointer), 0);
		break;
	}

	free(magic);

	return result;
}

mate_instance* deserializar_mensaje(void *magic, int desplazamiento, int socket_cliente) {
	char *mensaje;

	int largoMsj = malloc(sizeof(int));

	recv(socket_cliente, &largoMsj, sizeof(int), MSG_WAITALL);

	mensaje = malloc(largoMsj);

	recv(socket_cliente, mensaje, largoMsj, MSG_WAITALL);

	return mensaje;

}

void iterator(t_log* logger, char* value) {
	log_info(logger,"%s\n", value);
}


void finalizarProceso() {
	t_pcb* auxPcb;
	log_info(logger, "Iniciado el hilo para finalizar procesos");
	while (1) {
		sem_wait(&semFinalizarProceso);
		if (queue_size(processExit) > 0) {
			pthread_mutex_lock(&m_colaExit);
			auxPcb = queue_pop(processExit);
			pthread_mutex_unlock(&m_colaExit);

			sacarRecursos(auxPcb->PID);

			finalizacionCarpincho(auxPcb->PID);
			//*(unCarpincho->PID) = auxPcb->PID;
			//serializar_instancia(unCarpincho);

			log_info(logger, "Finalizando proceso PID: %i", auxPcb->PID);
			free(auxPcb);
			sem_post(&puedeEntrarAlSistema);
		}
	}
}

void finalizacionCarpincho(int unPID){

	mate_instance* lib_ref = malloc(sizeof(mate_instance));

	lib_ref->socket = malloc(sizeof(int));
	//*(lib_ref->socket) = conexion;

	lib_ref->tipo = malloc(sizeof(type));
	*(lib_ref->tipo) = MEMORIA;

	lib_ref->estado = malloc(sizeof(state));
	*(lib_ref->estado) = FINISHED;

	lib_ref->PID = malloc(sizeof(pid_t));
	lib_ref->PID = unPID;

	lib_ref->sema = malloc(sizeof(semaforo));
	lib_ref->sema->operacion = malloc(sizeof(operacion));
	lib_ref->sema->nombre_sem = malloc(1);
	lib_ref->sema->longitud = malloc(sizeof(int));
	*(lib_ref->sema->longitud) = 1;
	lib_ref->sema->valor = malloc(sizeof(int));

	lib_ref->nombreIO = malloc(sizeof(mate_io_resource));
	lib_ref->nombreIO->nombre = malloc(1);

	lib_ref->nombreIO->longitud = malloc(sizeof(int));
	*(lib_ref->nombreIO->longitud) = 1;

	lib_ref->memoria = malloc(sizeof(mate_memory));
	lib_ref->memoria->size = malloc(sizeof(int));
	*(lib_ref->memoria->size) = 1;
	lib_ref->memoria->addr = malloc(sizeof(mate_pointer));
	lib_ref->memoria->tipoOp = malloc(sizeof(operacion));
	lib_ref->memoria->valor = malloc(1);

	*(lib_ref->memoria->tipoOp) = PROCESS_ENDED;

	//TODO: despues ver porque es codigo repetido

	int tamanio = *(lib_ref->nombreIO->longitud) + *(lib_ref->memoria->size) +
				sizeof(int) + 6*sizeof(int) + sizeof(state) + sizeof(mate_pointer) + sizeof(operacion) + sizeof(codOperacion) +
				sizeof(type) + sizeof(operation) + *(lib_ref->sema->longitud);


	codOperacion tipoMensaje = PROCESS;

	void * magic = malloc(tamanio);
	int desplazamiento = 0;

	memcpy(magic+desplazamiento,&tipoMensaje,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->tipo,sizeof(type));
	desplazamiento+= sizeof(type);
	memcpy(magic+desplazamiento,lib_ref->sema->operacion,sizeof(operation));
	desplazamiento+= sizeof(operation);
	memcpy(magic+desplazamiento,lib_ref->sema->longitud,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->sema->valor,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->sema->nombre_sem,*(lib_ref->sema->longitud));
	desplazamiento+= *(lib_ref->sema->longitud);

	/*memcpy(magic+desplazamiento,lib_ref->PID,sizeof(pid_t));
	desplazamiento+= sizeof(pid_t);*/
	memcpy(magic+desplazamiento,&unPID,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->estado,sizeof(state));
	desplazamiento+= sizeof(int);

	memcpy(magic+desplazamiento,lib_ref->memoria->size,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->memoria->valor,*(lib_ref->memoria->size));
	desplazamiento+= *(lib_ref->memoria->size);
	memcpy(magic+desplazamiento,lib_ref->memoria->addr,sizeof(mate_pointer));
	desplazamiento+= sizeof(mate_pointer);
	memcpy(magic+desplazamiento,lib_ref->memoria->tipoOp,sizeof(operacion));
	desplazamiento+= sizeof(operacion);

	memcpy(magic+desplazamiento,lib_ref->nombreIO->longitud,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,lib_ref->nombreIO->nombre,*(lib_ref->nombreIO->longitud));
	desplazamiento+= *(lib_ref->nombreIO->longitud);

	log_info(logger, "PID %i: Se mandó un paquete", unPID);

	send(conexionMemoria, magic, desplazamiento, 0);

	//send(conexionMemoria, , , MSG_WAITALL);

	//return lib_ref;
}

void planificadorLargoPlazo() {
	pthread_t hiloFinProceso;
	pthread_create(&hiloFinProceso, NULL, (void*) &finalizarProceso, NULL);
	pthread_detach(hiloFinProceso);
	log_info(logger, "Planificador de Largo Plazo iniciado");
	t_pcb* auxPcb;
	while(1){
		sem_wait(&procesoNuevo);
		if (queue_size(processNew) > 0) {
			if (queue_size(processSuspendedReady) <= 0) {
				sem_wait(&puedeEntrarAlSistema);
				pthread_mutex_lock(&m_colaNew);
				auxPcb = queue_pop(processNew);
				pthread_mutex_unlock(&m_colaNew);
				log_info(logger, "Moviendo proceso con PID %i de NEW a READY",
						auxPcb->PID);
				auxPcb->timestamp_ready_in = temporal_get_string_time("%MS");
				pthread_mutex_lock(&m_colaReady);
				list_add(processReady, auxPcb);
				pthread_mutex_unlock(&m_colaReady);
				sem_post(&cantidadCarpinchosEnReady);
				log_info(logger, "Proceso con PID %i esta READY", auxPcb->PID);
				int *restrict semaforo1 = malloc(sizeof(int));
				int *restrict semaforo2 = malloc(sizeof(int));

				sem_getvalue(&planificar, semaforo1);
				sem_getvalue(&cantidadCarpinchosEnReady, semaforo2);

				printf("\nSem value Planificar: %i\n", *semaforo1);
				printf("\nSem value Cant Carpinchos ready: %i\n", *semaforo2);

				//TODO: esto esta bien????
				if ((list_size(processReady) == 0)||(uso==1)){
					printf("\nEl list size es: %i\n", list_size(processReady));
					printf("\nEl bit de uso: %i\n", uso);
					printf("\nEntre porque la cola de READY tiene 0 elementos\n");
					planificacion = 0;
					sem_post(&planificar);
					usleep(100);
					planificacion--;
					uso = 0;
				}
			}
		} else {
			log_info(logger,
					"No se Agrego ningun proceso a READY por haber alcanzado el maximo grado de multiprogramacion. Hay %i procesos en la cola de NEW",
					queue_size(processNew));
		}
	}
}

void planificarPorSJFSinDesalojo() {
	log_info(logger, "Algoritmo SJF Sin Desalojo Iniciado");
	while (1) {

		sem_wait(&cantidadCarpinchosEnReady);
		sem_wait(&planificar);
		printf("\nEntré a planificar\n");
		printf("\nEl valor de planificacion es: %i\n", planificacion);

		if(planificacion == 0) {
			t_pcb* procesoAExec = elDeMenorSJF(); // implementar calculo de estimacion
			procesoAExec->status = EXEC;
			procesoAExec->timestamp_exec_in = temporal_get_string_time("%MS");
			asignarCPU(procesoAExec);
			log_info(logger, "Se Paso el Proceso %d a EXEC para Ejecutar",
					procesoAExec->PID);

			//TODO: deberia revisar primero que haya una CPU disponible

			//printf("\nEstamos en planificar por SJFSinDesalojo\n");
			//signal a proceso REVISAR
			//sem_post(&entrenadorDesencolado->puedeEjecutar);
		}
		else {
			planificacion = 0;
		}

	}
}

t_pcb* elDeMenorSJF() {
	int iteraciones = list_size(processReady);
	printf("\nEl list size me da: %i\n", list_size(processReady));
	printf("\nEl elements count me da: %i\n", processReady->elements_count);
	//printf("\nNro de iteraciones: %i\n", iteraciones);
	//printf("\nTamanio de cola processReady: %i\n", list_size(processReady));
	//t_pcb* pcbReady;
	float minSJF = 9999999;
	int minID = 0;
	for (int i = 0; i < iteraciones; i++) {
		pthread_mutex_lock(&m_colaReady);
		t_pcb* pcbReady = list_get(processReady, i);
		//printf("\nPID de un Pcb: %i\n", pcbReady->PID);
		pthread_mutex_unlock(&m_colaReady);
		if (pcbReady->rafagaRealCPU == 0) {
			calcularEstimacion(pcbReady);

		}
		printf("\nProxima estimación de PID %i: %f\n", pcbReady->PID, pcbReady->proximaEstimacion);
		if (pcbReady->proximaEstimacion < minSJF) {
			minSJF = pcbReady->proximaEstimacion;
			minID = i;
		}
	}

	t_pcb* pcbElegido = list_get(processReady, minID);
	printf("\nEl PID elegido es: %i\n", pcbElegido->PID);

	pthread_mutex_lock(&m_colaReady);
	list_remove(processReady, minID);
	//printf("\nTamanio de la cola de process Ready: %i\n", list_size(processReady));
	pthread_mutex_unlock(&m_colaReady);
	return pcbElegido;
}

void calcularEstimacion(t_pcb* pcb) {
	pcb->proximaEstimacion = (int) datosConfigKernel->alfa * pcb->rafagaRealCPU
			+ (1 - datosConfigKernel->alfa) * pcb->previaEstimacion;
	log_info(logger, "Estimacion PID %d: %f", pcb->PID, pcb->proximaEstimacion);
}

void planificarPorHRRN() {
	log_info(logger, "Algoritmo HRRN Iniciado");
	while (1) {
		sem_wait(&cantidadCarpinchosEnReady);
		sem_wait(&planificar);//se le hace post cuando termina de ejecutar o va a bloqueado un prceso (proceso CPU)
		t_pcb* pcbCarpincho = elMayorRR(); //Elije segun Ratio mayor
		//validacionSobreCambioDeContextoDeCArpincho(carpinchoDesencolado);
		//carpinchoDesencolado->contadorIntercambio = 0; //ver bien la razón de existencia
		pcbCarpincho->status = EXEC;
		pcbCarpincho->timestamp_exec_in = temporal_get_string_time("%MS");
		//encolarCarpincho(processExec, carpinchoDesencolado, m_colaExec); //mandar a función quilombo
		log_info(logger, "Se Paso el Carpincho %i a EXECUTE",
				pcbCarpincho->PID);
		//sem_post(&carpinchoDesencolado->semaforo);

	}
}

t_pcb* elMayorRR() { //pasar el timestamp a segundos

	int iteraciones = queue_size(processReady);

	t_pcb* carpinchoElegido = list_get(processReady, 0);

	for (int i = 0; i < iteraciones; i++) {

		t_pcb* carpinchoPosible = list_get(processReady, i);

		if (calculoRR(carpinchoPosible) > calculoRR(carpinchoElegido)) {
			carpinchoElegido = carpinchoPosible;
		}

	}

	return carpinchoElegido;
}

int calculoRR(t_pcb* unCarpincho) {
	int tiempoReady = atoi(temporal_get_string_time("%MS"))
			- atoi(unCarpincho->timestamp_ready_in);
	int calculo = 1 + tiempoReady / unCarpincho->previaEstimacion;
	return calculo;
}

void crearCPUS(t_list* listaCpus) { //supongo que se llama al inicio?
	int grado = datosConfigKernel->grado_multiprocesamiento;
	for (int i = 0; i < grado; i++) {

		//Se añaden e inician los procesos CPU
		sem_t *puedeEjecutar = malloc(sizeof(sem_t));
		t_cpu *unaCPU = malloc(sizeof(t_cpu));
		sem_init(puedeEjecutar, 0, 0);
		unaCPU->estado = FREE;
		unaCPU->semaforoCPU = puedeEjecutar;
		pthread_t hiloCPU;
		list_add(listaCpus, unaCPU);
		pthread_create(&hiloCPU, NULL, &ejecutarCarpincho, (void*) unaCPU); //VER QUÉ FUNCION REALIZA
		pthread_detach(hiloCPU);
		log_info(logger, "Creada la CPU %d", i);
		//DESCOMENTAR DESPUES
	}
}

void asignarCPU(t_pcb* pcb) {
	//printf("\nEstamos en asignar CPU\n");

	t_cpu* cpu = buscarCPULibre();
	if (cpu != NULL) {
		cpu->estado = BUSY;
		cpu->pcbCarpincho = pcb;
		sem_post(cpu->semaforoCPU);
		ejecutar = 0;
	}
}

t_cpu* buscarCPULibre() {
	t_cpu *cpu;
	int i;
	while (cpu->estado != FREE) {
		for (i = 0; i < datosConfigKernel->grado_multiprocesamiento; i++) {
			pthread_mutex_lock(&m_listaCPUs);
			cpu = list_get(cpusExec, i);
			pthread_mutex_unlock(&m_listaCPUs);
			if (cpu->estado == FREE) {
				return cpu;
			}
		}
	}
	return NULL;
}

void planificadorMedianoPlazo(){
	/*pthread_t hiloSuspendidoListo;
	pthread_create(&hiloSuspendidoListo, NULL, (void*) &finalizarProceso, NULL);
	pthread_detach(hiloSuspendidoListo);*/
	while(1) {
		sem_wait(&planifMedianoPlazo);
		//usleep(500);
		log_info(logger, "Ejecutando el planificador de Mediano Plazo");
		if (list_size(processReady) == 0 && list_size(processBlocked) != 0
				&& queue_size(processNew) != 0) {
			pthread_mutex_lock(&m_colablocked);
			t_pcb* auxPcb = list_remove(processBlocked,
					list_size(processBlocked) - 1);
			pthread_mutex_unlock(&m_colablocked);
			pthread_mutex_lock(&m_colaSuspendedBlocked);
			log_info(logger,
					"Moviendo el proceso PID %i a Cola Suspendido/Bloqueado",
					auxPcb->PID);
			list_add(processSuspendedBlocked, auxPcb);
			pthread_mutex_lock(&m_colaSuspendedBlocked);
			sem_post(&puedeEntrarAlSistema);

			//TODO: esto lo agregue yo
			//sem_post(&cantidadCarpinchosEnReady);
			//TODO mandar mensaje a modulo de memoria para mandar memoria a swap.
		}
	}
}

void crearIO() {
	int i = 0;
	while (datosConfigKernel->dispositivos_io[i] != NULL) {
		t_io* ioDevice = malloc(sizeof(t_io));
		ioDevice->nombre = datosConfigKernel->dispositivos_io[i];
		ioDevice->duracion_io = atoi(datosConfigKernel->duraciones_io[i]);
		list_add(dispositivosIO, ioDevice);
		log_info(logger, "Creado recurso de IO %s",
				datosConfigKernel->dispositivos_io[i]);
		i++;
	}
	log_info(logger, "Dispositivos IO Creados");

}

void encolarIO(t_pcb* pcb, char* recurso) {
	t_io_req* ioRequest = malloc(sizeof(t_io_req));
	ioRequest->PID = pcb->PID;
	ioRequest->nombre = recurso;
	pthread_mutex_lock(&m_ioQueue);
	queue_push(colaIO, ioRequest);
	pthread_mutex_unlock(&m_ioQueue);
	sem_post(&ejecutarIO);
}

void planificadorIO() {
	log_info(logger, "Iniciado el planificador de IO");
	while (1) {
		int i;
		int rafaga = 0;
		sem_wait(&ejecutarIO);
		pthread_mutex_lock(&m_ioQueue);
		t_io_req* ioReq = queue_pop(colaIO);

		//printf("\nNombre de dispositivo: %s\n", ioReq->nombre);

		pthread_mutex_unlock(&m_ioQueue);
		for (i = 0; i < list_size(dispositivosIO); i++) {
			t_io* dispositivo = list_get(dispositivosIO, i);

			//printf("\nNombre de dispositivo: %s\n", dispositivo->nombre);

			if (strcmp(dispositivo->nombre, ioReq->nombre) == 0) {
				printf("\nEntramos al if\n");
				printf("\nDuracion: %i\n", dispositivo->duracion_io);
				rafaga = dispositivo->duracion_io;
				printf("\nDuracion: %i\n", rafaga);
				break;
			}

			//TODO: poner codigo de error
		}
		log_info(logger, "PID %i: Comienzo Rafaga IO", ioReq->PID);
		usleep(rafaga*1000);
		log_info(logger, "PID %i: Fin Rafaga IO", ioReq->PID);
		desbloquearPID(ioReq->PID);
		free(ioReq);
	}
}

int desbloquearProcesoPorSemaforo(semaforo* semaforo) {
	int i;
	int j;
	//t_pcb* procesoElegido;
	//t_pcb* proceso;
	//TODO: introduje un par de modificaciones acá

	for (i = 0; i < list_size(processSuspendedBlocked); i++) {
		t_pcb* proceso = list_get(processSuspendedBlocked, i);
		printf("\nSemaforo Requerido: %s\n", proceso->semaforoRequerido);
		printf("\nNombre Semaforo: %s\n", semaforo->nombre_sem);
		if (strcmp(proceso->semaforoRequerido, semaforo->nombre_sem) == 0) {
			printf("\nEntramos en el primer desbloquear\n");
			pthread_mutex_lock(&m_colaSuspendedBlocked);
			t_pcb* procesoElegido = list_remove(processSuspendedBlocked, i);
			pthread_mutex_unlock(&m_colaSuspendedBlocked);
			log_info(logger, "PID %i: Se desbloqueo", procesoElegido->PID);
			calcularEstimacion(procesoElegido);
			pthread_mutex_lock(&m_colaSuspendedReady);
			list_add(processSuspendedReady, procesoElegido);
			pthread_mutex_unlock(&m_colaSuspendedReady);

			return 0;
		}
	}

	for (j = 0; j < list_size(processBlocked); j++) {
		t_pcb* proceso = list_get(processBlocked, j);
		printf("\nSemaforo Requerido: %s\n", proceso->semaforoRequerido);
		printf("\nNombre Semaforo: %s\n", semaforo->nombre_sem);
		if (strcmp(proceso->semaforoRequerido, semaforo->nombre_sem)
				== 0) {
			printf("\nEntramos en el segundo desbloquear\n");
			pthread_mutex_lock(&m_colablocked);
			t_pcb* procesoElegido = list_remove(processBlocked, j);
			pthread_mutex_unlock(&m_colablocked);
			log_info(logger, "PID %i: Se desbloqueo", procesoElegido->PID);
			procesoElegido->status = READY;
			calcularEstimacion(procesoElegido);
			pthread_mutex_lock(&m_colaReady);
			list_add(processReady, procesoElegido);
			pthread_mutex_unlock(&m_colaReady);
			sem_post(&cantidadCarpinchosEnReady);
			return 0;
		}
	}
}

//TODO: esto tambien lo retoque un poco
int desbloquearPID(int pid) {
	int i;
	int j;
	for (i = 0; i < list_size(processBlocked); i++) {
		//printf("\nEntramos acá en desbloquear PID process Blocked\n");
		if (((t_pcb*) list_get(processBlocked, i))->PID == pid) {
			log_info(logger, "PID %i: Se desbloqueo", pid);
			pthread_mutex_lock(&m_colablocked);
			t_pcb* procesoElegido = list_remove(processBlocked, i);
			pthread_mutex_unlock(&m_colablocked);
			procesoElegido->status = READY;
			pthread_mutex_lock(&m_colaReady);
			calcularEstimacion(procesoElegido);

			printf("\nEl tamaño de la lista es: %i\n", processReady->elements_count);

			list_add(processReady, procesoElegido);
			printf("\nEl tamaño de la lista es: %i\n", processReady->elements_count);
			pthread_mutex_unlock(&m_colaReady);
			sem_post(&cantidadCarpinchosEnReady);
			//esto lo agregué yo (jose)
			//TODO: creo que aca meti la pata
			//queue_push(processNew, procesoElegido);
			//sem_post(&puedeEntrarAlSistema);
			//sem_post(&procesoNuevo);
			return 0;
		}
	}

	for (j = 0; j < list_size(processSuspendedBlocked); j++) {
		//printf("\nEntramos acá en desbloquear PID process Suspended Blocked\n");
		t_pcb* proceso = list_get(processSuspendedBlocked, j);
		if (proceso->PID == pid) {
			log_info(logger, "PID %i: Se desbloqueo", pid);
			pthread_mutex_lock(&m_colaSuspendedBlocked);
			t_pcb* procesoElegido = list_remove(processSuspendedBlocked,
					i);
			pthread_mutex_unlock(&m_colaSuspendedBlocked);
			sem_post(&suspendidoAReady);
			//esto lo agregué yo (jose)
			//queue_push(processNew, procesoElegido);
			//sem_post(&puedeEntrarAlSistema);
			//sem_post(&procesoNuevo);
			return 0;
		}
	}
}

void reasignarProcesos(char* unSemaforo) {

	for(int i = 0; i < processBlocked->elements_count; i++){

		pthread_mutex_lock(&m_colablocked);
		t_pcb* unPcb = list_get(processBlocked, i);
		pthread_mutex_unlock(&m_colablocked);

		if(strcmp(unPcb->semaforoRequerido,unSemaforo) == 0) {
			unPcb->PID = -1;
			pthread_mutex_lock(&m_colablocked);

			//TODO: esto esta mal
			list_replace(semaforosKernel, unPcb, i);
			pthread_mutex_unlock(&m_colablocked);
		}
	}
}

void crearSemaforoKernel(char* nombre, int valor) {
	int position = verificarExisteSem(nombre);
	if (position == -1) {
		t_sem_kernel* sem_kernel = malloc(sizeof(t_sem_kernel));
		sem_kernel->nombre = nombre;
		sem_kernel->valor = valor;
		pthread_mutex_lock(&m_semaforosKernel);
		list_add(semaforosKernel, sem_kernel);
		pthread_mutex_unlock(&m_semaforosKernel);
		log_info(logger, "Creado el semaforo %s", nombre);
	} else {
		log_error(logger, "El semaforo %s ya existe", nombre);
	}

}

void eliminarSemaforoKernel(char* nombre) {
	//printf("\nEntramos acá\n");

	int position = verificarExisteSem(nombre);

	if (position != -1) {
		pthread_mutex_lock(&m_semaforosKernel);
		t_sem_kernel* semaforo = list_remove(semaforosKernel, position);
		pthread_mutex_unlock(&m_semaforosKernel);
		reasignarProcesos(semaforo->nombre);
		free(semaforo);
		log_info(logger, "El semaforo %s fue destruido", nombre);
	} else {
		log_error(logger, "El semaforo %s no existe", nombre);
	}
}

void suspendidoAListo() {
	while (1) {
		sem_wait(&suspendidoAReady);
		pthread_mutex_lock(&m_colaSuspendedReady);
		t_pcb* auxPcb = queue_pop(processSuspendedReady);
		pthread_mutex_unlock(&m_colaSuspendedReady);
		sem_wait(&puedeEntrarAlSistema);
		pthread_mutex_lock(&m_colaReady);
		list_add(processReady, auxPcb);
		pthread_mutex_unlock(&m_colaReady);
		if (queue_size(processSuspendedReady) == 0) {
			int i;
			for (i = 0; i < queue_size(processNew); i++) {
				sem_post(&procesoNuevo);
			}
		}
	}
}






