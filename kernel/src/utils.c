#include"utils.h"
#include "lib.h"




int iniciar_servidor(char *ip, char* puerto)
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
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	uint32_t handshake;
	uint32_t resultOk = 0;
	uint32_t resultError = -1;

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor,NULL,NULL);

	recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);

	if(handshake == 1) {
		send(socket_cliente, &resultOk, sizeof(uint32_t), NULL);
	}
	else {
		send(socket_cliente, &resultError, sizeof(uint32_t), NULL);
	}

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) >= 0) {
		if(cod_op != 1) {
			close(socket_cliente);
		}
		//printf("\nCOD OP1: %i\n", cod_op);
		return cod_op;
	}
	else
	{
		//close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista
t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	uint32_t handshake = 1;
	uint32_t result;

	send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
	recv(socket_cliente, &result, sizeof(uint32_t), MSG_WAITALL);

	freeaddrinfo(server_info);

	return socket_cliente;
}

t_pcb* create_pcb(int socket, mate_instance* carpincho) {
	t_pcb* pcb;
	pcb = malloc(sizeof(t_pcb));
	//pcb->socket=malloc(sizeof(int));
	//*(pcb->socket)=socket;
	pcb->socket=socket;
	//modificar PID
	pcb->PID = *(carpincho->PID);
	pcb->previaEstimacion = datosConfigKernel->estimacion_inicial;
	pcb->rafagaRealCPU = 0;
	pcb->status = NEW;
	pcb->IO_finished = 0;
	return pcb;
}
