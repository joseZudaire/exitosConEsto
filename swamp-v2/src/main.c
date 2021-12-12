#include <stdio.h>
#include <stdlib.h>
#include "swamp.h"

int main() {
	read_and_log_config();
	iniciar_logger();
	log_debug(logger,"Iniciando servidor");

	iniciar_servidor();

	free(datosConfigSwamp);
	log_destroy(logger);

	return EXIT_SUCCESS;
}
