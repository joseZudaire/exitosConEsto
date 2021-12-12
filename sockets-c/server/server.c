#include "server.h"

void iniciar_servidor()
{
	int socket_servidor;
	log_debug(logger, "Estoy a la espera en ip: %s, puerto: %s", datosConfigServer->ip, datosConfigServer->puerto);
	printf("Estoy a la espera en ip: %s, puerto: %s\n", datosConfigServer->ip, datosConfigServer->puerto);
	socket_servidor = server_bind_listen(datosConfigServer->ip, datosConfigServer->puerto);

    while(1)
    	server_aceptar_clientes(socket_servidor);
}

void server_aceptar_clientes(int socket_servidor) {

	struct sockaddr_in dir_cliente;
	pthread_t thread;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	pthread_create(&thread,NULL,(void*)server_atender_cliente, (void*)socket_cliente);
	pthread_detach(thread);
}

uint8_t recibir_cod_op(int socket){
	uint8_t codigoOp;

	recv(socket, &codigoOp, sizeof(uint8_t), 0);

	return codigoOp;
}

void* server_atender_cliente(void* socket){

	int socketFD = (int) socket;
	uint8_t codigoOp;

	log_debug(logger, "Llego cliente a ip: %s, puerto: %s", datosConfigServer->ip, datosConfigServer->puerto);
	printf("Llego cliente a ip: %s, puerto: %s\n", datosConfigServer->ip, datosConfigServer->puerto);

	codigoOp = recibir_cod_op(socketFD);

	server_procesar_mensaje(codigoOp, socketFD);

	return NULL;
}

void server_procesar_mensaje(uint8_t codOp, int socket) {

	log_info(logger, "Nueva conexion");
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->codigo_operacion = codOp;
	t_msg* nodo_mensaje;
	t_buffer* buffer;

	switch (paquete->codigo_operacion) {
	case SERVER_MENSAJE:;
		log_info(logger, "Llegada de un nuevo mensaje con el codigo 1\n");
		printf("Llegada de un nuevo mensaje con el codigo 1\n");

		deserializar_mensaje(paquete,socket);
		printf("\n %s \n", paquete->buffer->stream);

		break;
	case MENSAJE:;
		log_info(logger, "Llegada de un nuevo mensaje con el codigo 2\n");
		printf("Llegada de un nuevo mensaje con el codigo 2\n");

		deserializar_mensaje(paquete,socket);
		t_msg_1* mensaje = deserializar_msg_parametros_cuatro(paquete->buffer->stream);

		printf("\n inicio mensaje descerializado \n");
		printf(" nombre: %s \n", mensaje->name);
		printf(" posicionX: %i \n", mensaje->posicionX);
		printf(" posicionY: %i \n", mensaje->posicionY);
		printf(" id: %i \n", mensaje->posicionX);
		printf("\n fin mensaje descerializado \n");

		break;


	case 0:
		pthread_exit(NULL);
        // del otro lado se cerro la conexion
	}
}


int server_bind_listen(char* ip, char* puerto)
{
	int socket_servidor;

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

    return socket_servidor;
}

t_msg_1* deserializar_msg_parametros_cuatro (char* stream){
	t_msg_1* objeto;
	objeto = malloc(sizeof(t_msg_1));
	memcpy(&(objeto->length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	objeto->name = malloc(objeto->length);
	memcpy(objeto->name, stream, objeto->length);
	stream += objeto->length;
	
	memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	
	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));

	return objeto;
}

void deserializar_mensaje(t_paquete* paquete, int socket) {

	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);

}