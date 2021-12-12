#include "swamp.h"

void iniciar_servidor()
{

	crear_archivos();
	iniciar_marcos();
	iniciar_carpinchos();
	int socket_servidor;
	log_info(logger, "Estoy a la espera en ip: %s, puerto: %s", datosConfigSwamp->ip, datosConfigSwamp->puerto);
	printf("Estoy a la espera en ip: %s, puerto: %s\n", datosConfigSwamp->ip, datosConfigSwamp->puerto);
	socket_servidor = server_bind_listen(datosConfigSwamp->ip, datosConfigSwamp->puerto);

  	server_aceptar_clientes(socket_servidor);
}

void iniciar_carpinchos(){
	carpinchos = list_create();
}

void server_aceptar_clientes(int socket_servidor) {

	struct sockaddr_in dir_cliente;
	pthread_t thread;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	server_atender_cliente((void*)socket_cliente);

	//pthread_create(&thread,NULL,(void*)server_atender_cliente, (void*)socket_cliente);
	//pthread_detach(thread);
}

int recibir_cod_op(int socket){
	int codigoOp;

	recv(socket, &codigoOp, sizeof(int), 0);

	return codigoOp;
}

void* server_atender_cliente(void* socket){

	int socketFD = (int) socket;
	int codigoOp;

	log_debug(logger, "Llego cliente a ip: %s, puerto: %s", datosConfigSwamp->ip, datosConfigSwamp->puerto);
	printf("Llego cliente a ip: %s, puerto: %s\n", datosConfigSwamp->ip, datosConfigSwamp->puerto);

	while(1) {
		codigoOp = recibir_cod_op(socketFD);
		//printf("\nrecibi otro msj\n");

		if((codigoOp == 11)||(codigoOp == 12)||(codigoOp == 13)||(codigoOp == 14)) {
			//printf("\nEmpecé procesar el mensaje\n");
			server_procesar_mensaje(codigoOp, socketFD);
			//printf("\nTerminé de procesar el mensaje\n");
		}
	}

	return NULL;
}

void server_procesar_mensaje(int codOp, int socket) {

	//log_info(logger, "LLego un mensaje valido");
	if (datosConfigSwamp->debug)
		printf("\nLLego un mensaje valido\n");
	paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	int numArchivos = 0;
	numArchivos = obtenerCantArchivos();

	usleep(datosConfigSwamp->retardo_swamp*1000);

	switch (codOp) {
	case LEER:
		log_info(logger, "[ LECTURA ] llego un mensaje de lectura de archivo Swap");
		if (datosConfigSwamp->debug)
			printf("\nLectura de archivo Swap\n");

		leer_swap(socket);

		break;
	case ESCRIBIR:
		log_info(logger, "[ ESCRITURA ] llego un mensaje de ESCRITURA de archivo Swap");
		if (datosConfigSwamp->debug)
		printf("\nEscritura de archivo Swap\n");

		escribir_swap(socket);

		break;
	case PUEDE_ESCRIBIR:
		log_info(logger, "[ VERIFICACION DE ESCIRTURA ] llego un mensaje de VERIFICACION DE ESCIRTURA de archivo Swap");
		if (datosConfigSwamp->debug)
			printf("\nChequea si puede escribir\n");

		fflush(stdout);
		puede_escribir(socket);

		break;
	case CASO_ASIGNACION:
		log_info(logger, "[ TIPO DE ASIGNACION ] Obtengo información sobre el caso de asignación");
		if (datosConfigSwamp->debug)
			printf("\nObtengo información sobre el caso de asignación\n");

		uint32_t tamanioBuffer = 0;
		recv(socket, &tamanioBuffer, sizeof(uint32_t), 0);
		if (datosConfigSwamp->debug)
			printf("\nEl tamanio de Buffer es: %i\n", tamanioBuffer);

		int caso = 0;
		recv(socket, &caso, sizeof(int), 0);
		if (datosConfigSwamp->debug)
			printf("\nEl numero de asignacion es: %i \n", caso);

		int resultado = 0;
		int desplazamiento = 0;
		void* buffer = malloc(sizeof(int));

		switch (caso) {
		case FIJA:
			casoAsignacion = FIJA;
			if (datosConfigSwamp->debug)
				printf("\nEl caso de asignación es fija.\n");
			log_info(logger, "[ TIPO DE ASIGNACION ] El tipo de asignacion es: FIJA");
			//log_info(logger, "\nEl caso de asignación es fija.\n");
			memcpy(buffer, &resultado, sizeof(int));
			desplazamiento += sizeof(int);
			send(socket, buffer, desplazamiento, 0);
			break;
		case GLOBAL:
			casoAsignacion = GLOBAL;
			if (datosConfigSwamp->debug)
				printf("\nEl caso de asignación es global.\n");
//			log_info(logger, "\nEl caso de asignación es global.\n");
			log_info(logger, "[ TIPO DE ASIGNACION ] El tipo de asignacion es: GLOBAL");
			memcpy(buffer, &resultado, sizeof(int));
			desplazamiento += sizeof(int);
			send(socket, buffer, desplazamiento, 0);
			break;
		default:
			log_debug(logger, "\nError obteniendo el caso de asignación\n");
			printf("\nError obteniendo el caso de asignación\n");
			resultado = -1;
			memcpy(buffer, &resultado, sizeof(int));
			send(socket, buffer, desplazamiento, 0);
			break;
		}

		break;
	default:
		printf("\nNo quieras meter la pata\n");
		break;
	}

	free(paquete->buffer);
	free(paquete);
}

void puede_escribir(int socket) {
	uint32_t tamanioBuffer;
	recv(socket,&tamanioBuffer,sizeof(uint32_t),0);
	int PID;
	int indicePagina;
	recv(socket, &PID, sizeof(int), 0);
	int resultado;
	int cantPaginas;
	recv(socket, &cantPaginas, sizeof(int), 0);


	char* archivo = recibirPagina(PID, PUEDE_ESCRIBIR);

	if((chequearAsignacion(PID) == 0)&&(archivo!=NULL)) {


		int hayPaginas = verSiHayPaginas(cantPaginas, archivo);

		if(hayPaginas != -1) {
			resultado = 1;
			if (datosConfigSwamp->debug)
				printf("\n[ PERMISO DE ESCRITURA ] El carpincho %d cantidad de paginas %d puede escribir\n", PID, cantPaginas);
			log_info(logger, "[ PERMISO DE ESCRITURA ] El carpincho %d cantidad de paginas %d puede escribir", PID, cantPaginas);
		}
		else {
			if (datosConfigSwamp->debug)
				printf("\n[ PERMISO DE ESCRITURA ] El carpincho %d cantidad de paginas %d NO puede escribir\n", PID, cantPaginas);
			log_info(logger, "[ PERMISO DE ESCRITURA ] El carpincho %d cantidad de paginas %d NO puede escribir", PID, cantPaginas);
			resultado = -1;
		}
	}
	else {
		log_info(logger, "[ PERMISO DE ESCRITURA ] El carpincho %d cantidad de paginas %d NO puede escribir", PID, cantPaginas);
		if (datosConfigSwamp->debug) {
			printf("\nNo puede escribir2\n");
			printf("\nValor de chequearAsignacion: %i\n", chequearAsignacion(PID));
			printf("\nValor de chequearAsignacion: %s\n", archivo);
		}
		resultado = -1;
	}

	void* buffer = malloc(sizeof(int));
	int desplazamiento = 0;
	estado swap = PUEDE_ESCRIBIR;

	/*memcpy(buffer, &PID, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer, &swap, sizeof(int));
	desplazamiento += sizeof(int);*/
	memcpy(buffer, &resultado, sizeof(int));
	desplazamiento += sizeof(int);

	send(socket, buffer, desplazamiento, 0);

}

int verSiHayPaginas(int cantPaginas, char* archivo1) {
	int cantMarcos = datosConfigSwamp->tamanio_swamp / datosConfigSwamp->tamanio_pagina;

	int marcosLibres = 0;

	for(int i = 0; i < (cantMarcos*obtenerCantArchivos()); i++){

		marco* unMarco = list_get(marcos, i);

		if(strcmp(unMarco->archivo,archivo1) == 0){
			if((unMarco->indice == -1)) {
				marcosLibres++;
			}
		}
	}

	if(marcosLibres >= cantPaginas) {
		return 0;
	}
	else {
		return -1;
	}
}

void crear_archivos() {
	int numArchivos = 0;
	numArchivos = obtenerCantArchivos();
	FILE *file[numArchivos];

	log_info(logger, "Creación de archivo Swap\n");
	printf("Creación de archivo Swap\n");

	for(int i = 0; i < numArchivos; i++) {
		file[i] = fopen(datosConfigSwamp->archivos_swamp[i], "w+");
		for (int count = 0; count < datosConfigSwamp->tamanio_swamp; count++) {
			fputc('\0', file[i]);
		}

		fclose(file[i]);
	}
}

void escribir_swap(int socket) {
	//codOperacion , PID , cantMarcos, buffer

	uint32_t tamanioBuffer;
	recv(socket,&tamanioBuffer,sizeof(uint32_t),0);
	int PID;
	int indicePagina;
	recv(socket, &PID, sizeof(int), 0);
	printf(_RED "\nEL NUMERO DE CARPINCHO ES %d \n"_RESET, PID);
	recv(socket, &indicePagina, sizeof(int), 0);

	char* archivo = recibirPagina(PID, ESCRIBIR);

	int tamanio;
	recv(socket, &tamanio, sizeof(int), 0);
	printf("\nEl tamanio es: %i\n", tamanio);
	void* buffer = malloc(tamanio);
	recv(socket, buffer, tamanio, 0);
	printf("\nBuffer: %s\n", buffer);

	int indiceMarco = hayEspacioArchivo(archivo);

	int desplazamiento = 0;
	int resultado = 0;
	void* bufferSalida = malloc(sizeof(int));

	if((indiceMarco != -1)&&(archivo!=NULL)) {
		int asignacion = chequearAsignacion(PID);

		if(asignacion == 0) {
			FILE *file;
			file = open(archivo,O_RDWR | O_CREAT, (mode_t) 0777); //open es una syscall que es más segura porque me da todos los permisos y puedo hacer lo que quiera
			if(file == -1){
				printf("\nError al abrir el archivo");
			}

			void *archivoMmapeado = mmap(0, datosConfigSwamp->tamanio_swamp,PROT_WRITE | PROT_READ,MAP_SHARED,file,0);

			printf("\nSe escribe: %s\n", bufferSalida);

			memcpy(archivoMmapeado + datosConfigSwamp->tamanio_pagina*indiceMarco, buffer, 64);

			memcpy(bufferSalida, &resultado, sizeof(int));
			desplazamiento += sizeof(int);

			send(socket, bufferSalida, desplazamiento, 0);
			free(bufferSalida);

			printf("\nYa mandamos algo acá\n");

			/*void *archivoMmapeado = mmap(0, datosConfigSwamp->tamanio_swamp,PROT_WRITE | PROT_READ,MAP_SHARED,file,0);
			void* algo2 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
			memcpy(archivoMmapeado + datosConfigSwamp->tamanio_pagina*indiceMarco, algo2, 64);*/

			/*void *archivoMmapeado3 = mmap(0, datosConfigSwamp->tamanio_swamp,PROT_WRITE | PROT_READ,MAP_SHARED,file,0);
			char* algo3 = malloc(7);
			memcpy(algo3, archivoMmapeado3, 3);

			printf("Valor escrito en archivo: %s", algo3);*/

			/*void *archivoMmapeado = mmap(2, datosConfigSwamp->tamanio_swamp,PROT_WRITE | PROT_READ,MAP_SHARED,file,0);
			char* algo = "bbbbb";
			memcpy(archivoMmapeado, algo, 7);*/
			//datosConfigSwamp->tamanio_pagina*indiceMarco
			//fseek(file, indiceMarco*datosConfigSwamp->tamanio_pagina, SEEK_SET);
			//fprintf(file,"%s",buffer);

			actualizarMarcos(archivo, indiceMarco, indicePagina, PID);
			marco* prueba = list_get(marcos, 0);

			actualizarCarpincho(ESCRIBIR, PID);
			if (datosConfigSwamp->debug)
				printf("\nSe realizó exitosamente la escritura\n");
			log_info(logger, "[ ESCRITURA ] El carpincho %d pagina %d pudo escribir exitosamente!", PID, indicePagina);

			close(file);
		}
		else {
			printf("\nNo cumple con el esquema de asignación\n");

			resultado = -1;
			memcpy(bufferSalida, &resultado, sizeof(int));
			desplazamiento += sizeof(int);

			send(socket, bufferSalida, desplazamiento, 0);
			free(bufferSalida);
		}

	}
	else {
		printf("\nNo hay espacio\n");

		resultado = -1;
		memcpy(bufferSalida, &resultado, sizeof(int));
		desplazamiento += sizeof(int);

		send(socket, bufferSalida, desplazamiento, 0);
	}

	free(buffer);

}

void actualizarMarcos(char* archivo, int indiceMarco, int indicePagina, int PID){

	for(int i = 0; i < marcos->elements_count; i++) {
		marco* iterador = list_get(marcos,i);
		if(!strcmp(iterador->archivo, archivo)&&(iterador->marco == indiceMarco)) {
			iterador->indice = indicePagina;
			iterador->PID = PID;
			list_replace(marcos,i,iterador);

			marco* prueba2 = list_get(marcos, i);
		}
	}

}

/*int chequearEntradaNuevosCarpinchos(int PID){

	int marco;

	switch(casoAsignacion) {
	case FIJA:
		for(int i = 0; i < carpinchos->elements_count; i++) {
			carpincho* iterator = list_get(carpinchos,i);
			if(iterator->PID == PID) {
				return 0;
			}
		}
		int contador[datosConfigSwamp->cant_archivos_swamp];
		for(int c = 0; c < datosConfigSwamp->cant_archivos_swamp; c++){
			contador[c] = 0;
			for(int d = 0; d < carpinchos->elements_count; d++) {
				carpincho* iterator = list_get(carpinchos,d);
				if(strcmp(iterator->archivo,datosConfigSwamp->archivos_swamp[d]) == 0){
					contador[c]++;
				}
			}
		}
		for(int e = 0; e < datosConfigSwamp->cant_archivos_swamp; e++) {
			int cantCarpinchos = ((datosConfigSwamp->tamanio_swamp/datosConfigSwamp->tamanio_pagina)/(datosConfigSwamp->marcos_maximos));
			if(contador[e] < cantCarpinchos){
				return 0;
			}
		}
		return -1;
		break;
	case GLOBAL:
		return 0;
		break;
	default:
		return -1;
		break;
	}
	return -1;

}*/

int chequearAsignacion(int PID){

	int marco;

	switch(casoAsignacion) {
	case FIJA:
		for(int i = 0; i < carpinchos->elements_count; i++) {
			carpincho* iterator = list_get(carpinchos,i);
			if(iterator->PID == PID) {
				if(iterator->cantMarcos < datosConfigSwamp->marcos_maximos) {
					return 0;
				}
				else {
					printf("\nError de asignación fija\n");
					return -1;
				}
			}
			else {
				int cantArchivos = obtenerCantArchivos();
				int contador[cantArchivos];

				for(int c = 0; c < cantArchivos; c++){
					contador[c] = 0;
					for(int d = 0; d < carpinchos->elements_count; d++) {
						carpincho* iterator = list_get(carpinchos,d);
						if(strcmp(iterator->archivo,datosConfigSwamp->archivos_swamp[d]) == 0){
							contador[c]++;
						}
					}
				}
				for(int e = 0; e < cantArchivos; e++) {
					int cantCarpinchos = ((datosConfigSwamp->tamanio_swamp/datosConfigSwamp->tamanio_pagina)/(datosConfigSwamp->marcos_maximos));
					if(contador[e] < cantCarpinchos){
						return 0;
					}
				}
				return -1;
			}
		}
		return 0;
		break;
	case GLOBAL:
		printf("\n");
		char* unArchivo = recibirPagina(PID, ESCRIBIR);
		marco = hayEspacioArchivo(unArchivo);
		if (marco != -1) {
			return 0;
		}
		else {
			printf("\nError de asignación global\n");
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}
	return -1;

}

void actualizarCarpincho(estado operacion, int PID){

	switch(operacion) {
	case ESCRIBIR:
		for(int i = 0; i < carpinchos->elements_count; i++) {
			carpincho* iterador = list_get(carpinchos,i);
			if(iterador->PID == PID) {
				iterador->cantMarcos++;
				list_replace(carpinchos,i,iterador);
			}
		}
		break;
	case LEER:
		for(int i = 0; i < carpinchos->elements_count; i++) {
			carpincho* iterador = list_get(carpinchos,i);
			if(iterador->PID == PID) {
				iterador->cantMarcos--;
				list_replace(carpinchos,i,iterador);
			}
		}
		break;
	default:
		printf("\nOperación inválida\n");
		break;
	}

}

int hayEspacioArchivo(char* archivo) {

	int cantMarcos = datosConfigSwamp->tamanio_swamp / datosConfigSwamp->tamanio_pagina;

	for(int i = 0; i <= (cantMarcos*obtenerCantArchivos()); i++){

		marco* unMarco = list_get(marcos, i);

		if(strcmp(unMarco->archivo,archivo)==0){
			if((unMarco->indice == -1)) {
				return unMarco->marco;
			}
		}

	}
	return -1;

}

void iniciar_marcos(){
	int cantMarcos = datosConfigSwamp->tamanio_swamp / datosConfigSwamp->tamanio_pagina;

	marcos = list_create();

	for(int c = 0; c < obtenerCantArchivos(); c++) {

		for(int i = 0; i < cantMarcos; i++){
			marco* unMarco = malloc(sizeof(marco));
			unMarco->marco = i;
			unMarco->archivo = malloc((strlen(datosConfigSwamp->archivos_swamp[c])+1)*sizeof(char));
			memcpy(unMarco->archivo,datosConfigSwamp->archivos_swamp[c],strlen(datosConfigSwamp->archivos_swamp[c])+1);
			unMarco->indice = -1;
			list_add(marcos, unMarco);
		}

	}
}

char* recibirPagina(int PID, estado operacion) {

	for(int i = 0; i < carpinchos->elements_count; i++) {
		carpincho* iterador = list_get(carpinchos,i);

		if(iterador->PID == PID) {
			return iterador->archivo;
		}
	}

	if(operacion == PUEDE_ESCRIBIR) {

		if (hayEspacioArchivo(archivoConMasEspacio()) != -1) {
			return archivoConMasEspacio();
		}
		else {
			return NULL;
		}
	}

	if(operacion == ESCRIBIR) {

		if (hayEspacioArchivo(archivoConMasEspacio()) != -1) {
			carpincho* unCarpincho = malloc(sizeof(carpincho));
			unCarpincho->PID = PID;
			unCarpincho->cantMarcos = 0;
			//unCarpincho->archivo = malloc((strlen(archivoConMasEspacio())+1)*sizeof(char));
			unCarpincho->archivo = archivoConMasEspacio();

			list_add(carpinchos, unCarpincho);

			return unCarpincho->archivo;
		}
		else {
			return NULL;
		}
	}
	else {
		return NULL;
	}

}

void leer_swap(int socket) {
	//codOperacion , PID , cantMarcos, buffer

	uint32_t tamanioBuffer;
	recv(socket,&tamanioBuffer,sizeof(uint32_t),0);

	int PID;
	int indicePagina;
	printf("\n[LECTURA]: El PID1 de lectura: %i\n", PID);
	recv(socket, &PID, sizeof(int), 0);
	printf("\n[LECTURA]: El PID2 de lectura: %i\n", PID);
	recv(socket, &indicePagina, sizeof(int), 0);
	printf("\n[LECTURA]: La pagina es: %i\n", indicePagina);

	char* archivo = recibirPagina(PID, LEER);

	if(archivo != NULL) {
		printf("\n[LECTURA]: Entra con archivo\n");
		FILE* file;
		file = fopen(archivo, "r");

		char* contenido = malloc(datosConfigSwamp->tamanio_pagina);

		if(conseguirMarco(PID, indicePagina) != -1) {
			printf("\n[LECTURA]: Entra en conseguirMarco\n");
			fseek(file, conseguirMarco(PID, indicePagina)*datosConfigSwamp->tamanio_pagina, SEEK_SET);
			fgets(contenido,datosConfigSwamp->tamanio_pagina,file);
			if (datosConfigSwamp->debug)
				printf("\nContenido leido: %s\n", contenido);
			printf("\nEl PID es: %i\n", PID);
			printf("\nEl Indice de Pagina es: %i\n", indicePagina);
			printf("\nEl tamanio de pagina es: %i\n", datosConfigSwamp->tamanio_pagina);

			log_info(logger, "[ LEER ] El carpincho %d pagina %d pudo leer", PID, indicePagina);
			printf("\nEl PID es: %i\n", PID);
			printf("\nEl Indice de Pagina es: %i\n", indicePagina);
			printf("\nEl tamanio de pagina es: %i\n", datosConfigSwamp->tamanio_pagina);

			printf("\nContenido leido: %s\n", contenido);



			int tamanio = sizeof(int)*3 + datosConfigSwamp->tamanio_pagina;
			void* buffer = malloc(tamanio);
			int desplazamiento = 0;
			memcpy(buffer+desplazamiento, &PID, sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(buffer+desplazamiento, &indicePagina, sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(buffer+desplazamiento, &(datosConfigSwamp->tamanio_pagina), sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(buffer+desplazamiento, contenido, datosConfigSwamp->tamanio_pagina);
			desplazamiento += datosConfigSwamp->tamanio_pagina;
			send(socket, buffer, desplazamiento, 0);

			free(contenido);
			free(buffer);

			actualizarMarcos(archivo, conseguirMarco(PID, indicePagina), -1, PID);
			actualizarCarpincho(LEER, PID);
		}
		else {
			printf("\n[LECTURA]: No entra en conseguirMarco\n");
			int tamanio = sizeof(int);//sizeof(int)*3 + datosConfigSwamp->tamanio_pagina;
			void* buffer = malloc(tamanio);
			int desplazamiento = 0;
			PID = -1;
			memcpy(buffer, &PID, sizeof(int));
			desplazamiento += sizeof(int);
			/*memcpy(buffer, &indicePagina, sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(buffer, &(datosConfigSwamp->tamanio_pagina), sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(buffer, contenido, datosConfigSwamp->tamanio_pagina);
			desplazamiento += datosConfigSwamp->tamanio_pagina;
			send(socket, buffer, desplazamiento, 0);*/

			free(contenido);

			printf("\nNo existe esa página\n");
		}

		fclose(file);
	}
	else {
		printf("\n[LECTURA]: No entra con archivo\n");

		char* contenido = malloc(datosConfigSwamp->tamanio_pagina);
		int tamanio = sizeof(int)*3 + datosConfigSwamp->tamanio_pagina;
		void* buffer = malloc(tamanio);
		int desplazamiento = 0;
		PID = -1;
		memcpy(buffer, &PID, sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer, &indicePagina, sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer, &(datosConfigSwamp->tamanio_pagina), sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer, contenido, datosConfigSwamp->tamanio_pagina);
		desplazamiento += datosConfigSwamp->tamanio_pagina;
		send(socket, buffer, desplazamiento, 0);

		free(contenido);

		printf("\nNo existe esa página\n");
		printf("\nNo existe ese proceso en memoria\n");
	}

}

int conseguirMarco(int PID, int indicePagina) {
	for(int i = 0; i < marcos->elements_count; i++) {
		marco* iterador = list_get(marcos, i);
		if ((iterador->PID == PID) && (iterador->indice == indicePagina)) {
			return i;
		}
	}
	return -1;
}

char* archivoConMasEspacio() {

	int numArchivos = 0;
	numArchivos = obtenerCantArchivos();

	FILE *file[numArchivos];

	char c;
	int contador[numArchivos];
	int menor;

	for(int i = 0; i < numArchivos; i++) {

		contador[i] = 0;
		file[i] = fopen(datosConfigSwamp->archivos_swamp[i], "r");

		while ((c = fgetc(file[i])) != EOF) {
			if (c != '\0') {
				contador[i]++;
			}
		}
		ftruncate(file[i], datosConfigSwamp->tamanio_swamp);
		fclose(file[i]);
	}

	menor = 0;

	for(int i = 0; i < numArchivos; i++) {
		if (contador[i] < contador[menor]) {
			menor = i;
		}
	}

	return datosConfigSwamp->archivos_swamp[menor];
}

int obtenerCantArchivos() {

	char *config = malloc(sizeof(char)*200); //ver de hacerlo dinámico
	//char *token = malloc(sizeof(char)*100);
	char *s = ",";

	//int i = 0;

	memcpy(config, datosConfigSwamp->cant_archivos_swamp, strlen(datosConfigSwamp->cant_archivos_swamp)); //copio la ruta al config
	char *token = strtok(config, s); //divido la ruta por comas
	int i = 0;
	while (token != NULL) {
		i++;
		token = strtok(NULL, s); //saco las rutas sucesivas
	}

	/*memcpy(config, datosConfigSwamp->archivos_swamp, strlen(datosConfigSwamp->archivos_swamp));
	char *token = strtok(config, s);
	int i = 0;
	while (token != NULL) {
		i++;
		token = strtok(NULL, s);
	}*/
	//printf("Cant Archivos: %i\n", i);
	//printf("%s", datosConfigSwamp->archivos_swamp);
	free(config);
	return i;
	/*printf("%s\n", token);
	token = strtok(NULL, s);
	printf("%s\n", token);
	token = strtok(NULL, s);
	printf("%s\n", token);*/

}

/*t_list *crearLista() {

	printf("\nVamos a crear una lista\n");

	t_list * nombreArchivos = list_create();

	char *s = ",";
	char *t = "[";
	char *u = "]";
	char *token = strtok(datosConfigSwamp->archivos_swamp, s);

	printf("\nAntes de emprolijar tokens\n");

	while (token != NULL) {
		printf("\nEntramos una vez\n");
		printf("Token: %s", token);
		token = strtok(NULL, s);
		//printf("\n1\n");
		//obtenerNombre(token, '[');
		//printf("\n2\n");
		//obtenerNombre(token, ']');
		//printf("\n3\n");
		printf("Token: %s", token);
		list_add(nombreArchivos,token);
		//Sacar chars incorrectos
	}

	//printf("Cant Archivos: %i", i);
	//return i;
	//list_add(nombreArchivos,unElemento);

	return nombreArchivos;

}*/

void obtenerNombre(char* token, char letra) {
	char *pr = token, *pw = token;
	while(*pr) {
		*pw = *pr++;
		pw += (*pw != letra);
	}
	*pw = '\0';
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

	/*memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));*/

	return objeto;
}

int noSuperaMaximo(int totalMemoria) {

	if (totalMemoria > datosConfigSwamp->tamanio_swamp) {
		return 0;
	}
	else {
		return 1;
	}
}

void deserializar_mensaje(t_paquete* paquete, int socket) {

	paquete->buffer->size = malloc(sizeof(uint32_t));
	log_info(logger, "\nInicio deserealizacion\n");
	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);
	log_info(logger, "\nFin deserealizacion\n");
}
