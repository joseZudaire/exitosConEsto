#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/config.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include "matelib.h"
#include "utils.h"

//TODO reformular logica de mensajes

//------------------General Functions---------------------/

int logger_iniciado = 0;

void asignar_puerto_e_IP(char *unaConfig, mate_instance *lib_ref){

	t_config* config;

	config = iniciar_config(unaConfig);

	char* ip = config_get_string_value(config, "IP");
	char* puerto = config_get_string_value(config, "PUERTO");

	((carpincho*) lib_ref->group_info)->ip = malloc(sizeof(char)*(strlen(ip)+1));
	((carpincho*) lib_ref->group_info)->puerto = malloc(sizeof(char)*(strlen(puerto)+1));

	memcpy(((carpincho*) lib_ref->group_info)->ip,ip,strlen(ip)+1);
	memcpy(((carpincho*) lib_ref->group_info)->puerto,puerto,strlen(puerto)+1);

	config_destroy(config);
}

int mate_init(mate_instance *lib_ref, char *config)
{
	if (mate_logger == NULL) {

		mate_logger = mate_iniciar_logger();

	}

//	lib_ref = malloc(sizeof(mate_instance));
	//lib_ref = malloc(sizeof(mate_instance));

	lib_ref->group_info = malloc(sizeof(carpincho));

	asignar_puerto_e_IP(config, lib_ref);

	int conexion = crear_conexion(((carpincho*) lib_ref->group_info)->ip, ((carpincho*) lib_ref->group_info)->puerto);
	((carpincho*) lib_ref->group_info)->socketConexion = malloc(sizeof(int));
	*(((carpincho*) lib_ref->group_info)->socketConexion) = conexion;

	((carpincho*) lib_ref->group_info)->tipo = malloc(sizeof(type));
	*(((carpincho*) lib_ref->group_info)->tipo) = INICIALIZACION;

	((carpincho*) lib_ref->group_info)->estado = malloc(sizeof(state));
	*(((carpincho*) lib_ref->group_info)->estado) = NEW;

	((carpincho*) lib_ref->group_info)->PID = malloc(sizeof(pid_t));
	*(((carpincho*) lib_ref->group_info)->PID) = /*getpid() +*/ syscall(SYS_gettid);
	((carpincho*) lib_ref->group_info)->sema = malloc(sizeof(semaforo));
	((carpincho*) lib_ref->group_info)->sema->operacion = malloc(sizeof(operacion));
	((carpincho*) lib_ref->group_info)->sema->nombre_sem = malloc(1);
	((carpincho*) lib_ref->group_info)->sema->longitud = malloc(sizeof(int));
	*(((carpincho*) lib_ref->group_info)->sema->longitud) = 1;
	((carpincho*) lib_ref->group_info)->sema->valor = malloc(sizeof(int));

	((carpincho*) lib_ref->group_info)->nombreIO = malloc(sizeof(mate_io_resourc1));
	((carpincho*) lib_ref->group_info)->nombreIO->nombre = malloc(1);

	((carpincho*) lib_ref->group_info)->nombreIO->longitud = malloc(sizeof(int));
	*(((carpincho*) lib_ref->group_info)->nombreIO->longitud) = 1;

	((carpincho*) lib_ref->group_info)->memoria = malloc(sizeof(mate_memory));
	((carpincho*) lib_ref->group_info)->memoria->size = malloc(sizeof(int));
	*(((carpincho*) lib_ref->group_info)->memoria->size) = 1;
	((carpincho*) lib_ref->group_info)->memoria->addr = malloc(sizeof(mate_pointer));
	((carpincho*) lib_ref->group_info)->memoria->tipoOp = malloc(sizeof(operacion));
	((carpincho*) lib_ref->group_info)->memoria->valor = malloc(1);

	int resultado = serializar_instancia(lib_ref);

	if (resultado == 0) {
		log_info(mate_logger, "PID %i: Se inició la conexión y se creó una instancia", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo crear la instancia", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return resultado;

}

int mate_close(mate_instance *lib_ref)
{
	*(((carpincho*) lib_ref->group_info)->tipo) = FINALIZACION;
	//void* unResultado = serializar_instancia(lib_ref);
	int resultado = serializar_instancia(lib_ref);
	/*int resultado;
	memcpy(&resultado, unResultado, sizeof(int));*/
	if (resultado == 0) {

		close(*(((carpincho*) lib_ref->group_info)->socketConexion));

		log_info(mate_logger, "PID %i: Se cerró una instancia y la conexión", *(((carpincho*) lib_ref->group_info)->PID));
		free(((carpincho*) lib_ref->group_info)->PID);
		free(((carpincho*) lib_ref->group_info)->tipo);
		free(((carpincho*) lib_ref->group_info)->estado);
		free(((carpincho*) lib_ref->group_info)->sema->valor);
		free(((carpincho*) lib_ref->group_info)->sema->operacion);
		free(((carpincho*) lib_ref->group_info)->sema->nombre_sem);
		free(((carpincho*) lib_ref->group_info)->sema->longitud);
		free(((carpincho*) lib_ref->group_info)->sema);
		free(((carpincho*) lib_ref->group_info)->nombreIO->nombre);
		free(((carpincho*) lib_ref->group_info)->nombreIO->longitud);
		free(((carpincho*) lib_ref->group_info)->nombreIO);
		free(((carpincho*) lib_ref->group_info)->memoria->valor);
		free(((carpincho*) lib_ref->group_info)->memoria->addr);
		free(((carpincho*) lib_ref->group_info)->memoria->tipoOp);
		free(((carpincho*) lib_ref->group_info)->memoria->size);
		free(((carpincho*) lib_ref->group_info)->memoria);
		free(((carpincho*) lib_ref->group_info)->ip);
		free(((carpincho*) lib_ref->group_info)->puerto);
		free(((carpincho*) lib_ref->group_info)->socketConexion);
		free((carpincho*) lib_ref->group_info);
		//free(lib_ref);
		//free(&resultado);

		return resultado;
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo cerrar la instancia", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);

		return resultado;
	}

}

//-----------------Semaphore Functions---------------------/

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value) {
	*(((carpincho*) lib_ref->group_info)->tipo) = SEMAFORO;
	*(((carpincho*) lib_ref->group_info)->sema->operacion) = INIT;
	*(((carpincho*) lib_ref->group_info)->sema->valor) = value;

	/*if ((((carpincho*) lib_ref->group_info)->sema->sem_instance = sem_open(sem,O_CREAT,S_IRUSR | S_IWUSR,value)) == SEM_FAILED) {
	  printf("PID %i: Error inicializacion semaforo", *(((carpincho*) lib_ref->group_info)->PID));
	}*/
	((carpincho*) lib_ref->group_info)->sema->nombre_sem = malloc((strlen(sem)+1)*sizeof(char));
	memcpy(((carpincho*) lib_ref->group_info)->sema->nombre_sem, sem, strlen(sem)+1);
	*(((carpincho*) lib_ref->group_info)->sema->longitud) = strlen(((carpincho*) lib_ref->group_info)->sema->nombre_sem) + 1;
	//*(((carpincho*) lib_ref->group_info)->sema->activo) = ACTIVO;

	int resultado = (int) serializar_instancia(lib_ref);

	if (resultado == 0) {
		log_info(mate_logger, "PID %i: Se inició un semáforo", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: Error en la inicialización del semáforo", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return resultado;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem) {

	*(((carpincho*) lib_ref->group_info)->tipo) = SEMAFORO;
	*(((carpincho*) lib_ref->group_info)->sema->operacion) = WAIT;
	*(((carpincho*) lib_ref->group_info)->sema->longitud) = (strlen(sem)+1);

	((carpincho*) lib_ref->group_info)->sema->nombre_sem = malloc((strlen(sem)+1)*sizeof(char));

	memcpy(((carpincho*) lib_ref->group_info)->sema->nombre_sem, sem, strlen(sem)+1);

	//sem_wait(((carpincho*) lib_ref->group_info)->sema->sem_instance);

	int resultado = (int) serializar_instancia(lib_ref);

	if (resultado == 0) {
		log_info(mate_logger, "PID %i: Se cambió el estado del semáforo a wait", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo cambiar el estado del semáforo", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return resultado;

}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem) {

	*(((carpincho*) lib_ref->group_info)->tipo) = SEMAFORO;
	*(((carpincho*) lib_ref->group_info)->sema->operacion) = POST;
	*(((carpincho*) lib_ref->group_info)->sema->longitud) = (strlen(sem)+1);
	///*(((carpincho*) lib_ref->group_info)->sema->operacion) = POST;

	((carpincho*) lib_ref->group_info)->sema->nombre_sem = malloc((strlen(sem)+1)*sizeof(char));

	memcpy(((carpincho*) lib_ref->group_info)->sema->nombre_sem, sem, strlen(sem)+1);

	//sem_post(((carpincho*) lib_ref->group_info)->sema->sem_instance);

	int resultado = (int) serializar_instancia(lib_ref);

	if (resultado == 0) {
		log_info(mate_logger, "PID %i: Se hizo post al semáforo", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo cambiar el estado del semáforo", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return resultado;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem) {

	*(((carpincho*) lib_ref->group_info)->tipo) = SEMAFORO;
	*(((carpincho*) lib_ref->group_info)->sema->operacion) = DESTROY;

	((carpincho*) lib_ref->group_info)->sema->nombre_sem = malloc((strlen(sem)+1)*sizeof(char));
	memcpy(((carpincho*) lib_ref->group_info)->sema->nombre_sem, sem, strlen(sem)+1);
	*(((carpincho*) lib_ref->group_info)->sema->longitud) = (strlen(sem)+1);
	//sem_close(((carpincho*) lib_ref->group_info)->sema->sem_instance);
	//int res = sem_unlink(((carpincho*) lib_ref->group_info)->sema->nombre_sem);
	//*(((carpincho*) lib_ref->group_info)->sema->activo) = INACTIVO;

	int resultado = (int) serializar_instancia(lib_ref);

	if (resultado == 0) {
		log_info(mate_logger, "PID %i: Se destruyó el semáforo", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo destruir el semáforo", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return resultado;
}

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg)
{
	//printf("\nEl nombre de IO es: %s\n", io);


	//(((carpincho*) lib_ref->group_info)->tipo) = malloc(sizeof(type));
	*(((carpincho*) lib_ref->group_info)->tipo) = IO;


	//((carpincho*) lib_ref->group_info)->nombreIO->longitud = malloc(sizeof(int));
	*(((carpincho*) lib_ref->group_info)->nombreIO->longitud) = strlen(io)+1;//*(io->longitud);
	//printf("\nLa longitud es: %i\n", *(((carpincho*) lib_ref->group_info)->nombreIO->longitud));
	//memcpy(((carpincho*) lib_ref->group_info)->nombreIO->longitud,io->longitud,sizeof(int));
	//free(((carpincho*) lib_ref->group_info)->nombreIO->nombre);
	//((carpincho*) lib_ref->group_info)->nombreIO->nombre = malloc(*(((carpincho*) lib_ref->group_info)->nombreIO->longitud));
	((carpincho*) lib_ref->group_info)->nombreIO->nombre = malloc(*(((carpincho*) lib_ref->group_info)->nombreIO->longitud));
	memcpy(((carpincho*) lib_ref->group_info)->nombreIO->nombre,io,*(((carpincho*) lib_ref->group_info)->nombreIO->longitud));

	/*printf("\nEl nombre de IO es: %s\n", io);
	fflush(stdout);
	printf("\nEl nombre de IO es: %s\n", ((carpincho*) lib_ref->group_info)->nombreIO->nombre);
	printf("\nLa longitud es: %i\n", *(((carpincho*) lib_ref->group_info)->nombreIO->longitud));
*/
	//((carpincho*) lib_ref->group_info)->nombreIO->nombre = io->nombre;

	//printf("\nLargo: %i\n", *(lib_ref->nombreIO->longitud));
	//printf("\nNombre: %s\n", lib_ref->nombreIO->nombre);

	int resultado = (int) serializar_instancia(lib_ref);

	if (resultado == 0) {
		log_info(mate_logger, "PID %i: Se llamó a un dispositivo IO", *(((carpincho*) lib_ref->group_info)->PID));
		log_info(mate_logger, "PID %i: El mensaje es: %s", *(((carpincho*) lib_ref->group_info)->PID), msg);
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo llamar al dispositivo IO", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return resultado;
}

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size)
{
	//((mate_inner_structure *)lib_ref->group_info)->memory = malloc(size);

	*(((carpincho*) lib_ref->group_info)->tipo) = MEMORIA;

	*(((carpincho*) lib_ref->group_info)->memoria->size) = size;
	*(((carpincho*) lib_ref->group_info)->memoria->tipoOp) = MEMALLOC;

	void* resultado = serializar_instancia(lib_ref);
	printf("\nEl valor del resultado es: %s\n", resultado);
	mate_pointer unResultado;
	memcpy(&unResultado, resultado, sizeof(int32_t));
	printf("\nEl valor del resultado es: %i\n", unResultado);
	//mate_pointer resultado = (mate_pointer) serializar_instancia(lib_ref);

	if (resultado != NULL) {
		log_info(mate_logger, "PID %i: Se quiere hacer un memalloc", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo hacer un memalloc", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return unResultado;
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr)
{

	*(((carpincho*) lib_ref->group_info)->tipo) = MEMORIA;

	*(((carpincho*) lib_ref->group_info)->memoria->addr) = addr;
	*(((carpincho*) lib_ref->group_info)->memoria->tipoOp) = MEMFREE;

	int resultado = serializar_instancia(lib_ref);

	if (resultado == 0) {
		log_info(mate_logger, "PID %i: Se quiere hacer un memfree", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo hacer un memfree", *(((carpincho*) lib_ref->group_info)->PID));
		//free(&resultado);
	}

	return resultado;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size)
{

	*(((carpincho*) lib_ref->group_info)->tipo) = MEMORIA;

	*(((carpincho*) lib_ref->group_info)->memoria->addr) = origin;
	*(((carpincho*) lib_ref->group_info)->memoria->size) = size;
	*(((carpincho*) lib_ref->group_info)->memoria->tipoOp) = MEMREAD;

	void* buffer = serializar_instancia(lib_ref);
	int tamanio = -1;

	printf("\nAntes del printf\n");
	printf("\nEl buffer es de %s\n", buffer);

	memcpy(&tamanio, buffer, sizeof(int));

	if (tamanio > 0) {
		//COMENTAR EL DEST DESPUES
		dest = malloc(tamanio);
		memcpy(dest, buffer+sizeof(int), tamanio);
		log_info(mate_logger, "PID %i: Se quiere hacer un memread", *(((carpincho*) lib_ref->group_info)->PID));
		free(buffer);
		//COMENTAR ESTO TAMBIEN DESPUEES
		free(dest);
		return 0;
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo hacer un memread", *(((carpincho*) lib_ref->group_info)->PID));
		free(buffer);
		return -1;
	}
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size)
{

	*(((carpincho*) lib_ref->group_info)->tipo) = MEMORIA;
	*(((carpincho*) lib_ref->group_info)->memoria->addr) = dest;
	*(((carpincho*) lib_ref->group_info)->memoria->size) = size;
	*(((carpincho*) lib_ref->group_info)->memoria->tipoOp) = MEMWRITE;

	free(((carpincho*) lib_ref->group_info)->memoria->valor);

	((carpincho*) lib_ref->group_info)->memoria->valor = malloc(size);

	memcpy(((carpincho*) lib_ref->group_info)->memoria->valor, origin, size);

	mate_pointer* buffer = (mate_pointer) serializar_instancia(lib_ref);
	memcpy(&dest, buffer, sizeof(mate_pointer));
	free(buffer);

	if (dest != NULL) {
		log_info(mate_logger, "PID %i: Se quiere hacer un memwrite", *(((carpincho*) lib_ref->group_info)->PID));
		//free(buffer);
		return 0;
	}
	else {
		log_info(mate_logger, "PID %i: No se pudo hacer un memwrite", *(((carpincho*) lib_ref->group_info)->PID));
		//free(buffer);
		return -1;
	}

}

void* serializar_instancia(mate_instance *lib_ref) {

	int tamanio = *(((carpincho*) lib_ref->group_info)->nombreIO->longitud)*sizeof(char) + *(((carpincho*) lib_ref->group_info)->memoria->size) +
			sizeof(pid_t) + 6*sizeof(int) + sizeof(state) + sizeof(mate_pointer) + sizeof(operacion) + sizeof(codOperacion) +
			sizeof(type) + sizeof(operation) + *(((carpincho*) lib_ref->group_info)->sema->longitud)*sizeof(char);

	codOperacion tipoMensaje = PROCESS;

	void * magic = malloc(tamanio);
	int desplazamiento = 0;

	memcpy(magic+desplazamiento,&tipoMensaje,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->tipo,sizeof(type));
	desplazamiento+= sizeof(type);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->sema->operacion,sizeof(operation));
	desplazamiento+= sizeof(operation);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->sema->longitud,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->sema->valor,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->sema->nombre_sem,*(((carpincho*) lib_ref->group_info)->sema->longitud));
	desplazamiento+= *(((carpincho*) lib_ref->group_info)->sema->longitud);

	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->PID,sizeof(pid_t));
	desplazamiento+= sizeof(pid_t);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->estado,sizeof(state));
	desplazamiento+= sizeof(int);

	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->memoria->size,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->memoria->valor,*(((carpincho*) lib_ref->group_info)->memoria->size));
	desplazamiento+= *(((carpincho*) lib_ref->group_info)->memoria->size);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->memoria->addr,sizeof(mate_pointer));
	desplazamiento+= sizeof(mate_pointer);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->memoria->tipoOp,sizeof(operacion));
	desplazamiento+= sizeof(operacion);

	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->nombreIO->longitud,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,((carpincho*) lib_ref->group_info)->nombreIO->nombre,*(((carpincho*) lib_ref->group_info)->nombreIO->longitud));
	desplazamiento+= *(((carpincho*) lib_ref->group_info)->nombreIO->longitud);

	log_info(mate_logger, "PID %i: Se mandó un paquete", *(((carpincho*) lib_ref->group_info)->PID));

	send(*(((carpincho*) lib_ref->group_info)->socketConexion), magic, desplazamiento, 0);

	//int resultado = -1;
	free(magic);

	void* result;
	int unTamanio;

	if(*(((carpincho*) lib_ref->group_info)->tipo) == MEMORIA) {
		switch (*(((carpincho*) lib_ref->group_info)->memoria->tipoOp)) {
		case MEMALLOC:
			result = malloc(sizeof(mate_pointer));
			recv(*(((carpincho*) lib_ref->group_info)->socketConexion), result, sizeof(mate_pointer), MSG_WAITALL);
			break;
		case MEMFREE:
			result = malloc(sizeof(int));
			recv(*(((carpincho*) lib_ref->group_info)->socketConexion), result, sizeof(int), MSG_WAITALL);
			break;
		case MEMREAD:
			printf("\nAntes del primer recv\n");
			recv(*(((carpincho*) lib_ref->group_info)->socketConexion), &unTamanio, sizeof(int), 0);
			printf("\nDespues del primer recv\n");
			result = malloc(unTamanio+sizeof(int));
			memcpy(result, &unTamanio, sizeof(int));
			printf("\nEl valor del tamanio es: %i\n", unTamanio);
			int desplaza = sizeof(int);
			printf("\nAntes del segundo recv\n");
			recv(*(((carpincho*) lib_ref->group_info)->socketConexion), result+desplaza, unTamanio, MSG_WAITALL);
			printf("\nDespues del segundo recv\n");
			break;
		case MEMWRITE:
			result = malloc(sizeof(mate_pointer));
			recv(*(((carpincho*) lib_ref->group_info)->socketConexion), result, sizeof(mate_pointer), MSG_WAITALL);
			break;
		default:
			result = malloc(sizeof(int));
			recv(*(((carpincho*) lib_ref->group_info)->socketConexion), result, sizeof(int), MSG_WAITALL);
			break;
		}
	}
	else {
		int resultado;
		//result = malloc(sizeof(int));
		recv(*(((carpincho*) lib_ref->group_info)->socketConexion), &resultado, sizeof(int), 0);
		return (void*) resultado;
		//TODO chequear
	}

	//close(*(((carpincho*) lib_ref->group_info)->socketConexion));

	return result;
}

/*void serializar_mensaje(mate_instance *lib_ref, char* mensaje) {
	int tamanio;

	tamanio = strlen(mensaje) + 1 + sizeof(int) + sizeof(codOperacion);
	codOperacion tipoMensaje = MESSAGE;

	void * magic = malloc(tamanio);
	int desplazamiento = 0;

	memcpy(magic+desplazamiento,&tipoMensaje,sizeof(int));
	desplazamiento+= sizeof(codOperacion);
	int longitud = strlen(mensaje) + 1;
	memcpy(magic+desplazamiento,&longitud,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic+desplazamiento,mensaje,strlen(mensaje)+1);
	desplazamiento+= strlen(mensaje)+1;

	log_info(mate_logger, "PID %i: Se mandó un mensaje", *(((carpincho*) lib_ref->group_info)->PID));

	//send(*(lib_ref->socket), magic, desplazamiento, 0);

	free(magic);

}*/

t_log* mate_iniciar_logger(void)
{
	t_log* nuevo_logger;
	if((nuevo_logger = log_create("matelib.log", "matelib", 1, LOG_LEVEL_INFO)) == NULL) {
		printf("No pude crear el logger\n");
		exit(1);
	}
	return nuevo_logger;
}
