#include "client.h"

int main() {
	t_config_client* datosConfigClient;
    t_log* logger;
	datosConfigClient = read_and_log_config();
	iniciar_logger();
	log_debug(logger,"Iniciando client");


    // envio mensaje 2
    printf("\n Envio mensaje de una estructura \n");
	t_paquete* structurePaquete;
	// serializar paquete
	t_buffer* buffer = serializar_cuatro_parametros("nombre", (uint32_t) 2, (uint32_t) 1, (uint32_t) 2);
    enviar_paquete(buffer, MENSAJE, datosConfigClient->ip_server, datosConfigClient->puerto_server);
    printf("\n Fin mensaje de una estructura \n");


    // envio mensaje 2
    printf("\n Envio mensaje de una estructura \n");
	t_paquete* structurePaquete2;
	// serializar paquete
	t_buffer* buffer2 = serializar_cuatro_parametros("nombre", (uint32_t) 2, (uint32_t) 1, (uint32_t) 2);
    enviar_paquete(buffer2, MENSAJE, datosConfigClient->ip_server, datosConfigClient->puerto_server);
    printf("\n Fin mensaje de una estructura \n");



	//free(datosConfigClient);
	log_destroy(logger);

	return EXIT_SUCCESS;
}
