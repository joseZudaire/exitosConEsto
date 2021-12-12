#include "memory.h"

void inicializarMemoria() {
	log_info(logger,"[ MEMORIA ] Se inicializa memoria. Por favor no la rompas! =)");
	CACHE = malloc(datosConfigMemoria->tamanio);
	memset(CACHE, '_', datosConfigMemoria->tamanio);

	cantidad_maxima_marcos =
		datosConfigMemoria->tamanio / datosConfigMemoria->tamanio_pagina;

	pthread_mutex_init(&mutex_lectura_escritura,NULL);
	pthread_mutex_init(&mutex_memalloc,NULL);

	contador_timestamp = 0;
	contador_timestamp_tlb = 0;
	contador_carpinchos = 0;

	algoritm_stack_pointer = 0;

    tlb_hits_totales = 0;
    tlb_miss_totales = 0;

	paginas_en_cache_list = list_create();
	paginas_en_tlb_list = list_create();
	carpinchos_list = list_create();
	marcos_list = list_create();
	mensajes_dump_list = list_create();

	create_marcos(CACHE);
	create_tlb();
	tlb_fifo_pointer = 0;
}

void close_memory() {
	log_info(logger,"[ MEMORIA ] Se cierra memoria. Adios! =)");
	close_mensajes_dump();
	free_tlb();
	close_marcos();
	free(datosConfigMemoria);

	log_destroy(logger);
}

/*
	Semaforo de lectura y escritura
*/

void luz_roja() {
	pthread_mutex_lock(&mutex_lectura_escritura);
	if (datosConfigMemoria->debug) 
		printf("\n Se bloqueo el semaforo de lectura/escritura"_RED "LUZ ROJA\n"_RESET);
	log_info(logger,"[ SEMAFORO ] Se bloqueo el semaforo de comunicación con Swamp espere LUZ VERDE para continuar");
}

void luz_verde() {
	pthread_mutex_unlock(&mutex_lectura_escritura);
	if (datosConfigMemoria->debug) 
		printf("\n Se libero el semaforo de lectura/escritura"_GREEN "LUZ VERDE\n"_RESET);
	log_info(logger,"[ SEMAFORO ] Se desbloqueo el semaforo de comunicación con Swamp a espera de una nueva tarea");
}

void luz_roja_memalloc() {
	pthread_mutex_lock(&mutex_memalloc);
	if (datosConfigMemoria->debug) 
		printf("\n Se bloqueo el semaforo de MEMALLOC"_RED "LUZ ROJA\n"_RESET);
	log_info(logger,"[ SEMAFORO MEMALLOC ] Se bloqueo el semaforo de comunicación con Swamp espere LUZ VERDE para continuar");
}

void luz_verde_memalloc() {
	pthread_mutex_unlock(&mutex_memalloc);
	if (datosConfigMemoria->debug) 
		printf("\n Se libero el semaforo de MEMALLOC"_GREEN "LUZ VERDE\n"_RESET);
	log_info(logger,"[ SEMAFORO MEMALLOC ] Se desbloqueo el semaforo de comunicación con Swamp a espera de una nueva tarea");
}

/*
	////////////       //\\       ///////    ////////   //	////	   //  //////////  //	  //  //////////
	//                //  \\      //	//   //     //  //	// //      //  //	       //	  //  //      //
	//               //    \\     //     //  //     //  //	//   //    //  //    	   //	  //  //      //
	//              //      \\    //	//   //    //   //	//    //   //  //   	   // 	  //  //      //
	//             //////////\\   //////     ///////    //	//     //  //  //    	   /////////  //      //
	//            //	   	  \\  //	//   //         //	//      // //  //    	   //	  //  //      //
	//           //			   \\ //	 //  //         //	//       ////  //	       //	  //  //      //
	//////////////			    \\//     //  //         //	//		  ///  //////////  //	  //  //////////
			carpinchos functions
*/


bool exist_carpincho(int pid) {
	bool same_pid(t_carpincho* carpincho) {
		return carpincho->pid == pid;
	}
	t_list* carpinchos = list_filter(carpinchos_list, (void*) same_pid);
	return list_size(carpinchos);
}

t_carpincho* get_carpincho(int pid) {
	bool same_pid(t_carpincho* carpincho) {
		return carpincho->pid == pid;
	}

	t_list* carpinchos = list_filter(carpinchos_list, (void*) same_pid);

	t_carpincho* carp = NULL;

	if(list_size(carpinchos) != 0)
		carp = list_get(carpinchos, 0);

	return carp;
}

void asignar_paginas_carpincho(t_carpincho* carpincho, int cantPag)
{
	//Tenemos que crear x cantidad de paginas a los carpinchos, y alocarlas al final y conseguir marco para cada una.
	for(int i = 1; i <= cantPag; i++)
	{
			//Creo las paginas, para eso necesito por cada pagina pedir un marco
		t_pagina* pag = create_pagina(carpincho);
	}
}


//funcion creada para testear
t_carpincho* create_carpincho(int pid) {
	if (exist_carpincho(pid)) {
		if (datosConfigMemoria->debug)
			printf("\n El carpincho %d ya existe\n", pid);
		log_info(logger, "[ CARPINCHO ] El carpincho %d ya existe, no se crea, solo se retorna.", pid);
		return get_carpincho(pid);
	}

	t_carpincho* carpincho = malloc(sizeof(t_carpincho));

	carpincho->pid = pid;
	carpincho->tlb_hits = 0;
	carpincho->tlb_miss = 0;
	carpincho->algoritm_stack_pointer = 0;
	carpincho->numero = contador_carpinchos +1;
	carpincho->allocs = list_create();
	carpincho->paginas = list_create();
	list_add(carpinchos_list, carpincho);
	log_info(logger, "[ CARPINCHO ] Se creo con exito el carpincho: %d", pid);
	if (datosConfigMemoria->debug)
		printf("\n Se creo con exito el carpincho %d\n", pid);
	contador_carpinchos++;
	return carpincho;
}

void close_carpincho(t_carpincho* carpincho)
 {
	free_pages_by_carpincho(carpincho);
	free_allocs_by_carpincho(carpincho);
	free(carpincho);
}

void close_carpinchos() {
	list_iterate(carpinchos_list, (void*) free_pages_by_carpincho);
	list_iterate(carpinchos_list, (void*) free);
}

void limpiar_conteos_tlb_de_carpincho(t_carpincho* carpincho) {
	carpincho->tlb_hits = 0;
	carpincho->tlb_miss = 0;
}

/*
	//\\      ////       //\\       ///////    ////////// //////////  /////////
	// \\    // //      //  \\      //	  //   //         //	  //  //
	//  \\  //  //     //    \\     //    //   //         //	  //  //
	//   \\//   //    //      \\    //	 //    //         //	  //  //
	//          //   //////////\\   //////     //         //	  //  /////////
	//          //  //			\\  //	 //    //         //	  //		 //
	//          // //			 \\ //	  //   //         //	  //		 //
	//          ////			  \\//     //  ////////// //////////  /////////
					marcos functions
*/

void dump_marcos(){
	t_list* marcos = marcos_list;
	char* dir = string_new();
	string_append(&dir,"/home/utnso/tp-2021-2c-SO_ftw/memoria/src/marcos");
	string_append(&dir,"/");
	char* fechaHora = retornarFechaHora();
	string_append(&fechaHora,".tlb");
	char* nombreArchivo = string_new();
	string_append(&nombreArchivo,"Dump_");
	string_append(&nombreArchivo,fechaHora);
	string_append(&dir,nombreArchivo);
	printf("\n LA DIR ES: %s \n\n", dir);
	FILE * file_dump = fopen(dir,"w+");
	fprintf(file_dump,"----------------------------------------------------------\n");
	fprintf(file_dump,"----------------------------------------------------------\n");
	fprintf(file_dump,"Dump: %s \n", fechaHora);
		bool sorted_by_marco_number(t_marco * marco1,t_marco * marco2){
			return (marco1->marco) < (marco2->marco);
		}
		list_sort(marcos, (void*) sorted_by_marco_number);

		char* end_pages[12];
		printf("\n%s\n",datosConfigMemoria->algoritmo_reemplazo_mmu);
		if (list_size(marcos) == 12 && strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "LRU")==0) {
			end_pages[0] = "C1P0";
			end_pages[1] = "C1P1";
			end_pages[2] = "C2P0";
			end_pages[3] = "C1P4";
			end_pages[4] = "C3P0";
			end_pages[5] = "C1P7";
			end_pages[6] = "C1P2";
			end_pages[7] = "C1P5";
			end_pages[8] = "C3P2";
			end_pages[9] = "C1P3";
			end_pages[10] = "C1P6";
			end_pages[11] = "C3P3";
		}
		if (list_size(marcos) == 12 && strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0)  {
			end_pages[0] = "C1P4";
			end_pages[1] = "C1P0";
			end_pages[2] = "C1P1";
			end_pages[3] = "C1P5";
			end_pages[4] = "C1P6";
			end_pages[5] = "C1P7";
			end_pages[6] = "C1P2";
			end_pages[7] = "C1P8";
			end_pages[8] = "C2P1";
			end_pages[9] = "C1P3";
			end_pages[10] = "C3P0";
			end_pages[11] = "C3P1";
		}

		for(int i=0;i < list_size(marcos); i++) {
			t_marco* marco = list_get(marcos,i);
			fprintf(file_dump,"\n---------------------------\n");
			fprintf(file_dump,"marco: %d	", marco->marco);
			if (!marco->isFree) {
				fprintf(file_dump,"IsFree: Ocupado	  ");
				fprintf(file_dump,"Carpincho: %d	", marco->pid);
				fprintf(file_dump,"Pagina %d ", marco->pagina -1);
				t_carpincho* carpincho = get_carpincho(marco->pid);
				fprintf(file_dump,"C%d", carpincho->numero);
				fprintf(file_dump,"P%d	", marco->pagina -1);
				if (list_size(marcos) == 12 )
					fprintf(file_dump,"Deberia ser: %s  ", (char*) end_pages[i]);
				fprintf(file_dump,"Marco: %d ", marco->marco);
				if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "LRU")==0) {
					t_pagina* pagina = get_page_by_ints(marco->pid, marco->pagina);
					fprintf(file_dump," timestamp %d \n",pagina->timestamp);
				} else {
					t_pagina* pagina = get_page_by_ints(marco->pid, marco->pagina);
					fprintf(file_dump," bits: ");
					if (pagina->bit_presencia) fprintf(file_dump,"P");
					else fprintf(file_dump,"-");
					if (pagina->bit_de_uso) fprintf(file_dump,"U");
					else fprintf(file_dump,"-");
					if (pagina->bit_de_modificacion) fprintf(file_dump,"M \n");
					else fprintf(file_dump,"- \n");
				}
			} else {
				fprintf(file_dump,"IsFree: Libre ");
				fprintf(file_dump,"Carpincho: %d ", marco->pid);
				fprintf(file_dump,"Pagina: %d ", marco->pagina-1);
				if (list_size(marcos) == 12 )
					fprintf(file_dump,"Deberia ser: %s  ", (char*) end_pages[i]);
				fprintf(file_dump,"Marco: %d\n", marco->marco);
			}
		}
		fprintf(file_dump,"\n---------------------------\n");

	fprintf(file_dump,"----------------------------------------------------------\n");
	fprintf(file_dump,"----------------------------------------------------------\n");

	fclose(file_dump);

	free(dir);
	free(nombreArchivo);
	free(fechaHora);
}

void imprimir_marcos() {
	imprimir_lista_de_marcos(marcos_list);
}

void imprimir_lista_de_marcos(t_list* marcos) {
	if (datosConfigMemoria->debug) {
		//dump_marcos(marcos);
		bool sorted_by_marco_number(t_marco * marco1,t_marco * marco2){
			return (marco1->marco) < (marco2->marco);
		}
		list_sort(marcos, (void*) sorted_by_marco_number);
		printf("\n Imprimo lista de marcos \n\n");
		char* end_pages[12];
		printf("\n%s\n",datosConfigMemoria->algoritmo_reemplazo_mmu);
		if (list_size(marcos) == 12 && strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "LRU")==0) {
			end_pages[0] = "C1P0";
			printf("\n\n %s \n\n",end_pages[0]);
			end_pages[1] = "C1P1";
			end_pages[2] = "C2P0";
			end_pages[3] = "C1P4";
			end_pages[4] = "C3P0";
			end_pages[5] = "C1P7";
			end_pages[6] = "C1P2";
			end_pages[7] = "C1P5";
			end_pages[8] = "C3P2";
			end_pages[9] = "C1P3";
			end_pages[10] = "C1P6";
			end_pages[11] = "C3P3";
		}
		if (list_size(marcos) == 12 && strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0)  {
			end_pages[0] = "C1P4";
			printf("\n\n %s \n\n",end_pages[0]);
			end_pages[1] = "C1P0";
			end_pages[2] = "C1P1";
			end_pages[3] = "C1P5";
			end_pages[4] = "C1P6";
			end_pages[5] = "C1P7";
			end_pages[6] = "C1P2";
			end_pages[7] = "C1P8";
			end_pages[8] = "C2P1";
			end_pages[9] = "C1P3";
			end_pages[10] = "C3P0";
			end_pages[11] = "C3P1";
		}

		for(int i=0;i < list_size(marcos); i++) {
			t_marco* marco = list_get(marcos,i);
			printf("\n---------------------------\n");
			printf("marco: %d ", marco->marco);
			if (!marco->isFree) {
				printf("IsFree: Ocupado ");
				printf("pid: %d	", marco->pid);
				printf("Pagina %d ", marco->pagina -1);
				t_carpincho* carpincho = get_carpincho(marco->pid);
				printf("C%d", carpincho->numero);
				printf("P%d	", marco->pagina -1);
				if (list_size(marcos) == 12 )
					printf("Deberia ser: %s  ", (char*) end_pages[i]);
				printf("Marco: %d ", marco->marco);
				if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "LRU")==0) {
					t_pagina* pagina = get_page_by_ints(marco->pid, marco->pagina);
					printf(" timestamp %d \n",pagina->timestamp);
				} else {
					t_pagina* pagina = get_page_by_ints(marco->pid, marco->pagina);
					printf(" bits: ");
					if (pagina->bit_presencia) printf(_GREEN"P"_RESET);
					else printf(_RED"-");
					if (pagina->bit_de_uso) printf(_GREEN"U"_RESET);
					else printf(_RED"-");
					if (pagina->bit_de_modificacion) printf(_GREEN"M\n"_RESET);
					else printf(_RED"-\n");
				}
			} else {
				printf("IsFree: Libre ");
				printf("Carpincho: %d ", marco->pid);
				printf("Pagina: %d ", marco->pagina-1);
				if (list_size(marcos) == 12 )
					printf("Deberia ser: %s  ", (char*) end_pages[i]);
				printf("Marco: %d\n", marco->marco);
			}
		}
		printf("\n---------------------------\n");
	}
}

t_marco* get_marco(int carpincho, int pagina) {
	bool same_pids(t_marco* marco1) {
		return marco1->pid == carpincho && marco1->pagina == pagina;
	}
	if (list_size(list_filter(marcos_list, (void*) same_pids))>1) {
		if (datosConfigMemoria->debug) {
			printf(_RED "\n\n El carpincho %d pagina %d se encuentra duplicada en CACHE\n"_RESET, carpincho, pagina);
		}
		log_error(logger,"[ ERROR MARCO ] El carpincho %d pagina %d se encuentra duplicada en CACHE", carpincho, pagina);
	}
	return list_get(list_filter(marcos_list, (void*) same_pids), 0);
}

int get_amount_marcos() {
	return datosConfigMemoria->tamanio / datosConfigMemoria->tamanio_pagina;
}

void create_marcos() {
	int marco_number = 0;
	int cant_marcos = get_amount_marcos();

	while (marco_number != cant_marcos) {
		t_marco* marco = malloc(sizeof(t_marco));
		marco->marco = marco_number;
		marco->isFree = TRUE;
		marco->pagina = NULL;
		marco->pid = NULL;
		marco_number++;

		list_add(marcos_list,marco);
	}

	log_info(logger,"[ MARCO ] Cantidad marcos: %d \n", cant_marcos);
	if (datosConfigMemoria->debug ) {
		printf("\n cantidad marcos: %d \n", cant_marcos);
		printf("\n cantidad marcos en la lista: %d\n", list_size(marcos_list));
	}

}

int amount_frame_used_by_carpincho(t_carpincho* carpincho) {
	bool iUseThisFrame(t_marco * marco){
		return marco->pid == carpincho->pid;
	}
	t_list* lista = list_filter(marcos_list, (void*) iUseThisFrame);

	return list_size(lista);
}

int amount_frame_used_by_carpincho_pid(int carpincho) {
	bool iUseThisFrame(t_marco * marco){
		return marco->pid == carpincho;
	}
	t_list* lista = list_filter(marcos_list, (void*) iUseThisFrame);

	return list_size(lista);
}

void close_marcos() {
	list_iterate(marcos_list, (void*) free);
	list_destroy(marcos_list);
}

uint32_t get_marco_ptr(int marco) {
	return (uint32_t) CACHE + marco * datosConfigMemoria->tamanio_pagina;
}

bool any_marco_free() {
	bool marcoIsFree(t_marco * marco){
		return marco->isFree;
	}
	t_list* lista = list_filter(marcos_list, (void*) marcoIsFree);

	return list_size(lista) != 0;
}

t_marco* get_free_marco() {
	//Filtro por isFree
	bool marcoIsFree(t_marco * marco){
		return marco->isFree;
	}
	t_list* lista = list_filter(marcos_list, (void*) marcoIsFree);
	//Ordeno la lista
	bool sorted_by_number(t_marco * marco1, t_marco * marco2) {
		return (marco1->marco) < (marco2->marco);
	}

	list_sort(lista, (void*) sorted_by_number);
	t_marco* marco = list_get(lista,0);

	return marco;
}

int obtener_marco_de_tlb(int carpincho, int pagina) {
	bool isPageFrame(t_tlb * tlb){
		return tlb->pid == carpincho && tlb->pagina == pagina;
	}
	t_list* lista = list_filter(tlb_list, (void*) isPageFrame);
	if (list_size(lista)>0) {
		t_tlb* tlb = list_get(lista,0);
		if (datosConfigMemoria->debug)
			printf(_GREEN "\n\n El carpincho %d pagina %d se encuentran en la tlb en el marco %d\n"_RESET, carpincho, pagina, tlb->marco);
		log_info(logger,"[ MARCO ] El carpincho %d pagina %d se encuentran en la tlb en el marco %d", carpincho, pagina, tlb->marco);
		return tlb->marco;
	}

	if (datosConfigMemoria->debug) {
		printf(_RED "\n\n El carpincho %d pagina %d no se encuentran en la tlb\n"_RESET, carpincho, pagina);
	}
	log_error(logger,"[ ERROR MARCO ] El carpincho %d pagina %d no se encuentran en la tlb", carpincho, pagina);
	return -1;
}

int obtener_marco_de_cache(int carpincho, int pagina) {
	bool isPageFrame(t_marco * marco){
		return marco->pid == carpincho && marco->pagina == pagina;
	}
	t_list* lista = list_filter(marcos_list, (void*) isPageFrame);
	if (list_size(lista)>0) {
		t_marco* marco = list_get(lista,0);
		if (datosConfigMemoria->debug)
			printf(_GREEN "\n\n El carpincho %d pagina %d se encuentran en la cache en el marco %d\n"_RESET, carpincho, pagina, marco->marco);
		log_info(logger,"[ MARCO ] El carpincho %d pagina %d se encuentran en la cache en el marco %d", carpincho, pagina, marco->marco);
		return marco->marco;
	}

	if (datosConfigMemoria->debug) {
		printf(_RED "\n\n El carpincho %d pagina %d no se encuentran en la cache\n"_RESET, carpincho, pagina);
	}
	log_error(logger,"[ ERROR MARCO ] El carpincho %d pagina %d no se encuentran en la cache\n"_RESET, carpincho, pagina);
	return -1;
}

t_marco* get_marco_by_marco_number(int marco_number) {
	bool isNumberFrame(t_marco * marco){
		return marco->marco == marco_number;
	}
	t_list* lista = list_filter(marcos_list, (void*) isNumberFrame);
	if (list_size(lista)>0) {
		t_marco* marco = list_get(lista,0);
		if (datosConfigMemoria->debug)
			printf(_GREEN "\n\n Se encontro el marco %d\n"_RESET, marco->marco);
		log_info(logger,"[ MARCO ] Se encontro el marco %d", marco->marco);
		return marco;
	}

	if (datosConfigMemoria->debug) {
		printf(_RED "\n\n El numero de marco no existe! %d\n"_RESET, marco_number);
	}
	log_error(logger,"[ ERROR MARCO ] El numero de marco no existe!", marco_number);
	return -1;
}

int obtener_marco_para_pagina(t_pagina* pagina) {
	int marco;
	printf(_YELLOW"\n\n la pagina es %d del carpincho %d\n\n"_RESET, pagina->pagina,pagina->carpincho_pid);
	if (pagina_is_in_tlb(pagina)) {
		tlb_hit(pagina);
		printf(_GREEN"\nENTRO EN TLB\n"_RESET);
		marco = get_marco_de_tlb(pagina);
		if (strcmp(datosConfigMemoria->algoritmo_reemplazo_tlb, "LRU")==0) {
			t_tlb* tlb_item = get_tlb_item(pagina);
			tlb_item->timestamp = obtenerTimeStamp();
		}
	} else {
		if (pagina_is_in_cache(pagina)) {
			tlb_miss(pagina);
			printf(_GREEN"\nENTRO EN MP\n"_RESET);
			marco = obtener_marco_de_cache(pagina->carpincho_pid, pagina->pagina);
		} else {
			tlb_hit(pagina);
			printf(_GREEN"\nENTRO EN swamp\n"_RESET);
			marco = swampear_pagina(pagina->carpincho_pid, pagina->pagina);
		}
		actualizar_tlb(pagina, marco);	
	}
	return marco;
}

int get_marco_de_tlb(t_pagina* pagina) {

	if (!pagina_is_in_tlb(pagina)) {
		if (datosConfigMemoria->debug)
			printf(_RED "Se esta buscando el marco en la tlb pero la pagina %d carpincho % no esta en la tlb"_RESET, pagina->pagina,pagina->carpincho_pid);
		log_error(logger,"[ ERROR MARCO ] Se esta buscando el marco en la tlb pero la pagina %d carpincho % no esta en la tlb", pagina->pagina,pagina->carpincho_pid);
		return -1;
	}
	t_tlb* tlb = get_tlb_item(pagina);	
	t_marco* marco = get_marco_by_marco_number(tlb->marco);

	if (!(marco->pagina == pagina->pagina && marco->pid == pagina->carpincho_pid)) {
		if ( datosConfigMemoria->debug) {
			printf(_RED "\nEl marco que esta en la tlb ya no corresponde a la pagina y carpincho buscado\n"_RESET);
			printf(_GREEN "\nVacio el item de tlb impostor.\n" _RESET);
		}
		log_error(logger,"[ ERROR MARCO ] El marco que esta en la tlb ya no corresponde a la pagina y carpincho buscado");
		vaciar_pagina_tlb(tlb);
		if (datosConfigMemoria->debug)
			printf(_GREEN "\nInicio acciones de recupero de marco\n" _RESET);
		log_debug(logger,"[ DEBUG MARCO ] Inicio acciones de recupero de marco", marco);
		return	obtener_marco_para_pagina(pagina);
	}
	return tlb->marco;
}

bool is_marco_in_tlb(int marco) {
	bool isMarcoInTlb(t_tlb * tlb) {
		return tlb->marco == marco;
	}
	t_list* lista = list_filter(tlb_list, (void * ) isMarcoInTlb);
	if (list_size(lista)>1) {
		if (datosConfigMemoria->debug) {
			printf(_RED "\nEl marco %d se encuentra más de una vez en la tlb\n"_RESET, marco);
			print_tlb_status();
		}
		log_error(logger,"[ ERROR MARCO ] El marco %d se encuentra más de una vez en la tlb\n", marco);
	}
	return (bool) list_size(lista);
}

t_list* get_tlb_items_with(int marco) {

}
/*
	//////////		 //\\       ////////// //		  // //////////
	//				//	\\      //		   //		  // //
	//			   //	 \\     //		   //		  // //
	//			  //	  \\    //		   //		  // //
	//			 //////////\\   //		   ///////////// ////////
	//			//			\\  //		   //		  // //
	//		   //			 \\ //		   //		  // //
	////////////			  \\////////// //		  // //////////
						cache funtions
*/

int escribir_en_cache(int marco, char* buffer, int size, uint32_t offset)
{
	int largo = size;

	if (largo+offset>datosConfigMemoria->tamanio_pagina)
	{
		if (datosConfigMemoria->debug)
		cache_imprimir();
		log_error(logger,"[ ERROR MEMORIA PRINCIPAL ] Error en escritura cache. El mensaje + offset no puede superar el tamaño del marco");
		return -1;
	}

	memcpy(get_marco_ptr(marco)+offset, buffer, sizeof(char)*largo);

	log_info(logger,"[ MEMORIA PRINCIPAL ] Se escribio correctamente en memoria en marco %d con %s", marco, buffer);

	if (datosConfigMemoria->debug)
		cache_imprimir();

	return 0;
}
// funcion para test
int escribir_en_cache_string(int marco, char* buffer, uint32_t offset){
	return (marco, buffer, strlen(buffer) + 1, offset);
}

void cache_imprimir() {
	int j;
	if (datosConfigMemoria->debug) {
		unsigned char ch;

		for (j = 0; j < datosConfigMemoria->tamanio; j++) {
			ch = (unsigned char) CACHE[j];

			if (CACHE[j] == '_') {
				printf(_BLUE "_" _RESET);
			} else if (CACHE[j] == 0) {
				printf(_GREEN "-" _RESET);	
			// '0' al '9'
			} else if ((unsigned int)ch >= 48 && (unsigned int)ch <= 57) { 
				printf("%c", CACHE[j]);
			// 'A' al 'Z'
			} else if ((unsigned int)ch >= 65 && (unsigned int)ch <= 90) { 
				printf(_GREEN "%c" _RESET, CACHE[j]);
			// 'a' al 'z'
			} else if ((unsigned int)ch >= 97 && (unsigned int)ch <= 122) { 
				printf(_GREEN "%c" _RESET, CACHE[j]);
			} else if ((unsigned int)ch < 256) {
				printf(_GREEN "%u" _RESET, (unsigned int)ch);
			} else {
				for (int i = 0; i < 8; i++) {
					printf(_GREEN "%d" _RESET, !!((CACHE[j] << i) & 0x80));
				}
			}
			printf(_BLUE "|" _RESET);
		}
		printf("\n");
	}
}

void vaciar_marco_en_cache(int marco) {
	t_marco* marcos_instance = list_get(marcos_list, marco);

	memset(get_marco_ptr(marco), '_', datosConfigMemoria->tamanio_pagina);

	if (is_marco_in_tlb(marco)) {
		vaciar_pagina_tlb(get_tlb_item_by_marco_number(marco));
	}

	log_info(logger,"[ MEMORIA PRINCIPAL ] Se vacio correctamente el marco: %d", marco);
	marcos_instance->isFree = TRUE;
	marcos_instance->pagina = NULL;
	marcos_instance->pid = NULL;
}

void limpiar_cache_con_offset(int marco, uint32_t offset) {
	t_marco* marcos_instance = list_get(marcos_list, marco);

	memset(get_marco_ptr(marco)+offset, '_', datosConfigMemoria->tamanio);
}

void close_cache() {
	free(CACHE);
}

char* retornar_contenido(int marco, uint32_t offset, int tamanio)
{
	if (offset+tamanio > datosConfigMemoria->tamanio_pagina)
	{
		log_error(logger, "[ MEMORIA PRINCIPAL ] Se exede del tamanio del marco");
		return -1;
	}

	char* buffer = malloc(sizeof(char)*tamanio);

	memcpy(buffer, get_marco_ptr(marco)+offset, sizeof(char)*tamanio);

	log_info(logger,"[ MEMORIA PRINCIPAL ] Se leyo correctamente el marco: %d retorna %s", marco, buffer);

	if (datosConfigMemoria->debug)
	{
		printf("\nEl buffer leído de la CACHE ES: ");

		for(int i=0; i<= tamanio -1; i++)
		{
			printf("%c",buffer[i]);
		}

		printf("\n\n");
	}

	return buffer;
}

/*
	///////// 		 //\\       //////////
	//		//		//	\\      //		  
	//		//	   //	 \\     //		  
	//	   //	  //	  \\    //		  
	////////	 //////////\\   //	//////
	//			//			\\  //		 //
	//		   //			 \\ //		 // ///
	//		  //			  \\//////////  ///
		paginas functions
*/

void free_pages_by_carpincho(t_carpincho* carpincho) {
	list_iterate(carpincho->paginas, (void*) free);
	list_destroy(carpincho->paginas);
	//TODO: Tenemos que elimnar de SWAmP todas las paginas ????
}

void imprimir_paginas() {
	if (datosConfigMemoria->debug) {
		int i;
		printf("\n\n----------------------------------\n\n");
		for (i=0; i<list_size(carpinchos_list); i++) {
			t_carpincho* carpincho = list_get(carpinchos_list,i);
			printf("\n El carpincho %d posee %d paginas", carpincho->pid, list_size(carpincho->paginas));
			printf("\nlas paginas son:\n");
			int j;
			for (j=0; j<list_size(carpincho->paginas); j++) {
				t_pagina* pagina = list_get(carpincho->paginas,j);
				printf(" %d, ", pagina->pagina);
			}
		}
		printf("\n\n----------------------------------\n\n");
	}
}
/*
void imprimir_paginas_list(t_list* paginas) {
	if (datosConfigMemoria->debug) {
		int i;
		printf("\n\n----------------------------------\n\n");
		for (i=0; i<list_size(paginas); i++) {
				t_pagina* pagina = list_get(carpincho->paginas,j);
				printf(" %d, ", pagina->pagina);
			
		}
		printf("\n\n----------------------------------\n\n");
	}
}
*/

t_pagina* get_page_by_ints(int carpincho, int pagina) {
	return get_page(get_carpincho(carpincho), pagina);
}

t_pagina* get_page(t_carpincho* carpincho, int pagina) {
	bool same_pids_page(t_pagina* pagina1) {
		return pagina1->carpincho_pid == carpincho->pid && pagina1->pagina == pagina;
	}
	if (list_size(list_filter(carpincho->paginas, (void*) same_pids_page))>1) {
		if (datosConfigMemoria->debug) {
			printf(_RED "\n\n El carpincho %d pagina %d se encuentra duplicada\n"_RESET, carpincho, pagina);
		}
		log_error(logger,"[ ERROR PAGINA ] El carpincho %d pagina %d se encuentra duplicada", carpincho, pagina);
	}
	return list_find(carpincho->paginas, (void*) same_pids_page);
}

bool can_carpincho_add_marco_to_page(t_carpincho* carpincho) {
	if (strcmp(datosConfigMemoria->tipo_asignacion,"DINAMICA")==0)
	 return list_size(get_all_pages_in_cache()) <= cantidad_maxima_marcos && any_marco_free();
	else 
	 return list_size(get_all_pages_in_cache()) <= cantidad_maxima_marcos && any_marco_free() &&
	   		amount_frame_used_by_carpincho(carpincho) < datosConfigMemoria->marcos_por_carpincho;
}

t_pagina* create_pagina(t_carpincho* carpincho) {
	t_pagina* pagina = malloc(sizeof(t_pagina));
	t_marco* marco;
	bool before_value = can_carpincho_add_marco_to_page(carpincho);
	pagina->pagina = list_size(carpincho->paginas)+1;
	pagina->carpincho_pid = carpincho->pid;
	pagina->carp_num = carpincho->numero;
	if (before_value){
			marco = get_free_marco();	
		pagina->marco = marco->marco;
	} else {
		pagina->marco = swampear_pagina_when_create_page(pagina->carpincho_pid, pagina->pagina);
		marco = get_marco_by_marco_number(pagina->marco);
	}
		marco->pid = carpincho->pid;
	marco->pagina = pagina->pagina;
	marco->isFree = FALSE;
	pagina->bit_presencia = TRUE;

	log_info(logger,"[ PAGINA ] Se creo la pagina %d del carpincho %d en el marco %d", pagina->pagina, pagina->carpincho_pid, pagina->marco);
	actualizar_tlb(pagina, marco->marco);

	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0) {
		pagina->bit_de_uso = TRUE;
		pagina->bit_de_modificacion = FALSE;
	} else {
		pagina->timestamp = obtenerTimeStamp();
	}
	if (datosConfigMemoria->debug) {
		printf(_YELLOW "\nSe creo pagina con exito\n" _RESET);
		if (before_value) {
			printf("\n----------MARCO-----------------------\n");
			printf("numero marco: %d\n", marco->marco);
			printf("carpincho que ocupa el marco: %d\n", marco->pid);
			printf("marco isFree: %d ",marco->isFree);
			if(marco->isFree) {
				printf(_GREEN " TRUE\n" _RESET);
			} else {
				printf(_RED " FALSE\n" _RESET);
			}
			printf("\n----------PAGINA----------------------\n");
			printf("pagina: %d\n",pagina->pagina);
			printf("pagina.marco: %d\n", pagina->marco);
			//presencia
			printf("pagina->bit_presencia:");
			if(pagina->bit_presencia) {
				printf(_GREEN " TRUE\n" _RESET);
			} else {
				printf(_RED " FALSE\n" _RESET);
			}
			if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0) {
				//de_uso
				printf("pagina->bit_de_uso:");
				if(pagina->bit_de_uso) {
					printf(_GREEN " TRUE\n" _RESET);
				} else {
					printf(_RED " FALSE\n" _RESET);
				}
				//modificacion
				printf("pagina->bit_de_modificacion:");
				if(pagina->bit_de_modificacion) {
					printf(_GREEN " TRUE\n" _RESET);
				} else {
					printf(_RED " FALSE\n" _RESET);
				}
			}else {
				printf("tiempo de creación: %d \n", pagina->timestamp);
			}

			printf("\n----------CARPINCHO-------------------\n");
			printf("carpincho pid: %d\n", carpincho->pid);
			printf("cantidad paginas del carpincho: %d\n", list_size(carpincho->paginas));
		} else {
			printf(_RED "la pagina no tiene marco asignado\n" _RESET);
			printf("\n----------PAGINA----------------------\n");
			printf("pagina: %d\n",pagina->pagina);
			printf("pagina.marco: NULL \n");
			//presencia
			printf("pagina->bit_presencia:");
			if(pagina->bit_presencia) {
				printf(_GREEN " TRUE\n" _RESET);
			} else {
				printf(_RED " FALSE\n" _RESET);
			}
			if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0) {
				//de_uso
				printf("pagina->bit_de_uso:");
				if(pagina->bit_de_uso) {
					printf(_GREEN " TRUE\n" _RESET);
				} else {
					printf(_RED " FALSE\n" _RESET);
				}
				//modificacion
				printf("pagina->bit_de_modificacion:");
				if(pagina->bit_de_modificacion) {
					printf(_GREEN " TRUE\n" _RESET);
				} else {
					printf(_RED " FALSE\n" _RESET);
				}
			}else {
				printf("tiempo de creación: %d \n", pagina->timestamp);
			}

			printf("\n----------CARPINCHO-------------------\n");
			printf("carpincho pid: %d\n", carpincho->pid);
			printf("cantidad paginas del carpincho: %d\n", list_size(carpincho->paginas));
		}
	}

	list_add(carpincho->paginas, pagina);

	if (before_value &&list_size(carpincho->paginas) != marco->pagina) 
		log_error(logger, "[ ERROR PAGINA ] ERROR DE ASIGNACION de NUMERO DE PAGIA DE CARPINCHO AL MARCO");

	if (datosConfigMemoria->debug) {
		if (before_value){
			if (list_size(carpincho->paginas) != marco->pagina) {
				printf("\n\n ERROR DE ASIGNACION de NUMERO DE PAGIA DE CARPINCHO AL MARCO\n\n");
				printf("\n Numero pagina Marco: %d\n\n", marco->pagina);
				printf("\n Numero pagina Carpincho: %d\n\n", list_size(carpincho->paginas));
			}
		}
		printf("\n''''''''''''''''''''''''''''\n");
	}
	if (datosConfigMemoria->debug)
		imprimir_marcos();
	return pagina;
}

t_list* get_all_pages_in_cache() {

	if (!list_is_empty(paginas_en_cache_list)) {
		list_destroy(paginas_en_cache_list);
		paginas_en_cache_list = list_create();
	}
	if(list_is_empty(paginas_en_cache_list)) {
		void get_carpinchos_pages(t_carpincho* carpincho) {
			list_add_all(paginas_en_cache_list, carpincho->paginas);
		}
		list_iterate(carpinchos_list, (void*) get_carpinchos_pages);
	}
	
	bool present_bit(t_pagina * pagina){
		return pagina->bit_presencia;
	}
	paginas_en_cache_list = list_filter(paginas_en_cache_list, (void*) present_bit);
	return paginas_en_cache_list;
}

t_list* get_pages_in_cache_by_carpincho_pid(int carpincho) {
	t_carpincho* carpincho_instance = get_carpincho(carpincho);
	t_list* paginas = list_filter(carpincho_instance->paginas,(void*) pagina_is_in_cache);
	if (list_size(paginas)>datosConfigMemoria->marcos_por_carpincho) {		
		if (datosConfigMemoria->debug)
			printf(_RED "\n\n El carpincho %d más paginas (%d) que las permitidas(%d) por el archivo de configuracion\n"_RESET,
				carpincho,
				list_size(paginas),
				datosConfigMemoria->marcos_por_carpincho);
		log_error(logger, "[ ERROR PAGINA ] El carpincho %d más paginas (%d) que las permitidas(%d) por el archivo de configuracion",
			carpincho,
			list_size(paginas),
			datosConfigMemoria->marcos_por_carpincho);
	} 
	
	return paginas;
}

int vaciar_pagina(t_pagina* pagina) {
	if (!pagina->bit_presencia) {
		if (datosConfigMemoria->debug) printf(_RED"\nLa pagina no tiene bit de presencia en cache no se puede liberar/escribir/leer\n"_RESET);
		log_error(logger,"[ ERROR PAGINA ] Se trato de liberar la pagina %d del carpincho %d pero no tiene bit_presencia para que suceda",pagina->pagina,pagina->carpincho_pid);
		return 0;
	}
	vaciar_marco_en_cache(pagina->marco);
	log_info(logger,"[ PAGINA ] Se trato de liberar la pagina %d del carpincho %d pero no tiene bit_presencia para que suceda",pagina->pagina,pagina->carpincho_pid);
	if (datosConfigMemoria->debug) cache_imprimir();
	pagina->bit_presencia = FALSE;
	return 1;
}

char* leer_en_pagina(t_pagina* pagina, int tamanio, uint32_t offset) {
	char* result = '-1';
	printf("\n\n\nLa pagina es %i\n\n\n", pagina->pagina);
	fflush(stdout);
	if (pagina_is_in_tlb(pagina) && strcmp(datosConfigMemoria->algoritmo_reemplazo_tlb, "LRU")==0) {
		t_tlb* tlb_item = get_tlb_item(pagina);
		tlb_item->timestamp = obtenerTimeStamp();
	}
	pagina->marco = obtener_marco_para_pagina(pagina);
	if (pagina->marco == -1) {
		if (datosConfigMemoria->debug)
			printf(_RED "\n\n Ocurrio un problema el numero de marco es -1 al leer\n\n" _RESET);
		return '-1';
	} else {
		pagina->bit_presencia = TRUE;
	}
	if (datosConfigMemoria->debug) {
		printf("\n pagina: %d carpincho: %d \n", pagina->pagina, pagina->carpincho_pid);
		printf("\n bit de presencia: %d\n",pagina->bit_presencia );
		printf("\n pagina_is_in_cache: %d\n",pagina_is_in_cache(pagina) );
		printf("\n pagina_is_in_tlb: %d\n",pagina_is_in_tlb(pagina) );
	}

	if (pagina->bit_presencia && pagina_is_in_cache(pagina) && pagina_is_in_tlb(pagina)) {
		result = retornar_contenido(pagina->marco, offset, tamanio);
	}else{
		if (datosConfigMemoria->debug)
			printf("\n\n Ocurrio un problema la pagina no esta en cache al leer\n\n");
		log_error(logger,"[ ERRROR PAGINA ] Ocurrio un problema la pagina no esta en cache al leer");
		return '-1';
	}

	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0) {
		pagina->bit_de_uso = TRUE;
	} else {
		pagina->timestamp = obtenerTimeStamp();
	}

	log_info(logger,"[ PAGINA ] Se Leyo en la direccion logica de pagina %d carpincho %d con resultado %s",
		pagina->pagina,
		pagina->carpincho_pid,
		result
	);
	if (datosConfigMemoria->debug)
		imprimir_marcos();
	return result;
}

int escribir_en_pagina(t_pagina* pagina, char* buffer, int size, uint32_t offset) {
	int result;

	if (pagina_is_in_tlb(pagina) && strcmp(datosConfigMemoria->algoritmo_reemplazo_tlb, "LRU")==0)
		get_tlb_item(pagina)->timestamp = obtenerTimeStamp();
	
	pagina->marco = obtener_marco_para_pagina(pagina);

	if (pagina->marco == -1) {
		if (datosConfigMemoria->debug)
			printf("\n\n Ocurrio un problema el numero de marco es -1 al escribir\n\n");
		return -1;
	} else { 
		pagina->bit_presencia = TRUE;
	}

	if (datosConfigMemoria->debug) {
		printf("\n pagina: %d carpincho: %d \n", pagina->pagina, pagina->carpincho_pid);
		printf("\n bit de presencia: %d\n",pagina->bit_presencia );
		printf("\n pagina_is_in_cache: %d\n",pagina_is_in_cache(pagina) );
		printf("\n pagina_is_in_tlb: %d\n",pagina_is_in_tlb(pagina) );
	}

	if (pagina->bit_presencia &&
		 pagina_is_in_cache(pagina) &&
		 pagina_is_in_tlb(pagina)) {
		log_info(logger, "\n\n\n\n\n\n\n\n\nEl valor2 es: %s\n\n\n\n\n\n\n\n\n", buffer);
		result = escribir_en_cache(pagina->marco, buffer, size, offset);
	}else{
		if (datosConfigMemoria->debug)
			printf(_RED "\n\n Ocurrio un problema la pagina no esta en cache al escribir\n\n" _RESET);
		log_error(logger,"[ ERRROR PAGINA ] Ocurrio un problema la pagina no esta en cache al escribir");
		return -1;
	}
	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0) {
		pagina->bit_de_uso = TRUE;
		pagina->bit_de_modificacion = TRUE;
	} else {
		pagina->timestamp = obtenerTimeStamp();
	}
	log_info(logger,"[ PAGINA ] Se escribio en la direccion logica de pagina %d carpincho %d con resultado %d",
		pagina->pagina,
		pagina->carpincho_pid,
		result
	);
	if (datosConfigMemoria->debug)
		imprimir_marcos();
	return result;
}

// FUNCION PARA TESTS
int escribir_en_pagina_string(t_pagina* pagina, char* buffer, uint32_t offset){
	return escribir_en_pagina(pagina, buffer, strlen(buffer) + 1, offset);
}

bool pagina_is_in_tlb(t_pagina* pagina) {
	bool same_page_tlb_item(t_tlb* tlb_item) {
		return tlb_item->pid == pagina->carpincho_pid &&
				tlb_item->pagina == pagina->pagina;
	}
	t_list* tlb = list_filter(tlb_list, (void*) same_page_tlb_item);
	
	if (list_size(tlb)>1) {
		if (datosConfigMemoria->debug) {
			printf(_RED "\n\n El carpincho %d pagina %d se encuentra duplicada en TLB\n"_RESET, pagina->carpincho_pid, pagina->pagina);
		}
		log_error(logger,"[ ERRROR PAGINA ] El carpincho %d pagina %d se encuentra duplicada en TLB", pagina->carpincho_pid, pagina->pagina);
		return -1;
	}
	return !list_is_empty(tlb);
}

bool pagina_is_in_cache(t_pagina* pagina) {
	printf(_RED"\nENTRE EN pagina_is_in_cache(t_pagina* pagina)\n"_RESET);
	bool same_page_marco(t_marco* marco) {
		return marco->pid == pagina->carpincho_pid &&
				marco->pagina == pagina->pagina;
	}
	t_list* marcos = list_filter(marcos_list, (void*) same_page_marco);
	if (list_size(marcos)>1) {
		if (datosConfigMemoria->debug) {
			printf(_RED "\n\n El carpincho %d pagina %d se encuentra duplicada en CACHE\n"_RESET, pagina->carpincho_pid, pagina->pagina);
		}
		log_error(logger,"[ ERRROR PAGINA ] El carpincho %d pagina %d se encuentra duplicada en CACHE", pagina->carpincho_pid, pagina->pagina);
	}
	printf(_RED"\n4\n"_RESET);
	return !list_is_empty(marcos);
}

t_list* get_all_pages_in_tlb() {

	if (!list_is_empty(paginas_en_cache_list)) {
		list_destroy(paginas_en_cache_list);
		paginas_en_cache_list = list_create();
	}
	if(list_is_empty(paginas_en_cache_list)) {
		void get_carpinchos_pages(t_carpincho* carpincho) {
			list_add_all(paginas_en_cache_list, carpincho->paginas);
		}
		list_iterate(carpinchos_list, (void*) get_carpinchos_pages);
	}
	
	bool present_bit(t_pagina * pagina){
		return pagina->bit_presencia;
	}
	paginas_en_cache_list = list_filter(paginas_en_cache_list, (void*) present_bit);
	return paginas_en_cache_list;
}

// retorna el numero de marco en el cual se posicion la pagina
int swampear_pagina(int carpincho, int pagina) {
	t_pagina* go_to_swamp;
	t_list* paginas;
	int marco;
	// busco la proxima pagina a swampear segun el algoritmo y el tipo de asignacion
	if (strcmp(datosConfigMemoria->tipo_asignacion,"FIJA")==0) {
		paginas = get_pages_in_cache_by_carpincho_pid(carpincho);
		if (list_size(paginas)>list_size(marcos_list)) {
			if (datosConfigMemoria->debug) {
				printf(_RED"\nHAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA\n"_RESET);
			}
			log_error(logger,"[ ERROR PAGINA ] HAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA");
		} 
	} else {
		paginas = get_all_pages_in_cache();
		if (list_size(paginas)>list_size(marcos_list)) {
			if (datosConfigMemoria->debug) {
				printf(_RED"\nHAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA\n"_RESET);
			}
			log_error(logger,"[ ERROR PAGINA ] HAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA");
		}
	}
	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu,"M-CLOCK")==0) {
		go_to_swamp = get_proxima_a_swampear_by_m_clock(paginas);
	} else {
		go_to_swamp = get_proxima_a_swampear_by_lru(paginas);		
	}


	if (!pagina_is_in_cache(go_to_swamp)) {
		if(datosConfigMemoria->debug) {
			printf(_RED "\n\n Error de inconsistencia la pagina a swampear no esta en CACHE\n\n" _RESET);
		}
		log_error(logger, "[ ERROR PAGINA ] Error de inconsistencia la pagina a swampear no esta en CACHE");
		return -1;
	}

	//	envio la pagina a swamp
	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu,"M-CLOCK")==0 && go_to_swamp->bit_de_modificacion == FALSE) {

	} else {
		luz_roja();
		enviar_escritura_swap(
			go_to_swamp->carpincho_pid,
			go_to_swamp->pagina,
			retornar_contenido( go_to_swamp->marco, 0,
				datosConfigMemoria->tamanio_pagina
			));//TODO: Posible leak de leer contenido.
		luz_verde();
		log_info(logger,"[ SWAMPEAR PAGINA ] Se envio a swamp la pagina %d del carpincho: %d en el marco %d.", go_to_swamp->pagina, go_to_swamp->carpincho_pid, go_to_swamp->marco);
	}
	//	vacio el marco
	vaciar_marco_en_cache(go_to_swamp->marco);

	// obtengo el numero de marco de la pagina
	t_marco* marco_instance = get_marco_by_marco_number(go_to_swamp->marco);
	marco_instance->pagina = pagina;
	marco_instance->pid = carpincho;
	marco_instance->isFree = FALSE;
	go_to_swamp->bit_presencia = FALSE;
	go_to_swamp->marco = NULL;

	// Leo pagina de swamp
	// obtengo el contenido de la nueva pagina desde swamp
	// retorno el numero de marco en caso que no haya errores sino retorno -1
	luz_roja();
	if (lectura_swamp(carpincho,pagina,marco_instance->marco) != -1){
		log_info(logger,"[ SWAMPEAR PAGINA ] Se leo de swamp la pagina %d del carpincho: %d en el marco %d.", pagina, carpincho, marco_instance->marco);
		luz_verde();
		return marco_instance->marco;
	} else {
		//ocurrio un error
		luz_verde();
		return -1;
	}
}

// retorna el numero de marco en el cual se posicion la pagina
int swampear_pagina_when_create_page(int carpincho, int pagina) {
	t_pagina* go_to_swamp;
	t_list* paginas;
	int marco;
	// busco la proxima pagina a swampear segun el algoritmo y el tipo de asignacion
	if (strcmp(datosConfigMemoria->tipo_asignacion,"FIJA")==0) {
		paginas = get_pages_in_cache_by_carpincho_pid(carpincho);
			if (list_size(paginas)>list_size(marcos_list)) {
			if (datosConfigMemoria->debug) {
				printf(_RED"\nHAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA\n"_RESET);
			}
			log_error(logger,"[ ERROR PAGINA ] HAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA");
		} 
	} else {
			paginas = get_all_pages_in_cache();
		if (list_size(paginas)>list_size(marcos_list)) {
			if (datosConfigMemoria->debug) {
				printf(_RED"\nHAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA\n"_RESET);
			}
			log_error(logger,"[ ERROR PAGINA ] HAY MAS PAGINAS QUE MARCOS EN CACHE. ERROR DE INCONSISTENCIA");
		}
	}
	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu,"M-CLOCK")==0) {
		go_to_swamp = get_proxima_a_swampear_by_m_clock(paginas);
	}	else {
		go_to_swamp = get_proxima_a_swampear_by_lru(paginas);
		printf(_YELLOW"LA PAGINA SELECCIONADA PARA SACAR ES C%dP%d"_RESET,go_to_swamp->carpincho_pid,go_to_swamp->pagina);
	}
	/*
	if(datosConfigMemoria->debug)
		printf(_RED"\n\nLa pagina a swampear esta en cache %d\n\n"_RESET,pagina_is_in_cache(go_to_swamp));
	if (!pagina_is_in_cache(go_to_swamp)) {
		f(datosConfigMemoria->debug) {
			printf(_RED "\n\n Error de inconsistencia la pagina a swampear no esta en CACHE\n\n" _RESET);
		}
		log_error(logger, "[ ERROR PAGINA ] Error de inconsistencia la pagina a swampear no esta en CACHE");
		return -1;
	}
	*/

	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu,"M-CLOCK")==0 && go_to_swamp->bit_de_modificacion == FALSE) {
	}	else {
	//	envio la pagina a swamp
		luz_roja();
		enviar_escritura_swap(
			go_to_swamp->carpincho_pid,
			go_to_swamp->pagina,
			retornar_contenido( go_to_swamp->marco, 0,
				datosConfigMemoria->tamanio_pagina
			));//TODO: Posible leak de leer contenido.
		luz_verde();
		log_info(logger,"[ SWAMPEAR PAGINA ] Se envio a swamp la pagina %d del carpincho: %d en el marco %d.", go_to_swamp->pagina, go_to_swamp->carpincho_pid, go_to_swamp->marco);
	}
	//	vacio el marco
	vaciar_marco_en_cache(go_to_swamp->marco);

	// obtengo el numero de marco de la pagina
	t_marco* marco_instance = get_marco_by_marco_number(go_to_swamp->marco);
	marco_instance->pagina = pagina;
	marco_instance->pid = carpincho;
	marco_instance->isFree = FALSE;
	go_to_swamp->bit_presencia = FALSE;
	go_to_swamp->marco = NULL;
	return marco_instance->marco;
}


int calcular_cantidad_paginas_necesaria(int espacioLibreActual, int tamanioDato)
{
	//Calculamos el espacio necesario es el alloc +9, restamos el espacio libre
	int espacioNecesario = (tamanioDato + 9) - espacioLibreActual;

	//Calculamos la cant de paginas que necesitamos pedir
	if((espacioNecesario % datosConfigMemoria->tamanio_pagina) == 0)
		return espacioNecesario / datosConfigMemoria->tamanio_pagina;
	
	int cantPag = (espacioNecesario / datosConfigMemoria->tamanio_pagina) + 1;

	return cantPag;
}

/*
	//			//////    // 	   //
	//			//	 //   // 	   //
	//			//	  //  // 	   //
	//			//	 //   // 	   //
	//			/////     // 	   //
	//			//	//    // 	   //
	//			//	 //   // 	   //
	//////////  //    //  ///////////
		LRU functions
*/


t_pagina* get_proxima_a_swampear_by_lru(t_list* paginas) {
	if (any_marco_free()) {
		if(datosConfigMemoria->debug) {
			printf(_RED "\nSi esta acción de lectura no preste atención al siguiente mensaje\n" _RESET);
			printf(_RED "\nHay marcos libres todavía no deben ejecutar el algoritmo\n" _RESET);
		}
//		return -1;
	}
/*	bool sorted_by_marco_number(t_pagina * pagina1,t_pagina * pagina2){
		return (pagina1->marco) < (pagina2->marco);
	}
	list_sort(paginas, (void*) sorted_by_marco_number);
*/	bool menorTimeStamp(t_pagina * pagina1,t_pagina * pagina2){
		return (pagina1->timestamp) < (pagina2->timestamp);
	}
	list_sort(paginas, (void*) menorTimeStamp);
	void printfPages(t_pagina* pagina) {
		printf(_YELLOW "\npagina %d carpincho %d timestamp %d\n" _RESET,
			pagina->pagina, pagina->carpincho_pid, pagina->timestamp);
	}
	list_iterate(paginas, (void*) printfPages);
	t_pagina* pagina = list_get(paginas,0);
	if(datosConfigMemoria->debug) {
		printf(_YELLOW "Proxima pagina a swampear: %d del carpincho: %d por el lru\n" _RESET, pagina->pagina, pagina->carpincho_pid);
	}
	log_info(logger,"[ MMU LRU ] Proxima pagina a swampear: %d del carpincho: %d por el lru.", pagina->pagina, pagina->carpincho_pid);
	return pagina;
}


/*
	//\\      ////				//////////  //			//////////  //////////  //	   //
	// \\    // //				//			//			//		//  //			//	  //
	//  \\  //  //				//			//			//		//  //			//	 //
	//   \\//   //				//			//			//		//  //			//	//
	//          //  //////////  //			//			//		//  //			////\\
	//          //				//			//			//		//  //			//	 \\
	//          //				//			//			//		//  //			//	  \\
	//          //				//////////  //////////  //////////  //////////  //	   \\
					m-clock functions
*/


t_pagina* get_proxima_a_swampear_by_m_clock(t_list* paginas) {
	printf(_YELLOW"\nENTRO EN EL ALGORITMO M-CLOCK\n"_RESET);
	if (any_marco_free()) {
		if(datosConfigMemoria->debug) {
			printf(_RED "\nSi esta acción de lectura no preste atención al siguiente mensaje\n" _RESET);
			printf(_RED "\nHay marcos libres todavía no deben ejecutar el algoritmo\n" _RESET);
		}
	}
	int i, j, stack_size;
	t_pagina* page;
	int use_bit_searched = 0;
	int edit_bit_searched = 0;		
	stack_size = list_size(paginas);
	int last_pointer_position;
	t_carpincho* carpincho;

	//Ordeno lista de paginas segun numero de marco
	bool sorted_by_marco_number(t_pagina * pagina1,t_pagina * pagina2) {
		return (pagina1->marco) < (pagina2->marco);
	}
	list_sort(paginas, (void*) sorted_by_marco_number);
	if (strcmp(datosConfigMemoria->tipo_asignacion,"FIJA")==0) {
		page = list_get(paginas,0);
		carpincho = get_carpincho(page->carpincho_pid);
		last_pointer_position = carpincho->algoritm_stack_pointer;
	} else {
		last_pointer_position = algoritm_stack_pointer;
	}
	if (datosConfigMemoria->debug)
		printf(_YELLOW"\n\nCANTIDAD DE PAGIANAS A RECORRER POR EL ALGORITMO M-CLOCK %d\n\n"_RESET,stack_size);

	for (i=0;i<4;i++) {
		printf(_YELLOW"\n\nCANTIDAD DE PAGIANAS A RECORRER POR EL ALGORITMO M-CLOCK %d\n\n"_RESET,stack_size);
		printf(_YELLOW"\n\nLast_pointer_position M-CLOCK %d\n\n"_RESET,last_pointer_position);
		printf(_YELLOW"\nIteracion %d\n"_RESET,i);
		for(j=last_pointer_position;j<stack_size;j++) {
			page = list_get(paginas,j);			
			if (datosConfigMemoria->debug) {
				printf(_YELLOW"\n C%dP%d "_RESET,page->carp_num,page->pagina);
				if (page->bit_presencia) printf(_GREEN"P"_RESET);
				else printf(_RED"-");
				if (page->bit_de_uso) printf(_GREEN"U"_RESET);
				else printf(_RED"-");
				if (page->bit_de_modificacion) printf(_GREEN"M\n"_RESET);
				else printf(_RED"-\n");
			}

			if(page->bit_de_uso == use_bit_searched && page->bit_de_modificacion == edit_bit_searched) {
				if (strcmp(datosConfigMemoria->tipo_asignacion,"FIJA")==0) {
					carpincho->algoritm_stack_pointer = j;
				} else {
					algoritm_stack_pointer=j;
				}
				if(datosConfigMemoria->debug) {
					printf(_YELLOW "\nProxima pagina a swampear: %d del carpincho: %d\n por el m-clock" _RESET, page->pagina, page->carpincho_pid);
					printf(_YELLOW "\n--------------------\n"_RESET);
					printf(_YELLOW "\n pagina %d \n"_RESET, page->pagina);
					printf(_YELLOW "\n carpincho %d \n"_RESET,page->carpincho_pid );
					printf(_YELLOW "\n marco %d \n"_RESET,page->marco);
					printf(_YELLOW "\n bit_presencia %d \n"_RESET,page->bit_presencia );
					printf(_YELLOW "\n bit_de_uso %d \n"_RESET,page->bit_de_uso );
					printf(_YELLOW "\n bit_de_modificacion %d\n" _RESET,page->bit_de_modificacion);
					printf(_YELLOW "\n--------------------\n"_RESET);
				}
				log_info(logger,"[ MMU M-CLOCK ] Proxima pagina a swampear: %d del carpincho: %d por el m-clock el puntero se quedo en la posición %d", page->pagina, page->carpincho_pid, j);
				if (strcmp(datosConfigMemoria->tipo_asignacion,"FIJA")==0) {
					carpincho->algoritm_stack_pointer = j + 1;
					if (carpincho->algoritm_stack_pointer >= stack_size) {
						carpincho->algoritm_stack_pointer = 0;
					}
					if(datosConfigMemoria->debug)
						printf(_YELLOW "\nEl puntero se quedo en la posicion: %d\n" _RESET, carpincho->algoritm_stack_pointer);
				} else {
					algoritm_stack_pointer=j + 1;
					if (algoritm_stack_pointer >= stack_size) {
						algoritm_stack_pointer = 0;
					}
					if(datosConfigMemoria->debug)
						printf(_YELLOW "\nEl puntero se quedo en la posicion: %d\n" _RESET, algoritm_stack_pointer);
				}

				return page;
			}
			if (i==1)
				page->bit_de_uso = 0;
		}
		if (i == 0 || i == 2)
			edit_bit_searched = 1;
		else
			edit_bit_searched = 0;
	}
	if (datosConfigMemoria->debug)
		printf(_YELLOW"\nNO ENCONTRO PAGINA PARA REMPLAZAR, SE PRODUCIRA UN SF M-CLOCK\n"_RESET);
}

void print_m_clock_status_cache() {
	int j;
	t_list* paginas = get_all_pages_in_cache();
	t_pagina* page;
	for(j=0;j<list_size(paginas);j++) {
		page = list_get(paginas,j);
		printf(_YELLOW "\n--------------------\n"_RESET);
		printf(_YELLOW "\n pagina %d \n"_RESET, page->pagina);
		printf(_YELLOW "\n carpincho %d \n"_RESET,page->carpincho_pid );
		printf(_YELLOW "\n marco %d \n"_RESET,page->marco);
		printf(_YELLOW "\n bit_presencia %d \n"_RESET,page->bit_presencia );
		printf(_YELLOW "\n bit_de_uso %d \n"_RESET,page->bit_de_uso );
		printf(_YELLOW "\n bit_de_modificacion %d\n" _RESET,page->bit_de_modificacion);
		retornar_contenido(page->marco,0,1); //TODO: Posbile leak
		printf(_YELLOW "\n--------------------\n"_RESET);

	}
}
void print_m_clock_gant_status_cache() {
	int j;
	t_list* paginas = get_all_pages_in_cache();
	t_pagina* page;
	bool sorted_by_number(t_pagina * pagina1, t_pagina * pagina2) {
		return (pagina1->marco) < (pagina2->marco);
	}
	list_sort(paginas, (void*) sorted_by_number);
	for(j=0;j<list_size(paginas);j++) {
		page = list_get(paginas,j);
		printf(_YELLOW "\n--------------------\n"_RESET);
	//		printf(_YELLOW "\n c%dp%d"_RESET,page->carpincho_pid, page->pagina);
		retornar_contenido(page->marco,0,1);//TODO: Posbile leak
		if (page->bit_de_uso)
			printf(_GREEN "%d"_RESET,page->bit_de_uso );
		else
			printf(_RED "%d"_RESET,page->bit_de_uso );
		if (page->bit_de_modificacion)
			printf(_GREEN "%d\n"_RESET,page->bit_de_modificacion );
		else
			printf(_RED "%d\n"_RESET,page->bit_de_modificacion );
		printf(_YELLOW "\n--------------------\n"_RESET);
	}
}


/*
	///////////  ////////////// 	    /\			///////////  //		 //
	//				   //			   //\\			//			 //		//
	//				   //			  //  \\		//			 //	   //
	//				   //			 //    \\		//			 //   //
	///////////		   //			////////\\		//			 /////\\
			 //		   //          //		 \\		//			 //	   \\
			 //		   //		  //		  \\	//			 //     \\
			 //		   //		 //    		   \\	//			 //      \\
	///////////		   //		//				\\  ///////////  //		  \\
								stack functions
		Solo es un dibager que va imprimir la secuencia en la cual se
				 ejecutaron los algoritmos lru y m-clock
*/

void print_stack() {
	if (strcmp(datosConfigMemoria->algoritmo_reemplazo_mmu, "M-CLOCK")==0) {}
}


/*

////////////////////		///////
		//		  //		//	  //
		//		  //		//	   //
		//		  //		//	  //
		//		  //		///////
		//		  //		//	   //
		//		  //		//		//
		//		  //		//	    //
		//		  //		//	   //
		//		  //////////////////

		TLB functions
*/
void free_tlb() {
	list_iterate(tlb_list, (void*) free);
}

t_list* create_tlb() {
	int i;
	tlb_list = list_create();
	for (i=0;i<datosConfigMemoria->cantida_entradas_tlb;i++) {
		t_tlb* tlb_item = malloc(sizeof(t_tlb));
		tlb_item->entrada = i;
		tlb_item->pid = NULL;
		tlb_item->pagina = NULL;
		tlb_item->marco = -1;
		if (strcmp(datosConfigMemoria->algoritmo_reemplazo_tlb,"LRU")==0)
			tlb_item->timestamp = obtenerTimeStampTLB();
		list_add(tlb_list,tlb_item);
	}
	return tlb_list;
}

t_tlb* get_tlb_item(t_pagina* pagina) {
	bool same_pid(t_tlb* tlb) { 
		return tlb->pagina == pagina->pagina && tlb->pid == pagina->carpincho_pid;
	}
	t_list* lista = list_filter(tlb_list,(void *) same_pid);
	if (list_size(lista)==0) {
		if (datosConfigMemoria->debug)		
			printf("\n\n La pagina %d carpincho %d no se encuentra en la tlb\n\n", pagina->pagina, pagina->carpincho_pid);
		log_error(logger,"[ ERROR TLB ] La pagina %d carpincho %d no se encuentra en la tlb", pagina->pagina, pagina->carpincho_pid);
	}

	if (list_size(lista)>1) {
		if (datosConfigMemoria->debug)
			printf("\n\n La pagina %d carpincho %d se encuentra duplicada en la tlb\n\n", pagina->pagina, pagina->carpincho_pid);
		log_error(logger,"[ ERROR TLB ] La pagina %d carpincho %d se encuentra duplicada en la tlb", pagina->pagina, pagina->carpincho_pid);
	}

	return list_get(lista,0);
}

t_tlb* get_tlb_item_by_marco_number(int marco) {
	bool same_frame_of_tlb_item(t_tlb* tlb) { 
		return tlb->marco == marco;
	}
	t_list* lista = list_filter(tlb_list,(void *) same_frame_of_tlb_item);
	if (list_size(lista)==0) {
		if (datosConfigMemoria->debug)
			printf("\n\n El marco %d no se encuentra en la tlb\n\n", marco);
		log_error(logger, "[ ERROR TLB ] El marco %d no se encuentra en la tlb", marco);
	}

	if (list_size(lista)>1) {
		if (datosConfigMemoria->debug)
			printf("\n\n El marco %d se encuentra duplicada en la tlb\n\n", marco);
		log_error(logger, "[ ERROR TLB ] El marco %d se encuentra duplicada en la tlb", marco);
	}

	return list_get(lista,0);
}

void actualizar_tlb(t_pagina* pagina, int marco) {
	if (marco != -1) {
		if (datosConfigMemoria->debug)
			print_tlb_status();
		if (is_marco_in_tlb(marco)) {
			t_tlb* tlb_item = get_tlb_item_by_marco_number(marco);
			vaciar_pagina_tlb(get_page_by_ints(tlb_item->pid,tlb_item->pid));
		} else {
			if (pagina_is_in_tlb(pagina)) {
				if (datosConfigMemoria->debug)
					printf("\n\n la pagina ya esta en tlb\n\n");
				log_error(logger, "[ ERROR TLB ] La pagina %d del carpincho %d ya esta en tlb", pagina->pagina, pagina->carpincho_pid);
			}
			t_pagina* geting_out_of_tlb;
			if (strcmp(datosConfigMemoria->algoritmo_reemplazo_tlb, "LRU")==0) {
				geting_out_of_tlb = proxima_pagina_a_sacar_tlb_by_lru();
			} else {
				geting_out_of_tlb = proxima_pagina_a_sacar_tlb_by_fifo();
			}
			vaciar_pagina_tlb(geting_out_of_tlb);

			cargar_pagina_pagina_tlb(pagina, marco);
			if ( datosConfigMemoria->debug)
				print_tlb_status();
		}
	}
}

bool any_tlb_item_is_free() {
	bool tlbItemIsFree(t_tlb * tlb_item) {
		return tlb_item->pid == NULL;
	}
	t_list* lista = list_filter(tlb_list, (void*) tlbItemIsFree);

	return !list_is_empty(lista);
}

t_tlb* proxima_pagina_a_sacar_tlb_by_lru() {
	if (any_tlb_item_is_free()) {
		t_tlb* tlb = get_next_free_tlb_item();
		log_info("[ TLB ] No se requiere quitar ninguna pagina de la tlb ya que posee un espacio vacio en la entrada %d", tlb->entrada);
		return tlb;
	}

	bool sorted_by_enter_number(t_tlb * tlb1,t_tlb * tlb2) {
		return (tlb1->entrada) < (tlb2->entrada);
	}
	list_sort(tlb_list, (void*) sorted_by_enter_number);
	bool menorTimeStampTlb(t_tlb * tlb1,t_tlb * tlb2) {
		return (tlb1->timestamp) < (tlb2->timestamp);
	}
	list_sort(tlb_list, (void*) menorTimeStampTlb);
	t_tlb* tlb = list_get(tlb_list,0);
	if(datosConfigMemoria->debug) {
		printf(_YELLOW "Proxima pagina a sacar de la tlb es la pagina: %d del carpincho: %d en la entrada %d por el algoritmo lru\n" _RESET, tlb->pagina, tlb->pid, tlb->entrada);
	}
	log_info("[ TLB ]Proxima pagina a sacar de la tlb es la pagina: %d del carpincho: %d en la entrada %d por el algoritmo lru", tlb->pagina, tlb->pid, tlb->entrada);
	return tlb;
}

t_tlb* proxima_pagina_a_sacar_tlb_by_fifo() {
	if (any_tlb_item_is_free()) {
		t_tlb* tlb = get_next_free_tlb_item();
		log_info("[ TLB ] No se requiere quitar ninguna pagina de la tlb ya que posee un espacio vacio en la entrada %d", tlb->entrada);
		return tlb;
	}

	bool sorted_by_enter_number(t_tlb * tlb1,t_tlb * tlb2){
		return (tlb1->entrada) < (tlb2->entrada);
	}
	list_sort(tlb_list, (void*) sorted_by_enter_number);

	t_tlb* tlb = list_get(tlb_list, tlb_fifo_pointer);
	if(datosConfigMemoria->debug) {
		printf(_YELLOW "\n tlb_fifo_pointer %d\n" _RESET, tlb_fifo_pointer);
		printf(_YELLOW "\n Proxima pagina a sacar de la tlb es la pagina: %d del carpincho: %d en la entrada %d por el algoritmo fifo\n" _RESET, tlb->pagina, tlb->pid, tlb->entrada);
	}
	tlb_fifo_pointer = tlb_fifo_pointer + 1;
	if (tlb_fifo_pointer>(datosConfigMemoria->cantida_entradas_tlb-1))
		tlb_fifo_pointer = 0;

	log_info(logger,"[ TLB ] Proxima pagina a sacar de la tlb es la pagina: %d del carpincho: %d en la entrada %d por el algoritmo fifo", tlb->pagina, tlb->pid, tlb->entrada);

	return tlb;
}

void vaciar_pagina_tlb(t_tlb* tlb_item) {
	log_info(logger, "[ TLB ] Se quito de la entrada %d de la TLB la pagina %d del carpincho %d con el marco %d",
		tlb_item->entrada,
		tlb_item->pid,
		tlb_item->pagina,
		tlb_item->marco
	);
	tlb_item->pid = NULL;
	tlb_item->pagina = NULL;
	tlb_item->marco = -1;
}

void cargar_pagina_pagina_tlb(t_pagina* pagina, int marco) {
	t_tlb* tlb_item = get_next_free_tlb_item();
	tlb_item->pid = pagina->carpincho_pid;
	tlb_item->pagina = pagina->pagina;
	tlb_item->marco = marco;
	tlb_item->timestamp = obtenerTimeStampTLB();
	log_debug(logger," [ TLB ] Se cargo la pagina %d del carpincho %d en la entrada %d con el marco %d",
		 pagina->pagina,
		 tlb_item->pid,
		 tlb_item->entrada,
		 tlb_item->marco
	);
}

t_tlb* get_next_free_tlb_item() {
	bool tlbItemIsFree(t_tlb * tlb_item) {
		return tlb_item->pid == NULL;
	}
	t_list* lista = list_filter(tlb_list, (void*) tlbItemIsFree);
	t_tlb* tlb = list_get(lista,0);
	if (list_size(lista) == 0) {
		if(datosConfigMemoria->debug)
			printf(_RED "\n\n Se pidio un item libre en la tlb pero no hay ninguno \n\n" _RESET);
		log_error(logger,"[ ERROR TLB ] Se pidio un item libre en la tlb pero no hay ninguno ");
	}
	return tlb;

}

void print_tlb_status() {
	if (datosConfigMemoria->debug) {
		bool sorted_by_enter_number(t_tlb * tlb1,t_tlb * tlb2){
			return (tlb1->entrada) < (tlb2->entrada);
		}
		list_sort(tlb_list, (void*) sorted_by_enter_number);
		printf("\n Imprimo tlb \n\n");
		for(int i=0;i < 10; i++) {
//		for(int i=0;i < datosConfigMemoria->cantida_entradas_tlb; i++) {
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
		printf("---------------------------\n");
	}
}

void tlb_hit(t_pagina* pagina) {
	log_info(logger, "[ HIT TLB ] Tlb hit por el carpincho %d pagina %d marco %d", pagina->carpincho_pid, pagina->pagina, pagina->marco);
	t_carpincho* carpincho = get_carpincho(pagina->carpincho_pid);
	carpincho->tlb_hits = carpincho->tlb_hits + 1;
	tlb_hits_totales = tlb_hits_totales + 1;
	usleep(datosConfigMemoria->retardo_acierto_tlb*1000);
}

void tlb_miss(t_pagina* pagina) {
	log_info(logger, "[ MISS TLB ] Tlb miss por el carpincho %d pagina %d ", pagina->carpincho_pid, pagina->pagina);
	t_carpincho* carpincho = get_carpincho(pagina->carpincho_pid);
	carpincho->tlb_miss = carpincho->tlb_miss + 1;
	tlb_miss_totales = tlb_miss_totales + 1;
	usleep(datosConfigMemoria->retardo_fallo_tlb*1000);
}


void vaciar_tlb() {
	free_tlb();
	list_destroy(tlb_list);
	tlb_list = list_create();
}


/*
	tlb dump functions
*/

void reinciar_conteos_tlb() {
	tlb_miss_totales = 0;
	tlb_hits_totales = 0;
	list_iterate(carpinchos_list, (void *) limpiar_conteos_tlb_de_carpincho);
}

t_mensaje_dump* create_mensaje_dump(int carpincho_pid, int estado, int pagina, int marco)
{
	t_mensaje_dump* mensaje_dump = malloc(sizeof(t_mensaje_dump));
	mensaje_dump->carpincho_pid = carpincho_pid;
	mensaje_dump->entrada = list_size(mensajes_dump_list) + 1;
	mensaje_dump->estado = estado;
	mensaje_dump->pagina = pagina;
	mensaje_dump->marco = marco;
	log_info(logger, "[ TLB DUMP ] Se Agrego el mensaje carpincho_pid: %d entrada: %d estado: %d pagina: %d marco: %d \n",
		mensaje_dump->carpincho_pid,
		mensaje_dump->entrada,
		mensaje_dump->estado,
		mensaje_dump->pagina,
		mensaje_dump->marco
	);

	list_add(mensajes_dump_list,mensaje_dump);

	if(datosConfigMemoria->debug)
		printf("Se Agrego el mensaje carpincho_pid: %d entrada: %d estado: %d pagina: %d marco: %d \n",
			mensaje_dump->carpincho_pid,
			mensaje_dump->entrada,
			mensaje_dump->estado,
			mensaje_dump->pagina,
			mensaje_dump->marco
		);
	return mensaje_dump;
}

void clear_mensajes_dump()
{
	list_iterate(mensajes_dump_list, (void*) free_mensaje_dump);
	mensajes_dump_list = list_create();

	log_info(logger, "[ TLB DUMP ] Se vacio la lista de mensajes dump");
	if (datosConfigMemoria->debug) {
		printf("Se vacio la lista de mensajes dump \n");
		printf("Cantidad de mensajes en la tlb %d \n", list_size(mensajes_dump_list));
	}
}

void free_mensaje_dump(t_mensaje_dump* mensaje_dump){
	free(mensaje_dump);
}

void close_mensajes_dump() {
	list_iterate(mensajes_dump_list, (void*) free_mensaje_dump);
	list_destroy(mensajes_dump_list);

	log_info(logger, "[ TLB DUMP ] Close mensajes dump");
	if (datosConfigMemoria->debug)
		printf("Close mensajes dump \n");
}


/*
	 	    /\			//		     //			  ///////////  ///////////
		   //\\			//			 //			  //	   //  //
		  //  \\		//			 //			  //	   //  //
		 //    \\		//			 //			  //	   //  //
		////////\\		//			 //			  //	   //  //
	   //		 \\		//			 //			  //	   //  //
	  //		  \\	//			 //			  //	   //  //
	 //    		   \\	//			 //			  //	   //  //
	//				\\  ///////////  ///////////  ///////////  ///////////
					alloc functions
*/

void free_allocs_by_carpincho(t_carpincho* carpincho)
{
	list_iterate(carpincho->allocs, (void*) free);
	list_destroy(carpincho->allocs);
}

//Creada para debug y mostrar por pantalla los allocs de un carpincho.
void mostrar_allocs(int pid)
{
	/*
	log_debug(logger, "[DUMP-ALLOCS]--------");

	t_carpincho* carpincho = get_carpincho(pid);
	if(carpincho == NULL)
	{
		log_debug(logger, "[DUMP-ALLOCS] No existe el pid:%i no se puede dumpear allocs", pid);
		return;
	}

	log_debug(logger, "[DUMP-ALLOCS] PID: %i", pid);
	log_debug(logger, "[DUMP-ALLOCS] CANT ALLOCS: %i", list_size(carpincho->allocs));
	log_debug(logger, "[DUMP-ALLOCS] CANT PAG: %i", list_size(carpincho->paginas));

	int index = 0;
	while(index < list_size(carpincho->allocs))
	{
		t_alloc* alloc = list_get(carpincho->allocs, index);
		log_debug(logger, "[DUMP-ALLOCS] ALLOC - POS:%i - DIR_INICIO:%i - DIR_INICIO_DATO:%i - DIR_FIN:%i - TAM:%i - FREE:%i - PAG_INICIO:%i - PAG_FIN:%i",
				index, alloc->direccion,alloc->direccion + 9 ,
				alloc->direccion + 9 +  alloc->tamanio_data - 1,
				alloc->tamanio_data, alloc->isFree, alloc->direccion / datosConfigMemoria->tamanio_pagina, (alloc->direccion + 9 +  alloc->tamanio_data - 1) / datosConfigMemoria->tamanio_pagina);
		index++;
	}

	log_debug(logger, "[DUMP-ALLOCS]--------");
	*/
}

t_alloc* crear_alloc(uint32_t dir, int data_size, bool isFree)
{
	t_alloc* alloc = malloc(sizeof(t_alloc));

	alloc->direccion = dir;
	alloc->tamanio_data = data_size;
	alloc->isFree = isFree;

	return alloc;
}

int list_get_index_alloc(t_list* allocs, t_alloc* alloc)
{
	int index = 0;
	while(index < list_size(allocs))
	{
		t_alloc* alloc = list_get(allocs, index);
		if (alloc->direccion == alloc->direccion)
			return index;
	}

	return -1;
}

bool check_size_swamp(int carpinchoID, int cantPag)
{
	bool result = false;

	result = enviar_consulta_puede_escribir_en_swap(socket_swamp, carpinchoID, cantPag);

	return result;
}

check_alloc_respuesta check_alloc(t_carpincho* carpincho, int size)
{
	//Chequeamos si hay allocs libres que podamos usar.
	int espacioOcupado = 0;
	
	/*bool findAllocToFitSize(t_alloc* alloc) {
	return alloc->isFree == true && alloc->tamanio_data >= size;
	}
	t_alloc* alloc = list_find(carpincho->allocs, (void*) findAllocToFitSize);
	if (alloc != NULL)
		espacioOcupado = alloc->tamanio_data + 9;*/

	int index = 0;
	while(index < list_size(carpincho->allocs))
	{
		t_alloc* alloc = list_get(carpincho->allocs, index);
		if (alloc->isFree == true)
		{
			//Tenemos que poder albergar el HEAP
			if(alloc->tamanio_data >= size)
				return ALLOC_HEAP;
		}
		espacioOcupado = espacioOcupado + alloc->tamanio_data + 9;
		index++;
	}
	
	//Si no se puede porque no hay libres o porque los libres son pequenios, chequeamos si podemos crear uno nuevo.

	//Aca chequeamos si en el espacio disponible, total - ocupado podemos allocar, es decir al final de la ultima pagina hay espacio.
	//Si no tiene paginas xq recien arranca el carpincho, no va a entrar aca xq da negativo.
	int espacioTotal = list_size(carpincho->paginas) * datosConfigMemoria->tamanio_pagina;
	if (espacioTotal - espacioOcupado >= size + 9)
		return ALLOC_PAGINA;
		
	//Tenemos que chequear si podemos asignar/pedir paginas
	if(strcmp(datosConfigMemoria->tipo_asignacion, "FIJA") == 0)
	{
		//Aca si la asignacion es fija, es decir cierta cantidad de marcos fijo para cada carpincho, lo cual se traduce en un maximo de paginas para cada carpincho 
		//en la lista, no mas, es decir, no vamos a tener 10 paginas pero 4 marcos en memoria, si no directamente 4 paginas maximo, si no obligariamos a sobrepaginar.
		//Calculo el espacio teorico q podria pedir
		int espacio_teorico_solicitar = (datosConfigMemoria->marcos_por_carpincho - list_size(carpincho->paginas)) * datosConfigMemoria->tamanio_pagina;
				
		//Le sumo el espacio libre actual sin ningun heap
		int espacioLibreActual = (espacioTotal -  espacioOcupado);
				int disponibleTeorico = espacio_teorico_solicitar + espacioLibreActual;
				//Ahora ya se se cuanto espacio tengo actualmente, mas el teorico, si es posible podemos allocar
		if (disponibleTeorico >= size + 9)
		{
			// Hay espacio "Teorico", chequeamos con SWAmP si realmente puedo hacer la asignacion
						int cantPag = calcular_cantidad_paginas_necesaria(espacioLibreActual, size);
						if(check_size_swamp(carpincho->pid, cantPag) == true){
				return ALLOC_SOLICITAR_PAGINA;
			}
					}
	}
	else
	{
		//Podemos pedir lo que queramos va a depender si Swamp tiene espacio o no, asi q calculamos la cant de paginas necesarias.
		int espacioLibreActual = (espacioTotal -  espacioOcupado);
		int cantPag = calcular_cantidad_paginas_necesaria(espacioLibreActual, size);

		if(check_size_swamp(carpincho->pid, cantPag) == true)
			return ALLOC_SOLICITAR_PAGINA;
	}
		return ALLOC_SIN_ESPACIO;
}

uint32_t memalloc_asignar_en_heap_libre(t_carpincho* carpincho, uint32_t alloc_size)
{
	//Buscamos el heap que tenga espacio
	bool buscar_alloc_libre_con_espacio(t_alloc* alloc)
	{
		return alloc->isFree == true && alloc->tamanio_data >= alloc_size;
	}

	//Tenemos el heap, chequeamos si hay que truncar:
	t_list* allocsLibresConEspacio = list_filter(carpincho->allocs, (void*)buscar_alloc_libre_con_espacio);
	t_alloc* alloc = list_get(allocsLibresConEspacio, 0);

	if(alloc->tamanio_data > alloc_size)
	{
		//Que pasa si el nuevo truncado es mas chico q lo minimo de metadadata + 1 Byte de datos, no truncamos
		if((alloc->tamanio_data - alloc_size) <= 9)
		{
			log_debug(logger,
					_BLUE "[MEMALLOC] - Carpincho:%i - Size:%i - ESTADO: No se trunca porque el truncado no tiene el espacio minimo para la metadata" _RESET,
					carpincho->pid, alloc_size);
		}
		else
		{
			//Truncamos ese en 2, creando uno nuevo con la data libre. Nos queda el alloc original, y el nuevo alloc a derecha.
			//La direccion nueva es el alloc origina, mas 9 bytes, mas el size del nuevo alloc

			log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - Size:%i - ESTADO: Truncando Alloc" _RESET, carpincho->pid, alloc_size);

			t_alloc* nuevoAlloc = crear_alloc( (alloc->direccion + alloc_size + 9),  alloc->tamanio_data - alloc_size - 9, true);

			//Agregamos de forma ordenada el nuevo alloc, va a estar a la derecha del alloc original
			int indiceAlloc = list_get_index_alloc(carpincho->allocs, alloc);

			//Agregamos el alloc truncado
			int indiceNuevoAllocTruncado = indiceAlloc + 1;
			list_add_in_index(carpincho->allocs, indiceNuevoAllocTruncado, nuevoAlloc);

			//Ahora tenemos que actualizar el alloc
			alloc->tamanio_data= alloc_size;
			alloc->isFree = false;

			log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - ESTADO: Persistiendo metada heap del alloc truncado" _RESET, carpincho->pid);	
			persistir_metadata_heap(carpincho, nuevoAlloc);

			return alloc->direccion + 9;
		}
	}

	//No hace falta crear otro heap porque el tamanio es el mismo.
	alloc->isFree = false;
	return alloc->direccion + 9;
}

uint32_t memalloc_asignar_en_pagina_con_espacio(t_carpincho* carpincho, uint32_t alloc_size)
{
	//Tengo espacio actual para agregar el alloc al final
	int cantPaginasDelCarpincho = list_size(carpincho->paginas);

	//Calculamos de esa pagina cual es la ultima direccion logica libre:

	uint32_t dirLogica_inicio_dato;
	if(list_size(carpincho->allocs) > 0)
	{
		//Obtenemos la proxima direccion logica libre, sabiendo que
		//Direccion es el inicio, que mas 9 nos da donde empieza la data, y + el tamania nos da el ultimo libre.
		t_alloc* ultimoAlloc = list_get(carpincho->allocs, list_size(carpincho->allocs)-1);
		dirLogica_inicio_dato = (ultimoAlloc->direccion + 9 ) + ultimoAlloc->tamanio_data + 9;
	}
	else
	{
		//Si no hay los datos arrancan en la direccion 9, xq los primeros 8 son de metadata.
		dirLogica_inicio_dato = 9;
	}

	//Creamos el alloc y lo agregamos a la lista
	//Marcamos el inicio del alloc, que es el del dato .9 por la metadata.
	uint32_t alloc_inicio = dirLogica_inicio_dato - 9;
	t_alloc* nuevoAlloc = crear_alloc(alloc_inicio, alloc_size, false);
	list_add(carpincho->allocs, nuevoAlloc);
//	agregar_alloc_a_carpincho(carpincho, nuevoAlloc);

	//Retornamos el inicio del dato SIEMPRE.
	return dirLogica_inicio_dato;
}

uint32_t memalloc_asignar_en_paginas_nuevas(t_carpincho* carpincho, uint32_t alloc_size)
{
	//Calculamos el espacio libre actual
	int espacioOcupado = 0;
	int index = 0;
	while(index < list_size(carpincho->allocs))
	{
		t_alloc* alloc = list_get(carpincho->allocs, index);
		espacioOcupado = espacioOcupado + alloc->tamanio_data + 9;
		index++;
	}

	int espacioTotal = list_size(carpincho->paginas) * datosConfigMemoria->tamanio_pagina;
	int espacioLibre = espacioTotal - espacioOcupado;
	printf(_RED"\n 8 \n"_RESET);
	printf(_RED"\nENTRO EN  'calcular_cantidad_paginas_necesaria(espacioLibre, alloc_size);'\n"_RESET);

	int cantPag = calcular_cantidad_paginas_necesaria(espacioLibre, alloc_size);
	
	//Asignamos las paginas al carpincho
	printf(_RED"\n 9 \n"_RESET);
	printf(_RED"\n carpincho: %d\n"_RESET, carpincho->pid);
	printf(_RED"\n cantPag: %d\n"_RESET, cantPag);
	printf(_RED"\nENTRO EN  'asignar_paginas_carpincho(carpincho, cantPag);'\n"_RESET);
	asignar_paginas_carpincho(carpincho, cantPag);
	
	printf(_RED"\n 10 \n"_RESET);
	printf(_RED"\nENTRO EN  ' memalloc_asignar_en_pagina_con_espacio(carpincho, alloc_size);'\n"_RESET);
	//Una vez asignadas las paginas, nos comportamos como el caso anterior, donde tenemos espacio para allocar al final
	return memalloc_asignar_en_pagina_con_espacio(carpincho, alloc_size);
}

t_alloc* obtener_alloc_de_un_carpincho(t_carpincho* carpincho, uint32_t abs)
{
	for(int i = 0; i< list_size(carpincho->allocs) ;i++)
	{
		//Itereamos los allocs para ver si la direccion que ponemos esta dentro de los rangos de cada uno.
		//Calculamos el inicio de cada alloc, sabiendo que es el inicio + 9 bytes de la meta data
		t_alloc* alloc = list_get(carpincho->allocs, i);
		uint32_t inicio_dato = alloc->direccion + 9;
		uint32_t fin_dato = inicio_dato + alloc->tamanio_data -1;

		//Si la direccion logica, esta en el rango de inicio y fin del alloc podemos escribir, si no, seguimos iterando
		if(abs >= inicio_dato && abs <= fin_dato)
		{
			return alloc;
		}
	}

	return NULL;
}

t_alloc* obtener_alloc_con_inicio_de_direccion_de_dato_de_un_carpincho(t_carpincho* carpincho, uint32_t abs)
{
	for(int i = 0; i< list_size(carpincho->allocs) ;i++)
	{
		t_alloc* alloc = list_get(carpincho->allocs, i);
		if(abs == (alloc->direccion + 9))
			return alloc;
	}

	return NULL;
}

int obtener_indice_de_alloc(t_list* allocs, t_alloc* alloc)
{

	for(int i = 0; i<= list_size(allocs); i++)
	{

		t_alloc* alloc_lista = list_get(allocs, i);

		if (alloc_lista->direccion == alloc->direccion)
		{
				return i;
		}

	}

	return -1;
}

/*
	//		//  //////////		   //\\			//////\\     ///////////
	//		//  //				  //  \\		//	   \\	 //
	//		//  //				 //    \\		//	    \\	 //
	//		//  //				//      \\		//		//	 //
	//////////  ///////		   //////\\\\\\		//	   //	 ///////////
	//		//  //			  //		  \\	///////				  //
	//		//  //			 //			   \\	//					  //
	//		//  //			//				\\	//					  //
	//		//  ////////// //				 \\ //			 ///////////
					heaps functions
*/

t_pagina_accion* obtener_pagina_accion_by_indice(t_list* lista, uint32_t indice_pag)
{
	for(int i = 0; i<=list_size(lista) -1; i++)
	{
		t_pagina_accion* pag = list_get(lista, i);
		if(pag->pagina == indice_pag)
			return pag;
	}

	return NULL;
}

void escribir_heap_memoria_fisica(t_heap_metadata* heap, uint32_t pos_log_abs, t_carpincho* carpincho)
{
	char* valor = heap;

	t_list* escrituras = list_create();

	for(int i = 0; i<=8; i++)
	{
		uint32_t pag = (pos_log_abs + i) / datosConfigMemoria->tamanio_pagina;
		int offset = (pos_log_abs + i)  % datosConfigMemoria->tamanio_pagina;

		t_pagina_accion* node = obtener_pagina_accion_by_indice(escrituras, pag);
		if(node ==  NULL) {
			node = malloc(sizeof(t_pagina_accion));
			node->pagina = pag;
			node->direcciones = list_create();
			list_add(escrituras, node);
		}

		list_add(node->direcciones, offset);
	}

	int sizeAnterior = 0;
	for(int i = 0; i<= list_size(escrituras) - 1; i++)
	{
		//Iteramos llamando a la escritura de cache
		t_pagina_accion* escritura = list_get(escrituras, i);

		int sizeValAcotdado = list_size(escritura->direcciones);

		char* valorAcotado = malloc(sizeof(char)*sizeValAcotdado);

		memcpy(valorAcotado, valor + sizeAnterior, sizeValAcotdado);

		t_pagina* page = get_page(carpincho, escritura->pagina+1);

		escribir_en_pagina(page, valorAcotado, sizeValAcotdado, list_get(escritura->direcciones, 0));

		free(valorAcotado);

		sizeAnterior = sizeValAcotdado;
	}

	for(int k = 0; k<= list_size(escrituras) - 1; k++)
	{
		t_pagina_accion* borrame = list_get(escrituras, k);
		list_destroy(borrame->direcciones);
		free(borrame);
	}
	list_destroy(escrituras);
}

void persistir_metadata_heap(t_carpincho* carpincho, t_alloc* unAclloc)
{
	int indiceAlloc = obtener_indice_de_alloc(carpincho->allocs, unAclloc);

	int dirPrev = 0;
	if(indiceAlloc - 1 >= 0)
	{
	    t_alloc* allocPrevio = list_get(carpincho->allocs, indiceAlloc - 1);
	    dirPrev = allocPrevio->direccion;
	}

	int dirNext = 0;
	if(indiceAlloc + 1 <= list_size(carpincho->allocs) -1)
	{
	    t_alloc* allocNextNuevo = list_get(carpincho->allocs, indiceAlloc + 1);
	    dirNext = allocNextNuevo->direccion;
	}

	//Creamos un heap para escribirlo en CACHE
	t_heap_metadata* heap = malloc(sizeof(t_heap_metadata));
	heap->isFree = unAclloc->isFree;
	heap->prevAlloc = dirPrev;
	heap->nextAlloc = dirNext;

	//Escribimos el heap.
	escribir_heap_memoria_fisica(heap, unAclloc->direccion, carpincho);

	//Liberamos el buffer
	free(heap);
}


/*
	//\\      ////	//////////	//\\	   //	//////////		   //\\				////////// //////////	//////////
	// \\    // //	//			// \\	   //	//				  //  \\				//	   //			//
	//  \\  //  //	//			//	\\	   //	//				 //	   \\				//	   //			//
	//   \\//   //	//			//	 \\	   //	//				//		\\				//	   //			//
	//          //  ////////    //	  \\   //	//////////	   //////\\\\\\		////	//	   ////////		//////////
	//          //	//			//	   \\  //			//	  //		  \\	//		//	   //					//
	//          //	//			//		\\ //			//	 //			   \\	//		//	   //					//
	//          //	//////////	//		 \\//	//////////	//				\\	//////////	   //////////	//////////
					mensajes functions
*/

bool validar_direccion_logica(t_carpincho* carpincho, uint32_t abs)
{
	//Primero validado que haya coherencia, es decir que esa direccion sea una pagina y un offset valido.
	uint32_t numPag = abs / datosConfigMemoria->tamanio_pagina;
	uint32_t offset = abs % datosConfigMemoria->tamanio_pagina;

	//Vemos si esa pag existe en el carpincho
	if(numPag > list_size(carpincho->paginas) - 1 )
	{
		return false;
	}

	//Vemos si el offset tiene sentido respecto del tamanio de pagina.
	if(offset >  datosConfigMemoria->tamanio_pagina -1)
	{
		return false;
	}

	return true;
}

bool validar_direccion_logica_con_size(t_carpincho* carpincho, uint32_t abs, int size)
{
	for(uint32_t i = 0; i<size; i++)
	{
		if(validar_direccion_logica(carpincho, abs + i) == false)
			return false;
	}
	return true;
}

bool validar_direccion_logica_escritura(t_carpincho* carpincho, uint32_t abs)
{
	//Validamos que la direccion pertenece al carpincho
	if(validar_direccion_logica(carpincho, abs) == false)
		return false;

	//Chequeamos que sea una direccion de escritura y usamos el size, porque no queremos que se escriba la mitad de memoria
	//Y recien cuando escribe en la dir invalida, ahi tire error, lo atrapamos antes de empezar a escribir
	int i = 0;
	while(i < list_size(carpincho->allocs))
	{
		//Itereamos los allocs para ver si la direccion que ponemos esta dentro de los rangos de cada uno.
		//Calculamos el inicio de cada alloc, sabiendo que es el inicio + 9 bytes de la meta data
		t_alloc* alloc = list_get(carpincho->allocs, i);
		uint32_t inicio = alloc->direccion + 9;
		uint32_t fin = inicio + alloc->tamanio_data -1;

		//Si la direccion logica, esta en el rango de inicio y fin del alloc podemos escribir, si no, seguimos iterando.
		if(abs >= inicio && abs <= fin)
			return true;

		i++;
	}

	//Si la direccion no corresponde a ningun alloc a la parte de datos, no es valida.
	return false;
}

bool validar_direccion_logica_escritura_con_size(t_carpincho* carpincho, uint32_t abs, int size)
{
	for(uint32_t i = 0; i<size; i++)
	{
		if(validar_direccion_logica_escritura(carpincho, abs + i) == false)
		{
			return false;
		}
	}
	return true;
}

int memory_memalloc(int carpinchoId, uint32_t alloc_size, uint32_t* direccion)
{
	check_alloc_respuesta check_status = 100;
	status_memory_op result = 100;

	//Obtenemos el carpincho
	t_carpincho* carpincho = get_carpincho(carpinchoId);

	log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - SIZE:%i" _RESET, carpinchoId, alloc_size);

	if (carpincho == NULL)
	{
		log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - SIZE:%i - Estado: Carpincho no existe" _RESET, carpinchoId, alloc_size);

		//El carpincho no existe asi que tenemos que crearlo pero antes hay que chequear si hay espacio.
		//No podemos llamar a check_alloc, porque esta pensada para un carpincho que existe.
		//No importa comprobar el espacio ocupado o disponible, ni si la asig fija le permite xq esta en cero, solo importar chequear con SWAmP

		//Chequeamos cuantas paginas tenemos, en este caso tenemos 0 espacio disponible en las paginas ya alocadas.
		int cantPag = calcular_cantidad_paginas_necesaria(0, alloc_size);

		//Usamos el id xq no creamos aun al carpincho hasta estar seguros q vamos a poder darle memoria.
		if(check_size_swamp(carpinchoId, cantPag) == true)
		{
			//Si hay espacio, entonces lo que hacemos es marcar la estrategia pedir a SWAMp y creamos el carpincho.
			carpincho = create_carpincho(carpinchoId);
			check_status = ALLOC_SOLICITAR_PAGINA;
		}
		else
		{
			//SI no hay espacio en SWAMp, marcamos esa estrategia.			
			check_status = ALLOC_SIN_ESPACIO;
		}
	}
	else
	{
		//Chequeamos que de pueda hacer el alloc necesario
		luz_roja();
		check_status = check_alloc(carpincho, alloc_size);
		log_info(logger,"[ CHECK ALLOC ] RESULTADO DE CHECK ALLOC ES", check_status);
		luz_verde();
	}
	printf(_RED"\nEL RESULTADO DE CHECK ALLOC ES\n"_RESET, check_status);
	switch(check_status)
	{
		//No podemos hacer alloc para ese carpincho, para ese size
		case ALLOC_SIN_ESPACIO:
			result = SIN_ESPACIO;
			log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - SIZE:%i - Sin Espacio" _RESET, carpinchoId, alloc_size);
			break;
		//Podemos hacer alloc, en un heap creado pero libre
		case ALLOC_HEAP:
			printf(_RED"\n3\n"_RESET);
			result = EXITO;
			*direccion = memalloc_asignar_en_heap_libre(carpincho, alloc_size);
			log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - SIZE:%i - Estrategia de alloc: HEAP - DIR: %i" _RESET, carpinchoId, alloc_size, *direccion);
			break;
		//Podemos hacer alloc, en un heap que tenemos que crear
		case ALLOC_PAGINA:
			printf(_RED"\n4\n"_RESET);
			result = EXITO;
			*direccion = memalloc_asignar_en_pagina_con_espacio(carpincho, alloc_size);
			log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - SIZE:%i - Estrategia de alloc: Pagina - DIR: %i" _RESET, carpinchoId, alloc_size, *direccion);
			break;
		//Podemos hacer alloc, pero tenemos que pedir paginas adicionales y crear el heap.
		case ALLOC_SOLICITAR_PAGINA:			
			*direccion = memalloc_asignar_en_paginas_nuevas(carpincho, alloc_size);
			log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - SIZE:%i - Estrategia de alloc: Nueva Pagina - DIR: %i" _RESET, carpinchoId, alloc_size, *direccion);
			result = EXITO;
			break;
		default:
			printf(_RED"\n6\n"_RESET);
			log_debug(logger,_RED "[MEMALLOC] - Carpincho:%i - SIZE:%i - no se ha podido realizar la asignacion ABORTANDO" _RESET, carpinchoId, alloc_size);
			getchar();
			exit(EXIT_FAILURE);
	}
	
	if (datosConfigMemoria->debug) 
		mostrar_allocs(carpinchoId);

	if(result == EXITO)
	{
		log_debug(logger,_BLUE "[MEMALLOC] - Carpincho:%i - ESTADO: Persistiendo metada heap" _RESET, carpinchoId);	

		t_alloc* alloc = obtener_alloc_de_un_carpincho(carpincho, *direccion);
		printf(_RED"\nLLEGO A PERSISTIR MEDATA\n"_RESET);
		persistir_metadata_heap(carpincho, alloc);
		printf(_RED"\nPASO PERSISTIR METADATA\n"_RESET);
	}
	
	return result;
}

int memory_memwrite(int carpinchoId, uint32_t pos_log_abs , uint32_t size, char* valor)
{
	log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - DIR:%i - SIZE:%i " _RESET, carpinchoId, pos_log_abs,size);

	t_carpincho* carpincho = get_carpincho(carpinchoId);

	if(carpincho == NULL)
	{
		log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - DIR:%i - ESTADO: Carpincho no existe" _RESET, carpinchoId, pos_log_abs);
		return DIRECCION_INVALIDA;
	}
	
	//Validamos la direccion respecto a los permisos de escritura, y a su vez el desplazamiento usando el size
	if(validar_direccion_logica_escritura_con_size(carpincho, pos_log_abs, size) == false)
	{
		log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - DIR:%i - ESTADO: En esa direccion no se puede escribir para el size dado" _RESET,
				carpinchoId, pos_log_abs);

		return DIRECCION_INVALIDA;
	}

	//Obtenemos el alloc
	t_alloc* alloc = obtener_alloc_de_un_carpincho(carpincho, pos_log_abs);

	//Chequeamos si podemos  escribir en ese inicio con ese size
	uint32_t dir_fin_alloc = alloc->direccion + 9 + alloc->tamanio_data - 1;
	uint32_t bytesDisponibles = dir_fin_alloc - pos_log_abs + 1;
	if(bytesDisponibles < size)
	{
		log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - DIR:%i - ESTADO: Se intenta escribir con un size mas grande al del heap en la pos enviada" _RESET,
			carpinchoId, pos_log_abs);
		return DIRECCION_INVALIDA;
	}
		
	log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - ESTADO: Calculando cantidad de escrituras" _RESET, carpinchoId);	
	t_list* escrituras = list_create();
	for (int i = 0; i<size; i++)
	{
		//Generamos la estructura para contabilizar la escritura
		uint32_t pag = (pos_log_abs + i) / datosConfigMemoria->tamanio_pagina;
		int offset = (pos_log_abs + i) % datosConfigMemoria->tamanio_pagina;

		t_pagina_accion* node = obtener_pagina_accion_by_indice(escrituras, pag);
		if(node ==  NULL)
		{
			node = malloc(sizeof(t_pagina_accion));
			node->pagina = pag;
			node->direcciones = list_create();
			list_add(escrituras, node);
		}

		list_add(node->direcciones, offset);
	}
	
	log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - ESTADO:  Escribiendo en memoria fisica, cant de escrituras:%i" _RESET, carpinchoId, list_size(escrituras));
	
	int sizeAnterior = 0;
	for(int i = 0; i<= list_size(escrituras) - 1; i++)
	{
		t_pagina_accion* escritura = list_get(escrituras, i);

		int sizeValAcotdado = list_size(escritura->direcciones);

		char* valorAcotado = malloc(sizeof(char)*sizeValAcotdado);

		memcpy(valorAcotado, valor + sizeAnterior, sizeValAcotdado);

		log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - ESTADO: Escribiendo en memoria fisica - Pag:%i - Size:%i - Valor:%s" _RESET,
		 carpinchoId, escritura->pagina, sizeValAcotdado, valorAcotado);

		t_pagina* ptro_pag = list_get(carpincho->paginas, escritura->pagina);

		escribir_en_pagina(ptro_pag, valorAcotado, sizeValAcotdado, list_get(escritura->direcciones, 0));
		free(valorAcotado);

		sizeAnterior = sizeValAcotdado;
	}

	log_debug(logger,_BLUE "[MEMWRITE] - Carpincho:%i - ESTADO: Liberando estructuras auxiliares" _RESET, carpinchoId);
	for(int k = 0; k<= list_size(escrituras) - 1; k++)
	{
		t_pagina_accion* borrame = list_get(escrituras, k);
		list_destroy(borrame->direcciones);
		free(borrame);
	}

	list_destroy(escrituras);
	
	if (datosConfigMemoria->debug) 
		mostrar_allocs(carpinchoId);

	return EXITO;
}

int memory_memread(int carpinchoId, uint32_t pos , uint32_t size, char* buffer)
{
	log_debug(logger,_BLUE "[MEMREAD] - Carpincho:%i - DIR:%i - SIZE:%i" _RESET, carpinchoId, pos,size);

	t_carpincho* carpincho = get_carpincho(carpinchoId);

	if(carpincho == NULL)
	{
		log_debug(logger,_BLUE "[MEMREAD] - Carpincho:%i - DIR:%i - ESTADO: Carpincho no existe" _RESET, carpinchoId, pos);
		return DIRECCION_INVALIDA;
	}

	//Validamos que sea una direccion q se pueda escribir, si se puede escribir, se puede leer.
	if(validar_direccion_logica_escritura_con_size(carpincho, pos, size) == false)
	{
		log_debug(logger,_BLUE "[MEMREAD] - Carpincho:%i - DIR:%i - ESTADO: En esa direccion no se puede leer para el size dado" _RESET,
				carpinchoId, pos);

		return DIRECCION_INVALIDA;
	}

	/*uint32_t pag = pos / datosConfigMemoria->tamanio_pagina;
	int offset = pos % datosConfigMemoria->tamanio_pagina;*/

	t_list* lecturas = list_create();
	for (int i = 0; i<size; i++)
	{
		//Generamos la estructura para contabilizar la escritura
		uint32_t pag = (pos+i) / datosConfigMemoria->tamanio_pagina;
		int offset = (pos+i) % datosConfigMemoria->tamanio_pagina;

		t_pagina_accion* node = obtener_pagina_accion_by_indice(lecturas, pag);;
		if(node ==  NULL)
		{
			node = malloc(sizeof(t_pagina_accion));
			node->pagina = pag;
			node->direcciones = list_create();
			list_add(lecturas, node);
		}

		list_add(node->direcciones, offset);
	}
	
	log_debug(logger,_BLUE "[MEMREAD] - Carpincho:%i - ESTADO: Buscando en la cache" _RESET, carpinchoId);
	
	int sizeAnterior = 0;
	for(int i = 0; i<= list_size(lecturas) - 1; i++)
	{
		//Iteramos llamando a la escritura de cache
		t_pagina_accion* lectura = list_get(lecturas, i);

		int tamanio = list_size(lectura->direcciones);

		printf("\n\n\nLa pagina es %i\n\n\n", (t_pagina) (list_get(carpincho->paginas, lectura->pagina))->pagina);
		fflush(stdout);

		char* resultadoParcial = leer_en_pagina(list_get(carpincho->paginas, lectura->pagina), tamanio, list_get(lectura->direcciones, 0));

		memcpy(buffer + sizeAnterior, resultadoParcial, tamanio);

		sizeAnterior = tamanio;

		free(resultadoParcial);
	}

	//buffer = leer_en_pagina(list_get(carpincho->paginas, 0), 1, list_get(offset, 0));

	log_debug(logger,_BLUE "[MEMREAD] - Carpincho:%i - DIR:%i - SIZE:%i - ESTADO: Liberando estructuras administrativas" _RESET, carpinchoId, pos,size);

	//Liberamos
	for(int k = 0; k<= list_size(lecturas) - 1; k++)
	{
		t_pagina_accion* borrame = list_get(lecturas, k);
		list_destroy(borrame->direcciones);
		free(borrame);
	}
	list_destroy(lecturas);

	return EXITO;
}

bool alloc_esta_en_pagina(t_alloc* alloc,int num_pag)
{
	uint32_t dir = alloc->direccion;
	int data_size = alloc->tamanio_data;

	//Llenamos la lista de paginas de ese alloc
	for(int i= 0; i<= dir + 9 + data_size - 1; i++)
	{
		uint32_t dirActual = dir+ i;
		int pagina = dirActual / datosConfigMemoria->tamanio_pagina;

		if(pagina == num_pag)
			return true;
	}

	return false;
}

t_list* obtener_todos_los_allocs_de_carpincho_de_una_pagina(t_carpincho* carpincho, int indexPag)
{
	t_list* allocs = list_create();

	for(int i = 0; i <= list_size(carpincho->allocs) - 1 ; i++)
	{
		t_alloc* alloc = list_get(carpincho->allocs, i);

		if(alloc_esta_en_pagina(alloc, indexPag))
		{
			list_add(allocs, alloc);
		}
	}

	return allocs;
}

bool todos_los_allocs_estan_libres(t_list* allocs)
{
	if(list_size(allocs) == 0)
		return false;

	bool result = true;

	for(int i = 0; i <= list_size(allocs) - 1 ; i++)
	{
		t_alloc* alloc = list_get(allocs, i);

		if(alloc->isFree == false)
			return false;
	}

	return result;
}

void compactar_pagina(t_carpincho* carpincho, t_pagina* pag, t_list* allocs_de_una_pagina)
{
	int acumEspacioLiberado = 0;

	int indicePag = pag->pagina - 1;

	for(int i_alloc = 0; i_alloc <= list_size(allocs_de_una_pagina) - 1; i_alloc++)
	{
		t_alloc* alloc_borrar = list_get(allocs_de_una_pagina, i_alloc);
		acumEspacioLiberado = alloc_borrar->tamanio_data + 9;
		list_remove(carpincho->allocs, obtener_indice_de_alloc(carpincho->allocs, alloc_borrar));
		free(alloc_borrar);
	}

	log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - ESTADO: Se ha eliminado la PAG:%i" _RESET, carpincho->pid, indicePag);

	list_remove(carpincho->paginas, indicePag);

	free(list_get(carpincho->paginas, indicePag));//TODO habria que hacer algo con SWAMP con la pagina a liberar?

	//Necesitamos actualizar todos los allocs afectadas
	//Ahora bien para saber cuales son, decimos q son todos aquellos, a derecha
	//Para eso necesitamos de la pagina actual, obtener el ultimo alloc libre, 
	//y el PROXIMO es a partir del cual tenemos que hacer cambios.
	//Y asu vez como se borraron los allocs ya, el proximo ahora tiene un indice meno hay que restar
	//Nos queda al valor inicial.
	int indice_ultimo_alloc_libre = list_size(allocs_de_una_pagina) - 1;

	//Actualizamos las direcciones de los allocs ya que eliminamos una o mas paginas y sus allocs, ahora debemos actualizar con las nuevas direcciones
	if(acumEspacioLiberado > 0)
	{
		for(int i = indice_ultimo_alloc_libre; i<= list_size(carpincho->allocs)-1; i++)
		{
			t_alloc* all_mod = list_get(carpincho->allocs, i);
			all_mod->direccion = all_mod->direccion - acumEspacioLiberado;
		}
	}
}

int compactar(t_carpincho* carpincho)
{
	//Iteramos las paginas buscando aquellas que tenga solamente allocs free
	for(int i = 0; i <= list_size(carpincho->paginas) - 1 ; i++)
	{
		t_pagina* pag = list_get(carpincho->paginas, i);
		t_list* allocs_de_una_pagina = obtener_todos_los_allocs_de_carpincho_de_una_pagina(carpincho, i);

		if(todos_los_allocs_estan_libres(allocs_de_una_pagina) == true)
		{
			compactar_pagina(carpincho, pag, allocs_de_una_pagina);
			return 1;
		}

		list_destroy(allocs_de_una_pagina);

		return 0;
	}
}

int memory_memfree(int carpinchoId, uint32_t pos)
{
	log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i " _RESET, carpinchoId, pos);

	t_carpincho* carpincho = get_carpincho(carpinchoId);

	if(carpincho == NULL)
	{
		log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i - ESTADO: Carpincho no existe" _RESET, carpinchoId, pos);
		return DIRECCION_INVALIDA;
	}

	if(validar_direccion_logica(carpincho, pos) == false)
	{
		log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i - ESTADO:Direccion invalida, no corresponde a una pagina del carpincho" _RESET, carpinchoId, pos);
		return DIRECCION_INVALIDA;
	}

	//Traemos el alloc cuya direccion de dato coincida con la q piden liberar.
	t_alloc* alloc = obtener_alloc_con_inicio_de_direccion_de_dato_de_un_carpincho(carpincho, pos);

	if(alloc == NULL)
	{
		log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i - ESTADO: No hay alloc a liberar con esa direccion" _RESET, carpinchoId, pos);
		return DIRECCION_INVALIDA;
	}

	if (alloc->isFree == true)
	{
		log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i - ESTADO: Este alloc ya esta libre" _RESET, carpinchoId, pos);
		return DIRECCION_INVALIDA;
	}

	//Lo marcamos como libre.
	alloc->isFree = true;

	//Ahora tenemos que unficar los allocs izq y der si estan free
	int indiceAlloc = obtener_indice_de_alloc(carpincho->allocs, alloc);
	bool unifique_izq = false;

	if(indiceAlloc - 1 >= 0)
	{
		t_alloc* allocPrev = list_get(carpincho->allocs, indiceAlloc - 1);

		if(allocPrev->isFree == true)
		{
			log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i - ESTADO: Unificando a Izq" _RESET, carpinchoId, pos);

			//Tengo que unifica con el prev le asignamos su direccion, le sumamos su tamanio y el tamanio de los 9 bytes de su metadata
			alloc->direccion = allocPrev->direccion;
			alloc->tamanio_data = allocPrev->tamanio_data + alloc->tamanio_data + 9;

			//Lo borramos de la lista
			list_remove(carpincho->allocs, indiceAlloc - 1);
			free(allocPrev);

			//Marco que unifique a iz
			unifique_izq = true;
		}
	}

	if(unifique_izq == true)
	{
		//Si unifique a izq el indiceAlloc, orifinal por ejemplo 2, ya no es dos si no es 2 -1 = 1
		//Por eso le restamos uno
		indiceAlloc--;
	}

	if(indiceAlloc + 1 <= list_size(carpincho->allocs) -1)
	{
		t_alloc* allocNext = list_get(carpincho->allocs, indiceAlloc + 1);

		if(allocNext->isFree == true)
		{
			log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i - ESTADO: Unificando a Der" _RESET, carpinchoId, pos);

			//Tengo que unifica con el next, la dir no cambia, si le sumamos su tamanio y el tamanio de los 9 bytes de su metadata
			alloc->tamanio_data = allocNext->tamanio_data + alloc->tamanio_data + 9;

			//Lo borramos de la lista
			list_remove(carpincho->allocs, indiceAlloc + 1);
			free(allocNext);
		}
	}

	log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - ESTADO: Persistiendo metada heap" _RESET, carpinchoId);	
	persistir_metadata_heap(carpincho, alloc);

	log_debug(logger,_BLUE "[MEMFREE] - Carpincho:%i - DIR:%i - ESTADO: Intentando compactar " _RESET, carpinchoId, pos);
	//Cada vez q se compacta una pagina se mueve, todo y no podemos seguir la ejecucion normal, tenemos q abortar y volver a chequear
	//Chequeamos recursivamente hasta q el compactador no compacte mas y ahi nos detenemos.
	//Otra opcion seria analizar todo y dps hacer todos los cambios juntos pero esta mas complicado.
	int resultado_compactar = compactar(carpincho);
	while(resultado_compactar == 1)
	{
		resultado_compactar = compactar(carpincho);
	}
	
	if (datosConfigMemoria->debug) 
		mostrar_allocs(carpinchoId);

	return EXITO;
}

int memory_memfree_all(int carpinchoId)
{
	log_debug(logger,_BLUE "[MEMFIN] - Carpincho:%i" _RESET, carpinchoId);

	t_carpincho* carpincho = get_carpincho(carpinchoId);

	if(carpincho == NULL)
	{
		log_debug(logger,_BLUE "[MEMFIN] - Carpincho:%i - ESTADO: Carpincho no existe" _RESET, carpinchoId);
		return DIRECCION_INVALIDA;
	}

	//Eliminamos al carpincho de la lista.
	log_debug(logger,_BLUE "[MEMFIN] - Carpincho:%i - ESTADO: Eliminando Carpincho de la lista" _RESET, carpinchoId);
	
	int index = -1;
	for(int i = 0; i<= list_size(carpinchos_list) -1; i++)
	{
		t_carpincho* carp = list_get(carpinchos_list, i);

		if(carp->pid == carpinchoId)
		{
			index = i;
		}
	}

	list_remove(carpinchos_list, index);

	log_debug(logger,_BLUE "[MEMFIN] - Carpincho:%i - ESTADO: Liberando al carpincho" _RESET, carpinchoId);
	
	//Free al carpincho y a sus cosas
	close_carpincho(carpincho);

	if (datosConfigMemoria->debug) 
		mostrar_allocs(carpinchoId);

	return EXITO;
}

/*
	utils funtions
*/
unsigned long long obtenerTimeStamp() {
	/*
	struct timeval tv;

	gettimeofday(&tv, NULL);

	unsigned long long millisecondsSinceEpoch =
	    (unsigned long long)(tv.tv_sec) * 1000 +
	    (unsigned long long)(tv.tv_usec) / 1000;
	return millisecondsSinceEpoch;
	*/
	unsigned long long result = (unsigned long long*) contador_timestamp;
	if (contador_timestamp<10)
		printf("\n\n\ncontador_timestamp %d\n\n\n",contador_timestamp);
	contador_timestamp++;
	return result;
}
unsigned long long obtenerTimeStampTLB() {
	/*
	struct timeval tv;

	gettimeofday(&tv, NULL);

	unsigned long long millisecondsSinceEpoch =
	    (unsigned long long)(tv.tv_sec) * 1000 +
	    (unsigned long long)(tv.tv_usec) / 1000;
	return millisecondsSinceEpoch;
	*/
	unsigned long long result = (unsigned long long*) contador_timestamp_tlb;
	if (contador_timestamp_tlb<10)
		printf("\n\n\ncontador_timestamp_tlb %d\n\n\n",contador_timestamp_tlb);
	contador_timestamp_tlb++;
	return result;
}
