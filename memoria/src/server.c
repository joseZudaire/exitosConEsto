#include "server.h"
#include "memory.h"

void iniciar_servidor() {
	int socket_servidor;
	log_info(logger, "Inicio a la espera en ip: %s, puerto: %s\n", datosConfigMemoria->ip, datosConfigMemoria->puerto);
	socket_servidor = server_bind_listen(datosConfigMemoria->ip, datosConfigMemoria->puerto);

	pthread_t hiloLlegada;
	int socket_server = 0;

    while(1) {
		socket_server = esperar_cliente(socket_servidor);

		if ((socket_server != 0) && (socket_server != -1)) {
			log_info(logger, "Valor de socket servidor: %i", socket_server);
			pthread_create(&hiloLlegada, NULL, &recibirOperaciones, socket_server);
		}
    }
}

int esperar_cliente(int socketServidor)
{
	log_info(logger,"Estoy esperando clientes");

	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	uint32_t handshake;
	uint32_t resultOk = 0;
	uint32_t resultError = -1;

	// Aceptamos un nuevo cliente
	int socketCliente = accept(socketServidor,NULL,NULL);

	recv(socketCliente, &handshake, sizeof(uint32_t), MSG_WAITALL);

	if(handshake == 1) {
		log_info(logger, "Se conecto un cliente!");
		send(socketCliente, &resultOk, sizeof(uint32_t), NULL);
	}
	else {
		send(socketCliente, &resultError, sizeof(uint32_t), NULL);
	}


	return socketCliente;
}


void* recibirOperaciones(int cliente_fd) {
	void* buffer;
	int cod_op;

	log_info(logger, "Creamos un nuevo hilo para el cliente que se conecto");

	while(1) {
		cod_op = recibir_operacion(cliente_fd);

		if (cod_op == 1) {
			//mate_instance* carpincho = deserializar_instancia(buffer,cliente_fd);
			//log_info(logger, "PID %i: Llegó un carpincho", *(carpincho->PID));

			server_procesar_mensaje(1,cliente_fd);

			/*if(*(carpincho->tipo) == FINALIZACION) {
				pthread_exit(0);
				//log_info(logger, "PID %i: El carpincho terminó", *(carpincho->PID));
			}*/
			//ejecutarCarpincho(carpincho, cliente_fd);

		}
	}

	return buffer;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) >= 0) {
		if(cod_op != 1) {
			close(socket_cliente);
		}
		printf("\nCOD OP1: %i\n", cod_op);
		return cod_op;
	}
	else
	{
		//close(socket_cliente);
		return -1;
	}
}

mate_instance* deserializar_instancia(void *magic, int socket_cliente) {

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

	/*void* algo1 = malloc(sizeof(int));
	int algo12 = 0;

	memcpy(algo1, &algo12, sizeof(int));

	usleep(1000000);

	printf("\nEl resultado de mandar el retorno fue: %i\n", send(socket_cliente, algo1, sizeof(int), 0));*/

	/*
	 * PARA PROBAR CÓMO LLEGAN LOS PAQUETES
	 *
	 */

	log_info(logger, "Tipo: %i", *(nuevaInstancia->tipo));
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
	log_info(logger, "Tipo: %i", *(nuevaInstancia->tipo));


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



int recibir_cod_op(int socket){
	int codigoOp;

	recv(socket, &codigoOp, sizeof(int), 0);

	return codigoOp;
}

void server_procesar_mensaje(uint8_t codOp, int socket)
{

	//TODO:Hay q liberar las structs de recibir mensajes?

	log_info(logger, "[SERVER_PROCESAR_MENSAJE] - Procesando mensaje");

	t_paquete* paquete = malloc(sizeof(t_paquete));
	//paquete->buffer = malloc(sizeof(t_buffer));
	//paquete->codigo_operacion = codOp;
	char* ptr_cache;
	t_buffer* buffer;

	log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - Antes de deserealizar");
	mate_instance* mate_instans = deserializar_instancia(buffer,socket);
	log_debug(logger, "[SERVER_PROCESAR_MENSAJE] - Despues de deserealizar");
	int resultado = 0;

	int pid = 0;
	uint32_t size =  0;
	uint32_t addr =  0;
	char* valor = NULL;

	switch(*(mate_instans->tipo)) {
	case MEMORIA:
		switch ((int) *(mate_instans->memoria->tipoOp)) {
			case MEMALLOC:
				luz_roja_memalloc();
				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMALLOC - Un carpincho solicita  memalloc");

				uint32_t direccion = 0;
				pid = *(mate_instans->PID);
				size =  *(mate_instans->memoria->size);

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMALLOC - PID:%i - SIZE:%i", pid, size);

				int resultadoMemoriaAlloc = memory_memalloc(pid,size, &direccion);

				if(resultadoMemoriaAlloc == EXITO)
				{
					//Enviamos cod error y la direccion
					resultado = MATE_ALLOC_OK;
					//send(socket, &resultado, sizeof(int), 0);
					printf(_RED"\n\n\n EXITO MEMALLOC LE QUIERO ENVIAR A KERNEL: %d\n\n\n"_RESET,resultado);
					send(socket, &direccion, sizeof(uint32_t), 0);
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMALLOC - Direccion enviada - Valor:%i", direccion);
				}
				else
				{
					//No hay espacio, enviamos error
					resultado = MATE_ALLOC_FAULT;
					printf(_RED"\n\n\n FALLO MEMALLOC LE QUIERO ENVIAR A KERNEL: %d\n\n\n"_RESET,resultado);
					send(socket, &resultado, sizeof(uint32_t), 0);
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMALLOC - No hay espacio MATE FAULT enviado");
				}

				imprimir_marcos();
				//close(socket);
				luz_verde_memalloc();
				break;
			case MEMFREE:
				luz_roja_memalloc();

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFREE - Un carpincho solicita  memfree");

				pid = *(mate_instans->PID);
				addr =  *(mate_instans->memoria->addr);

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFREE - PID:%i - ADDR:%i", pid, addr);

				int resultadoMemoriaFree = memory_memfree(pid, addr);

				if(resultadoMemoriaFree == EXITO)
				{
					//Liberamos correctamente
					resultado = MATE_FREE_OK;
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFREE - Se enviará FREE OK");
				}
				else
				{
					//Error direccion invalida, enviamos error
					resultado = MATE_FREE_FAULT;
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFREE - Se enviara FREE FAULT");
				}

				send(socket, &resultado, sizeof(int), 0);

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFREE - Respuesta enviada - Valor:%i", resultado);

				//close(socket);
				imprimir_marcos();
				luz_verde_memalloc();
				break;
			case MEMREAD:
				luz_roja_memalloc();

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMREAD - Un carpincho solicita  memread");

				pid = *(mate_instans->PID);
				size = *(mate_instans->memoria->size);
				addr =  *(mate_instans->memoria->addr);

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMREAD - PID:%i - SIZE:%i - ADDR:%i", pid, size, addr);

				char* buffer = malloc(sizeof(char)*size);

				int resultadoMemoriaRead = memory_memread(pid, addr, size, buffer);

				if(resultadoMemoriaRead == EXITO)
				{
					//Se leyo con exito, enviamos OK y el valor leido
					resultado = MATE_READ_OK;

					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMREAD - Leido correctamente");

					int tamanio = size;
					send(socket, &tamanio, sizeof(int), 0);					
					send(socket, buffer, tamanio*sizeof(char), 0);
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMREAD - Tamanio y valor leid enviado - Tamanio:%i - Leido:%s", tamanio, buffer);
				}
				else
				{
					//Error direccion invalida
					resultado = MATE_READ_FAULT;
					send(socket, &resultado, sizeof(int), 0);
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMREAD - No se puede leer, MATE FAULT enviado - Valor:%i", resultado);
				}
				//close(socket);
				imprimir_marcos();
				luz_verde_memalloc();
				break;
			case MEMWRITE:
				luz_roja_memalloc();

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMWRITE - Un carpincho solicita  memwrite");

				pid = *(mate_instans->PID);
				size =  *(mate_instans->memoria->size);
				addr =  *(mate_instans->memoria->addr);

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMWRITE - PID:%i - SIZE:%i - ADDR:%i", pid, size, addr);

				char* valor = mate_instans->memoria->valor;

				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMWRITE - Valor a escribir:%s", valor);

				int resultadoMemoriaWritte = memory_memwrite(pid,addr,size,valor);

				if(resultadoMemoriaWritte == EXITO)
				{
					//Se escribio exitosamente
					resultado = MATE_WRITE_OK;
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMWRITE - Se enviara WRITE OK");
				}
				else
				{
					//Error direccion invalida
					resultado = MATE_WRITE_FAULT;
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMWRITE - Se enviara WRITE FAULT");
				}

				send(socket, &resultado, sizeof(int), 0);
				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMWRITE - Respuesta enviada - Valor:%i", resultado);

				//close(socket);
				imprimir_marcos();
				luz_verde_memalloc();
				break;
			case FINALIZACION:
				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFIN - Un carpincho solicita  memfin");

				pid = *(mate_instans->PID);

				int resultadoMemoriaFin = memory_memfree_all(pid);

				if(resultadoMemoriaFin == EXITO)
				{
					//Liberamos correctamente
					resultado = MATE_END_OK;
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFIN - Se enviara FIN OK");
				}
				else
				{
					//Error direccion invalida, enviamos error
					resultado = MATE_END_FAULT;
					log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - Se enviara FIN FAULT");
				}

				send(socket, &resultado, sizeof(int), 0);
				log_debug(logger,"[SERVER_PROCESAR_MENSAJE] - MEMFIN - Respuesta enviada - Valor:%i", resultado);

				//close(socket);
				imprimir_marcos();
				//pthread_exit(0);
				break;
			default:
				log_info(logger, "[SERVER_PROCESAR_MENSAJE] - Error al procesar a solicitud de un cliente, codigo no identificado");
				break;
		}
		break;
	default:
		log_info(logger, "[SERVER_PROCESAR_MENSAJE] - Error al procesar a solicitud de un cliente, operacion invalida");
		resultado = -10;
		send(socket, &resultado, sizeof(int), 0);
		break;
	}

	cache_imprimir();
	// del otro lado se cerro la conexion
}

int server_bind_listen(char* ip, char* puerto)
{

	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    // Creamos el socket de escucha del servidor
    socket_servidor = socket(servinfo->ai_family,
    						servinfo->ai_socktype,
							servinfo->ai_protocol);
    // Asociamos el socket a un puerto
    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

    // Escuchamos las conexiones entrantes
    listen(socket_servidor, SOMAXCONN);

    //int socket_cliente = accept(socket_servidor,NULL,NULL);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;

	/*int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;*/
}
