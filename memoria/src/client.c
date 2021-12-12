#include "client.h"

int crear_conexion_swamp(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(ip, puerto, &hints, &server_info);
	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		freeaddrinfo(server_info);
		log_debug(logger, "Fallo al conectarme al ip: %s, puerto: %s", ip, puerto);
		return -1;
	}
	freeaddrinfo(server_info);
	log_debug(logger, "Me conecto al ip: %s, puerto: %s con exito!", ip, puerto);
	return socket_cliente;
}

void enviar_paquete_swamp(t_buffer* buffer, int codigo_operacion) {

	t_paquete* paquete = malloc(sizeof(t_paquete));

	//paquete->codigo_operacion = (uint8_t) codigo_operacion;
	//paquete->codigo_operacion = codigo_operacion;
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
	int offset = 0;

	/*memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);*/
	memcpy(envio + offset, &codigo_operacion, sizeof(int));
	offset += sizeof(int);
	/*memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);*/
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	send(socket_swamp, envio, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

	log_info(logger, "Se envio un mensaje a swamp codigo operacion: %d", codigo_operacion);
	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


int enviar_tipo_asignacion() {
	int asignacion = -1;

	if(strcmp(datosConfigMemoria->tipo_asignacion,"FIJA")==0) {
		asignacion = 0;
		log_debug(logger, "Se envia el tipo de asignacion a swamp de tipo FIJA");
	}
	if(strcmp(datosConfigMemoria->tipo_asignacion,"DINAMICA")==0) {
		asignacion = 1;
		log_debug(logger, "Se envia el tipo de asignacion a swamp de tipo DINAMICA");
	}
	
	t_buffer* buffer = malloc(sizeof(t_buffer));

	//uint32_t largo = strlen(tipo_asignacion) + 1;

	buffer->size = sizeof(int);

	//buffer->size += sizeof(uint32_t);
	void* stream = malloc(buffer->size);
	int offset = 0;

	// Serializo el tamano del tipo_asignacion
	/*
		memcpy(stream + offset, &(buffer->size), sizeof(uint32_t));
		offset += sizeof(uint32_t);
	*/
	// Serializo el tipo_asignacion
	printf(_RED"\n\nEl tipo de asignacion es %d\n\n"_RESET, asignacion);
	memcpy(stream, &asignacion, sizeof(int));
	offset += sizeof(int);

	
	buffer->stream = stream;

//	send()
	enviar_paquete_swamp(buffer, 14);

//	free(buffer);

	int resultado = -2;
	recv(socket_swamp, &resultado, sizeof(int), 0);

	if (datosConfigMemoria->debug)
		printf(_YELLOW "\n El resultado de la descerializacion es %d \n" _RESET, resultado);

	return resultado;

}

int enviar_consulta_puede_escribir_en_swap(int socket, int carpinchoID, int cantPag)
{

	t_buffer* buffer = malloc(sizeof(t_buffer));

	//uint32_t largoCarpinchoId = sizeof(carpinchoID);

	//uint32_t largoCantPag = sizeof(cantPag);

	//buffer->size = sizeof(uint32_t) + largoCarpinchoId + largoCantPag;
	buffer->size = sizeof(int)*2;

	void* stream = malloc(buffer->size);
	int offset = 0;

	/*memcpy(stream + offset, &largoCarpinchoId, sizeof(uint32_t));
	offset += sizeof(uint32_t);*/

	memcpy(stream + offset, &carpinchoID, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &cantPag, sizeof(int));
	offset += sizeof(int);

	/*memcpy(stream + offset, cantPag, largoCantPag);
	offset += largoCantPag;*/

	buffer->stream = stream;

	enviar_paquete_swamp(buffer, 13);
	log_info(logger,"[ PUEDE ESCRIBIR ] El carpincho %d consulta si puede escribir %d paginas (comunicacion con swamp cop_op13)", carpinchoID, cantPag);

	//free(buffer);

	int resultado = 0;

	recv(socket_swamp, &resultado, sizeof(int), 0);
	//free(buffer);

	return resultado;
}

t_buffer* serializar_escritura_swap(int pid_carpincho, int pagina, char* msg){

	if (datosConfigMemoria->debug) {
		printf("\n Serializo escritura a swamp proceso: %d pagina: %d\n", pid_carpincho, pagina);
	}
	
	t_buffer* buffer = malloc(sizeof(t_buffer));

	int largo = strlen(msg) + 1;

	buffer->size = sizeof(int)*3 + largo;

	//buffer->size += sizeof(uint32_t);

	void* stream = malloc(buffer->size);
	int offset = 0;

	// Serializo pid
	memcpy(stream + offset, &pid_carpincho, sizeof(int));
	offset += sizeof(int);
	// Serializo pagina
	memcpy(stream + offset, &pagina, sizeof(int));
	offset += sizeof(int);
	// Serializo el tamano del msg
	memcpy(stream + offset, &largo, sizeof(int));
	offset += sizeof(int);
	// Serializo el msg

	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\nEl mensaje es: %s\n", msg);
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");
	printf("\n---------------------------------------\n");

	memcpy(stream + offset, msg, largo);
	offset += largo;

	if (datosConfigMemoria->debug) {
		printf(" msg: %s \n", msg);
		printf(" pid carpincho: %d \n", pid_carpincho);
		printf(" pagina: %d \n", pagina);

		printf("\n fin mensaje descerializado \n");

	}
	buffer->stream = stream;
	return buffer;
}


int enviar_escritura_swap(int pid_carpincho, int pagina, char* msg) {
	t_buffer* buffer = serializar_escritura_swap(pid_carpincho, pagina, msg);
	enviar_paquete_swamp(buffer, 12);
	log_info(logger,"[ ESCRIBIR SWAMP ] El carpincho %d quiere escribir la pagina %d msg %s(comunicacion con swamp cop_op12)", pid_carpincho, pagina, msg);
	int resultado = 0;

	recv(socket_swamp, &resultado, sizeof(int), 0);
	//free(buffer);

	return resultado;
}


int lectura_swamp(int pid_carpincho, int pagina, int marco) {
	if (datosConfigMemoria->debug)
		printf("\n Serializo lectura a swamp proceso: %d pagina: %d\n", pid_carpincho, pagina);

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(int) * 2;

	buffer->size += sizeof(uint32_t);

	void* stream = malloc(buffer->size);
	int offset = 0;

	// Serializo pid
	memcpy(stream + offset, &pid_carpincho, sizeof(int));
	offset += sizeof(int);
	// Serializo pagina
	memcpy(stream + offset, &pagina, sizeof(int));
	offset += sizeof(int);


	if (datosConfigMemoria->debug) {
		printf(" pid carpincho: %d \n", pid_carpincho);
		printf(" pagina: %d \n", pagina);
		printf("\n fin mensaje descerializado \n");
	}
	buffer->stream = stream;

	enviar_paquete_swamp(buffer, 11);
	log_info(logger,"[ LEER SWAMP ] El carpincho %d quiere leer la pagina %d (comunicacion con swamp cop_op11)", pid_carpincho, pagina);
	int PID = 0;
	int indicePagina = 0;
	int tamanioPagina = 0;
	void* contenidoLectura = malloc((int) datosConfigMemoria->tamanio_pagina);

	recv(socket_swamp, &PID, sizeof(int), 0);
	printf("\n\nEl PID es: %i\n\n", PID);
	if (datosConfigMemoria->debug) {
		printf(_RED"\n EL PID ES %d \n"_RESET, PID);
	}
	//free(buffer);

	if(PID == -1) {
		return -1;
	}
	else {
		//return 0;
		recv(socket_swamp, &indicePagina, sizeof(int), 0);
		recv(socket_swamp, &tamanioPagina, sizeof(int), 0);
		recv(socket_swamp, contenidoLectura, (int) datosConfigMemoria->tamanio_pagina, 0);
		escribir_en_cache(marco, contenidoLectura, datosConfigMemoria->tamanio_pagina, 0);
		free(contenidoLectura);
		return 0;
	}

}
