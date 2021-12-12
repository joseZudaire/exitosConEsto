#include "signals.h"

void sig_handler_sigint(int signo)
{
	/*
		Cantidad de TLB Hit totales
		Cantidad de TLB Hit por carpincho indicando carpincho y cantidad.
		Cantidad de TLB Miss totales.
		Cantidad de TLB Miss por carpincho indicando carpincho y cantidad.
	*/
	log_info(logger, "--------------------signal sigint--------------------");
	log_info(logger, "Cantidad de hits TLB Totales: %d", tlb_hits_totales);
	log_info(logger, "Cantidad de miss TLB Totales: %d", tlb_miss_totales);
	
	if (datosConfigMemoria->debug) 
		printf("cantidad de carpinchos: %d\n", list_size(carpinchos_list));

	for(int i=0;i < list_size(carpinchos_list); i++)
	{
		t_carpincho* carpincho = list_get(carpinchos_list, i);
		log_info(logger, "------------------------------------");
		log_info(logger, "Carpincho pid: %d", carpincho->pid);
		log_info(logger, "Cantidad hits carpincho: %d", carpincho->tlb_hits);
		log_info(logger, "Cantidad miss carpincho: %d", carpincho->tlb_miss);
	}

	log_info(logger, "--------------------signal sigint--------------------");

	exit(EXIT_SUCCESS);
}


void sig_handler_sig1(int signo)
{
	log_info(logger, "signal sig1");
	/*
	 * Generar un dump del contenido de la TLB en un archivo de texto,
	 * el cual deberá encontrarse dentro del path indicado por archivo de configuración
	 * y deberá llamarse Dump_<Timestamp>.tlb
	 * */
	dump();
}

void sig_handler_sig2(int signo)
{
	log_info(logger, "signal sig2");
	//deberá limpiar todas las entradas de la TLB.
	clear_mensajes_dump();
	//Limpiar tlb
}

void register_signals()
{
	signal(SIGINT, sig_handler_sigint);

	signal(SIGUSR1, sig_handler_sig1);

	signal(SIGUSR2, sig_handler_sig2);
}

void increment_hit(t_carpincho* carpincho)
{
	tlb_hits_totales = tlb_hits_totales + 1;
	carpincho->tlb_hits = carpincho->tlb_hits + 1;
	if (datosConfigMemoria->debug) {
		printf("Cantidad total de hits: %d\n", tlb_hits_totales);
		printf("Cantidad de hits carpincho: %d\n", carpincho->tlb_hits);
	}
}

void increment_miss(t_carpincho* carpincho)
{
	tlb_miss_totales = tlb_miss_totales + 1;
	carpincho->tlb_miss = carpincho->tlb_miss + 1;
	if (datosConfigMemoria->debug) {
		printf("Cantidad total de miss: %d\n", tlb_miss_totales);
		printf("Cantidad de miss carpincho: %d\n", carpincho->tlb_miss);
	}
}


char* retornarFechaHora (){

	char* output = malloc(128);

    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    strftime(output,128,"%d-%m-%y %H:%M:%S",tlocal);

    return output;
}


void dump (){
	log_info(logger,"Se solicito un dump de la cache");
	char* dir = string_new();
	string_append(&dir,datosConfigMemoria->path_dump_tlb);
	string_append(&dir,"/");
	char* fechaHora = retornarFechaHora();
	string_append(&fechaHora,".tlb");
	char* nombreArchivo = string_new();
	string_append(&nombreArchivo,"Dump_");
	string_append(&nombreArchivo,fechaHora);
	string_append(&dir,nombreArchivo);
	printf("\n LA DIR ES: %s \n\n", dir);
	log_info(logger,"El nombre del archivo es: %s ", dir);

	FILE * file_dump = fopen(dir,"w+");
	fprintf(file_dump,"----------------------------------------------------------\n");
	fprintf(file_dump,"Dump: %s \n", fechaHora);

	for(int i=0;i < list_size(mensajes_dump_list); i++)
	{
		t_mensaje_dump* mensaje = list_get(mensajes_dump_list, i);
		fprintf(file_dump,"Entrada: %d	", mensaje->entrada);
		if (mensaje->estado == 1) {
			fprintf(file_dump,"Estado: Ocupado	");
			fprintf(file_dump,"Carpincho: %d	", mensaje->carpincho_pid);
			fprintf(file_dump,"Pagina: %d	", mensaje->pagina);
			fprintf(file_dump,"Marco: %d\n", mensaje->marco);
		}
		if (mensaje->estado == 0) {
			fprintf(file_dump,"Estado: Libre  	");
			fprintf(file_dump,"Carpincho: -	");
			fprintf(file_dump,"Pagina: -	");
			fprintf(file_dump,"Marco: -\n");
		}

		if (datosConfigMemoria->debug) {
			t_tlb* tlb = list_get(tlb_list,i);
			printf("---------------------------\n");
			printf("Entrada: %d	", tlb->entrada);
			if (tlb->pid != NULL) {
				printf("Estado: Ocupado	  ");
				printf("Carpincho: %d	", tlb->pid);
				printf("Pagina: %d	", tlb->pagina);
				printf("Marco: %d\n", tlb->marco);
			} else {
				printf("Estado: Libre  	  ");
				printf("Carpincho: -	");
				printf("Pagina: -	");
				printf("Marco: -\n");
			}
		}
	}

	fprintf(file_dump,"----------------------------------------------------------\n");

	fclose(file_dump);
	log_debug(logger, "El dump de la cache ha sido realizado");

	free(dir);
	free(nombreArchivo);
	free(fechaHora);
}

