#include "signals.h"
#include "memory.h"
#include "server.h"
#include "client.h"
#include "lib.h"
#include "tests.h"

int main()
{
	read_and_log_config();

	iniciar_logger(datosConfigMemoria->debug);
	log_info(logger, "Iniciando memoria");
	inicializarMemoria();

	socket_swamp = crear_conexion_swamp(datosConfigMemoria->ip_swap, datosConfigMemoria->puerto_swap);
	if((socket_swamp == -1) && (datosConfigMemoria->debug)){
		log_info(logger,_RED "\nNo se pudo conectar con swamp.\n"_RESET);
		log_info(logger,_RED "\nIP: %s\n"_RESET, datosConfigMemoria->ip_swap);
		log_info(logger,_RED "\nPuerto: %s\n"_RESET, datosConfigMemoria->puerto_swap);
	} else {
		int valorRetorno = enviar_tipo_asignacion();
		if (datosConfigMemoria->debug)
			log_info(logger,"El valor de retorno es: %i", valorRetorno);
	}

	log_info(logger, "Registrando manejadores de signals");
	register_signals();

//	log_info(logger, "El valor de retorno de la escritura es: %i", enviar_escritura_swap(15, 1, "hola como va "));

	iniciar_servidor();
	printf("\n--------------------------------\n");
	imprimir_marcos();
	printf("\n--------------------------------\n");

	close_memory();

	return EXIT_SUCCESS;
}