#include "tests.h"
#include "signals.h"
#include "memory.h"
#include "server.h"
#include "client.h"
#include "lib.h"
#include "tests.h"

void test_tlb() {
	print_tlb_status();
}

// Tests de integracion de forma dinamica

// Test LRU CACHE
void test_GLOBAL_CACHE_LRU_TLB_FIFO() {
	datosConfigMemoria->tipo_asignacion = "DINAMICA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "LRU";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;
	datosConfigMemoria->algoritmo_reemplazo_tlb = "FIFO";
	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	cache_imprimir();
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);
	cache_imprimir();

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 3 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 2 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 1 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 7, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 7 es pagina: %d carpincho %d\n", pagina7->pagina,pagina7->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION GLOBAL\n" _RESET);
	printf(_GREEN "			MP ALGORITMO LRU\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO FIFO\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);

}

void test_GLOBAL_CACHE_LRU_TLB_LRU() {
	datosConfigMemoria->tipo_asignacion = "DINAMICA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "LRU";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;
	datosConfigMemoria->algoritmo_reemplazo_tlb = "LRU";
	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina (2) que ya tengo en la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	leer_en_pagina(pagina2, 4, 0);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Leo pagina2.\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear y esta en cache todavia\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Ya esta en cache\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 1 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina2, esta en tlb actualizo timestamp\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, esta en tlb en entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 0 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 3, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 3 es pagina: %d carpincho %d\n", pagina3->pagina,pagina3->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION GLOBAL\n" _RESET);
	printf(_GREEN "			MP ALGORITMO LRU\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO LRU\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);

}
// Test M-CLOCK CACHE
void test_GLOBAL_CACHE_MCLOCK_TLB_FIFO() {
	datosConfigMemoria->tipo_asignacion = "DINAMICA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "M-CLOCK";
	datosConfigMemoria->algoritmo_reemplazo_tlb = "FIFO";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;

	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	cache_imprimir();
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);
	cache_imprimir();

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 3 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 2 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 1 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 7, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 7 es pagina: %d carpincho %d\n", pagina7->pagina,pagina7->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION GLOBAL\n" _RESET);
	printf(_GREEN "			MP ALGORITMO M-CLOCK\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO FIFO\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);

	print_m_clock_gant_status_cache();
}

void test_GLOBAL_CACHE_MCLOCK_TLB_LRU() {
	datosConfigMemoria->tipo_asignacion = "DINAMICA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "M-CLOCK";
	datosConfigMemoria->algoritmo_reemplazo_tlb = "LRU";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;

	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina (2) que ya tengo en la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	leer_en_pagina(pagina2, 4, 0);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Leo pagina2.\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear y esta en cache todavia\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Ya esta en cache\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 1 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina2, esta en tlb actualizo timestamp\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, esta en tlb en entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 0 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 3, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 3 es pagina: %d carpincho %d\n", pagina3->pagina,pagina3->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION GLOBAL\n" _RESET);
	printf(_GREEN "			MP ALGORITMO M-CLOCK\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO LRU\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);

	print_m_clock_gant_status_cache();
}

// Tests de integracion de forma FIJA

// Test LRU CACHE
void test_FIJA_CACHE_LRU_TLB_FIFO() {
	datosConfigMemoria->tipo_asignacion = "FIJA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "LRU";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;
	datosConfigMemoria->algoritmo_reemplazo_tlb = "FIFO";
	datosConfigMemoria->marcos_por_carpincho = 2;

	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	cache_imprimir();
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);
	cache_imprimir();

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 3 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 2 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 1 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 7, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 7 es pagina: %d carpincho %d\n", pagina7->pagina,pagina7->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION GLOBAL\n" _RESET);
	printf(_GREEN "			MP ALGORITMO LRU\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO FIFO\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);

}

void test_FIJA_CACHE_LRU_TLB_LRU() {
	datosConfigMemoria->tipo_asignacion = "DINAMICA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "LRU";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;
	datosConfigMemoria->algoritmo_reemplazo_tlb = "LRU";
	datosConfigMemoria->marcos_por_carpincho = 2;
	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo en la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina2, 4, 0);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	cache_imprimir();
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);
	cache_imprimir();
	leer_en_pagina(pagina3, 4, 0);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);
	cache_imprimir();

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 3 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 2 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 1 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 7, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 7 es pagina: %d carpincho %d\n", pagina7->pagina,pagina7->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION GLOBAL\n" _RESET);
	printf(_GREEN "			MP ALGORITMO LRU\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO FIFO\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);


}
// Test M-CLOCK CACHE
void test_mini_FIJA_CACHE_MCLOCK_TLB_FIFO() {
	datosConfigMemoria->tipo_asignacion = "FIJA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "M-CLOCK";
	datosConfigMemoria->algoritmo_reemplazo_tlb = "FIFO";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;
	datosConfigMemoria->marcos_por_carpincho = 2;

	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho2 = create_carpincho(2);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);

	escribir_en_pagina_string(pagina3,"33333333",0);
	printf(_GREEN "----------------------------------\n" _RESET);

	escribir_en_pagina_string(pagina4,"44444444",0);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	cache_imprimir();
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);
	cache_imprimir();

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION FIJA\n" _RESET);
	printf(_CYAN "			MAXIMO MARCO POR CARPINCHOS 2\n" _RESET);
	printf(_GREEN "			MP ALGORITMO M-CLOCK\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO FIFO\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);
	print_m_clock_gant_status_cache();
}

void test_FIJA_CACHE_MCLOCK_TLB_FIFO() {
	datosConfigMemoria->tipo_asignacion = "FIJA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "M-CLOCK";
	datosConfigMemoria->algoritmo_reemplazo_tlb = "FIFO";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;
	datosConfigMemoria->marcos_por_carpincho = 2;

	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	cache_imprimir();
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);
	cache_imprimir();

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 3 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 2 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 1 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 7, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 7 es pagina: %d carpincho %d\n", pagina7->pagina,pagina7->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION FIJA\n" _RESET);
	printf(_CYAN "			MAXIMO MARCO POR CARPINCHOS 2\n" _RESET);
	printf(_GREEN "			MP ALGORITMO M-CLOCK\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO FIFO\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);
	print_m_clock_gant_status_cache();
}

void test_FIJA_CACHE_MCLOCK_TLB_LRU() {
	datosConfigMemoria->tipo_asignacion = "FIJA";
    datosConfigMemoria->algoritmo_reemplazo_mmu = "M-CLOCK";
	datosConfigMemoria->algoritmo_reemplazo_tlb = "LRU";
	datosConfigMemoria->tamanio = 160;
	datosConfigMemoria->tamanio_pagina = 32;
	datosConfigMemoria->marcos_por_carpincho = 2;

	printf(_YELLOW "\n tipo de asignacion: %s\n"_RESET,datosConfigMemoria->tipo_asignacion);
	printf(_YELLOW "\n algoritmo de remplazo mmu: %s\n"_RESET,datosConfigMemoria->algoritmo_reemplazo_mmu);

	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	escribir_en_pagina_string(pagina1,"11111111",0);
	escribir_en_pagina_string(pagina2,"22222222",0);
	escribir_en_pagina_string(pagina3,"33333333",0);
	escribir_en_pagina_string(pagina4,"44444444",0);
	escribir_en_pagina_string(pagina5,"55555555",0);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "------LRU Asignación global-------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_RED "----------------------------------\n" _RESET);
	printf(_RED "--------------Caso 1--------------\n" _RESET);
	printf(_RED "----------------------------------\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 1\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina6 = create_pagina(carpincho1);
	escribir_en_pagina_string(pagina6, "66666666", 0);
	print_tlb_status();
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Referencio a una pagina que ya tengo (pagina 3) que es la primera en quedar afuera de la tlb\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	leer_en_pagina(pagina3, 4, 0);
	cache_imprimir();
	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 2\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina7 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina7, "777777777", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 4\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina8 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina8, "888888888", 0);
	cache_imprimir();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Agrego nueva pagina y escribo en ella. La proxima pagina a salir es la 5\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	t_pagina* pagina9 = create_pagina(carpincho2);
	escribir_en_pagina_string(pagina9, "999999999", 0);
	cache_imprimir();

	print_tlb_status();

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "EL RESULTADO DE LA CACHE TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "6|6|6|6|6|6|6|6|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|7|7|7|7|7|7|7|7|7|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|3|3|3|3|3|3|3|3|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|8|8|8|8|8|8|8|8|8|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|9|9|9|9|9|9|9|9|9|-|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|" _RESET);
	printf(_GREEN "\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "RESUMEN DE ACCIONES\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "Se creo carpincho1\n" _RESET);
	printf(_GREEN "Se creo carpincho2\n" _RESET);
	printf(_GREEN "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina1 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina2 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina3 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina4 con marco\n" _RESET);
	printf(_GREEN "Se creo pagina5 con marco\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Escribo en pagina1\n" _RESET);
	printf(_GREEN "Escribo en pagina2\n" _RESET);
	printf(_GREEN "Escribo en pagina3\n" _RESET);
	printf(_GREEN "Escribo en pagina4\n" _RESET);
	printf(_GREEN "Escribo en pagina5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina6 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina6 no puede swampea pagina 1\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Leo pagina3. Por lo tanto es la ultima a swampear\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina7 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina7 no puede swampea pagina 2\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina8 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina8 no puede swampea pagina 4\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Se creo pagina9 sin marco\n" _RESET);
	printf(_GREEN "Trato de escribir en pagina9 no puede swampea pagina 5\n" _RESET);
	printf(_GREEN "\n\n" _RESET);
	printf(_GREEN "Compruebe que la pagina 3 sigue en la cache. De ser asi el algoritmo funciono correctamente\n" _RESET);

	printf(_GREEN "----------------------------------\n" _RESET);

	print_tlb_status();

	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "EL RESULTADO DE LA TLB TIENE QUE SER EL SIGUIENTE\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW " Entrada: 0 pagina: 3 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 1 pagina: 4 carpincho: 2\n" _RESET);
	printf(_YELLOW " Entrada: 2 pagina: 5 carpincho: 2\n" _RESET);
	printf(_YELLOW "\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);
	printf(_YELLOW "RESUMEN DE ACCIONES\n" _RESET);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_YELLOW "Se creo carpincho1\n" _RESET);
	printf(_YELLOW "Se creo carpincho2\n" _RESET);
	printf(_YELLOW "Carpincho3 no se crea ya que es el carpincho1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina1 con marco se agrega a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina2 con marco se agrega a entrada 1\n" _RESET);
	printf(_YELLOW "Se creo pagina3 con marco se agrega a entrada 2\n" _RESET);
	printf(_YELLOW "Se creo pagina4 con marco, no hay sugar en la tlb se remplaza a entrada 0\n" _RESET);
	printf(_YELLOW "Se creo pagina5 con marco, no hay sugar en la tlb se remplaza a entrada 1\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Escribo en pagina1, no esta en tlb se remplaza entrada 2 con pagina 1\n" _RESET);
	printf(_YELLOW "Escribo en pagina2, no esta en tlb se remplaza entrada 0 con pagina 2\n" _RESET);
	printf(_YELLOW "Escribo en pagina3, no esta en tlb se remplaza entrada 1 con pagina 3\n" _RESET);
	printf(_YELLOW "Escribo en pagina4, no esta en tlb se remplaza entrada 2 con pagina 4\n" _RESET);
	printf(_YELLOW "Escribo en pagina5, no esta en tlb se remplaza entrada 0 con pagina 5\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina6 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina6, no esta en tlb se remplaza entrada 1 con pagina 6\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Leo pagina3, no esta en tlb se remplaza entrada 2 con pagina 3\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina7 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina7, no esta en tlb se remplaza entrada 0 con pagina 7\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina8 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina8, no esta en tlb se remplaza entrada 1 con pagina 8\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Se creo pagina9 sin marco\n" _RESET);
	printf(_YELLOW "Trato de escribir en pagina9, no esta en tlb se remplaza entrada 2 con pagina 9\n" _RESET);
	printf(_YELLOW "\n\n" _RESET);
	printf(_YELLOW "Compruebe que la pagina 7, 8 y 9 estan en tlb\n" _RESET);

	printf("\n LA PAGINA 7 es pagina: %d carpincho %d\n", pagina7->pagina,pagina7->carpincho_pid);
	printf("\n LA PAGINA 8 es pagina: %d carpincho %d\n", pagina8->pagina,pagina8->carpincho_pid);
	printf("\n LA PAGINA 9 es pagina: %d carpincho %d\n", pagina9->pagina,pagina9->carpincho_pid);
	printf(_YELLOW "----------------------------------\n" _RESET);

	printf(_RED "		Que se comprobo con este test\n" _RESET);
	printf(_CYAN "			TIPO ASIGNACION FIJA\n" _RESET);
	printf(_CYAN "			MAXIMO MARCO POR CARPINCHOS 2\n" _RESET);
	printf(_GREEN "			MP ALGORITMO M-CLOCK\n" _RESET);
	printf(_YELLOW "			TLB ALGORITMO FIFO\n" _RESET);
	printf(_RED "		FIN TEST\n" _RESET);
}


void test_obtener_todas_las_paginas_con_presencia_en_cache() {
	//_______DEBUG____OBTENER_TODAS_LAS_PAGINAS_CON_PRESENCIA_EN_CACHE___
	printf("\n Cantidad de marcos %d \n", cantidad_maxima_marcos);
	t_carpincho* carpincho1 = create_carpincho(1);
	t_carpincho* carpincho2 = create_carpincho(2);
	t_carpincho* carpincho3 = create_carpincho(3);
	printf("Total de carpinchos creados: %d \n", list_size(carpinchos_list));

	if (carpincho3->pid == carpincho1->pid) {
	 	printf("\n El carpincho con pid %d, ya existia por eso se lo retorno sin crear otro\n", carpincho1->pid);
	}

	t_pagina* pagina1 = create_pagina(carpincho1);
	t_pagina* pagina2 = create_pagina(carpincho1);
	t_pagina* pagina3 = create_pagina(carpincho2);
	t_pagina* pagina4 = create_pagina(carpincho2);
	t_pagina* pagina5 = create_pagina(carpincho1);

	printf("\n Cantidad de paginas carpincho 1: %d\n", list_size(carpincho1->paginas));
	printf("\n Cantidad de paginas carpincho 2: %d\n", list_size(carpincho2->paginas));
	printf("\n\n La cantidad de paginas actuales en cache son: %d\n\n" ,list_size(get_all_pages_in_cache()));
	printf("\n\n La cantidad de paginas actuales en cache son: %d\n\n" ,list_size(paginas_en_cache_list));


	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "LLENO LOS MARCOS PARA PROBAR ALGORITMOS DE REMPLAZO\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf(_GREEN "\nCACHE\n\n" _RESET);
	//Hay que hacer una función escribir desde pagina
	// que tenga mas protocolos de seguridad
	//idem con allocs y carpinchos
	escribir_en_cache_string(pagina1->marco,"11111111",0);
	escribir_en_cache_string(pagina2->marco,"22222222",0);
	escribir_en_cache_string(pagina3->marco,"33333333",0);
	escribir_en_cache_string(pagina4->marco,"44444444",0);
	escribir_en_cache_string(pagina5->marco,"55555555",0);

	printf(_GREEN "----------------------------------\n" _RESET);
	printf(_GREEN "Estado inicial\n" _RESET);
	printf(_GREEN "----------------------------------\n" _RESET);

	printf("Todas los marcos estan ocupados la proxima pagina a escribir/leer \n que no este en la memoria debe cambiar algunar\n");
	luz_verde();
	luz_roja();

	//_______DEBUG____END
}

void test_paginacion()
{
	int carpID = 105;

	int adreess1 =  0;
	if(memory_memalloc(carpID, 5, &adreess1) == EXITO)
		printf("Direccion 1: %d \n", adreess1);
	else
		printf("Error no hay espacio de memoria \n");
	cache_imprimir();
	int adreess2 =  0;
	if(memory_memalloc(carpID, 5, &adreess2) == EXITO)
		printf("Direccion 2: %d \n", adreess2);
	else
		printf("Error no hay espacio de memoria \n");
	cache_imprimir();
	int adreess3 =  0;
	if(memory_memalloc(carpID, 5, &adreess3) == EXITO)
		printf("Direccion 3: %d \n", adreess3);
	else
		printf("Error no hay espacio de memoria \n");
	cache_imprimir();
	int adreess4 =  0;
	if(memory_memalloc(carpID, 5, &adreess4) == EXITO)
		printf("Direccion 4: %d \n", adreess4);
	else
		printf("Error no hay espacio de memoria \n");
	cache_imprimir();
	int adreess5 =  0;
	if(memory_memalloc(carpID, 5, &adreess5) == EXITO)
		printf("Direccion 5: %d \n", adreess5);
	else
		printf("Error no hay espacio de memoria \n");
	cache_imprimir();
	char valor[3] = {'1','2', '3'};
	int size = sizeof(valor)/sizeof(valor[0]);
	cache_imprimir();
	if(memory_memwrite(carpID, adreess1, size, valor) == EXITO)
		printf("Se escribio el valor: %s \n", valor);
	else
		printf("Error al escribir el valor %s \n", valor);
	cache_imprimir();
	char* leido = malloc(sizeof(char)*size);
	if(memory_memread(carpID, adreess1, size, leido) == EXITO)
	{
		for (int i = 0; i < size; i++)
		{
			printf("%c", leido[i]);
		}
		printf("\n");
	}

	cache_imprimir();

	if(memory_memfree(carpID, adreess1) == EXITO)
		printf("Se libero la direccion: %d \n", adreess1);
	else
		printf("Error al liberar la direccion: %d \n", adreess1);

	if(memory_memfree(carpID, adreess2) == EXITO)
		printf("Se libero la direccion: %d \n", adreess2);
	else
		printf("Error al liberar la direccion: %d \n", adreess2);


	if(memory_memfree(carpID, adreess1) == EXITO)
		printf("Se libero la direccion: %d \n", adreess1);
	else
		printf("Error al liberar la direccion: %d \n", adreess1);

	if(memory_memfree(carpID, adreess2) == EXITO)
		printf("Se libero la direccion: %d \n", adreess2);
	else
		printf("Error al liberar la direccion: %d \n", adreess2);

	if(memory_memfree(carpID, adreess3) == EXITO)
		printf("Se libero la direccion: %d \n", adreess3);
	else
		printf("Error al liberar la direccion: %d \n", adreess3);

	if(memory_memfree(carpID, adreess4) == EXITO)
		printf("Se libero la direccion: %d \n", adreess4);
	else
		printf("Error al liberar la direccion: %d \n", adreess4);

	if(memory_memfree(carpID, adreess5) == EXITO)
		printf("Se libero la direccion: %d \n", adreess5);
	else
		printf("Error al liberar la direccion: %d \n", adreess5);

}

void test_alloc()
{
	//Creamos un carpincho
	int carpID = 105;

	//Usamos 2 pag de 32 K
	uint32_t adreess1 =  0;
	if(memory_memalloc(carpID, 14, &adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 asignada es: %d \n", adreess1);
	else
		printf("[Test ALLOC] - La Direccion 1 - Error no hay espacio de memoria \n");

	uint32_t adreess2 =  0;
	if(memory_memalloc(carpID, 2, &adreess2) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 asignada es: %d \n", adreess2);
	else
		printf("[Test ALLOC] - La Direccion 2 - Error no hay espacio de memoria \n");

	uint32_t adreess3 =  0;
	if(memory_memalloc(carpID, 2, &adreess3) == EXITO)
		printf("[Test ALLOC] - La Direccion 3 asignada es: %d \n", adreess3);
	else
		printf("[Test ALLOC] - La Direccion 3 - Error no hay espacio de memoria \n");

	uint32_t adreess4 =  0;
	if(memory_memalloc(carpID, 2, &adreess4) == EXITO)
		printf("[Test ALLOC] - La Direccion 4 asignada es: %d \n", adreess4);
	else
		printf("[Test ALLOC] - La Direccion 4 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	//Liberamos el primer alloc
	if(memory_memfree(carpID, adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 - Se ha liberado \n");
	else
		printf("[Test ALLOC] - La Direccion 1 - Error al liberar \n");

	mostrar_allocs(carpID);

	//Volvemos a allocar mismo tamanio, ahora deberia usar estrategia 1 de meter ene l mismo heap xq esta libre
	adreess1 =  0;
	if(memory_memalloc(carpID, 14, &adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 asignada es: %d \n", adreess1);
	else
		printf("[Test ALLOC] - La Direccion 1 - Error no hay espacio de memoria \n");

	//Volvemos a liberar
	if(memory_memfree(carpID, adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 - Se ha liberado \n");
	else
		printf("[Test ALLOC] - La Direccion 1 - Error al liberar \n");

	mostrar_allocs(carpID);

	//Alocamos mas chico asi se trunca.
	adreess1 =  0;
	if(memory_memalloc(carpID, 2, &adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 asignada es: %d \n", adreess1);
	else
		printf("[Terst ALLOC] - La Direccion 1 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	uint32_t adreess5 =  0;
	if(memory_memalloc(carpID, 2, &adreess5) == EXITO)
		printf("[Test ALLOC] - La Direccion 5 asignada es: %d \n", adreess5);
	else
		printf("[Terst ALLOC] - La Direccion 5 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	//Intentamos escribir en un heap chiquito algo muy grande
	char valor[2] = {'G','A'};
	if(memory_memwrite(carpID, adreess2, 100, valor) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 se escribio el valor: %c%c \n", valor[0], valor[1]);
	else
		printf("[Test ALLOC] - La Direccion 2 error al escribir el valor %s \n", valor);

	//Ahora si escribimos algo q entre.
	if(memory_memwrite(carpID, adreess2, 2, valor) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 se escribio el valor: %c%c \n", valor[0], valor[1]);
	else
		printf("[Test ALLOC] - La Direccion 2 error al escribir el valor %c%c \n", valor[0], valor[1]);

	mostrar_allocs(carpID);

	int size = 2;
    char* leido = malloc(sizeof(char) * size);;
	if(memory_memread(carpID, adreess2, size, leido) == EXITO)
	{
		printf("[Test ALLOC] - La Direccion 2 Exito al leer - Leido:");
		for (int i = 0; i < size; i++)
		{
			printf("%c", leido[i]);
		}
		printf("\n");
		printf(_RED "\n ------------> Estoy ACA5\n" _RESET);
	}
	else
		printf("[Test ALLOC] - La Direccion 2 error al leer \n");

	free(leido);

	//Ahora si escribimos algo q entre.
	valor[0] = 'Z';
	if(memory_memwrite(carpID, adreess1, 1, valor) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 se escribio el valor: %c \n", valor[0]);
	else
		printf("[Test ALLOC] - La Direccion 1 error al escribir el valor %c \n", valor[0]);

	mostrar_allocs(carpID);

	size = 1;
	leido = malloc(sizeof(char) * size);;
	if(memory_memread(carpID, adreess1, size, leido) == EXITO)
	{
		printf("[Test ALLOC] - La Direccion 1 Exito al leer - Leido:");
		for (int i = 0; i < size; i++)
		{
			printf("%c", leido[i]);
		}
		printf("\n");
	}
	else
		printf("[Test ALLOC] - La Direccion 1 error al leer \n");

	free(leido);

	mostrar_allocs(carpID);

	uint32_t adreess6 =  999;
	if(memory_memalloc(carpID, 50, &adreess6) == EXITO)
		printf("[Test ALLOC] - La Direccion 6 asignada es: %d \n", adreess6);
	else
		printf("[Test ALLOC] - La Direccion 6 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	char* valor_largo = "PRUEBA*LARGA*LARGA*LARGA";
	if(memory_memwrite(carpID, adreess6, 24, valor_largo) == EXITO)
		printf("[Test ALLOC] - La Direccion 6 se escribio el valor: %s \n", valor_largo);
	else
		printf("[Test ALLOC] - La Direccion 6 error al escribir el valor %s \n", valor_largo);

	cache_imprimir();
}

void test_alloc_compactar_simple()
{
	//Creamos un carpincho
	int carpID = 105;

	uint32_t adreess1 =  0;
	if(memory_memalloc(carpID, 22, &adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 asignada es: %d \n", adreess1);
	else
		printf("[Test ALLOC] - La Direccion 1 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	uint32_t adreess2 =  0;
	if(memory_memalloc(carpID, 20, &adreess2) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 asignada es: %d \n", adreess2);
	else
		printf("[Test ALLOC] - La Direccion 2 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	if(memory_memfree(carpID, adreess2) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 - Se ha liberado \n");
	else
		printf("[Test ALLOC] - La Direccion 2 - Error al liberar \n");

	mostrar_allocs(carpID);
}

void test_alloc_compactar_complejo()
{
	//Creamos un carpincho
	int carpID = 105;

	uint32_t adreess1 =  0;
	if(memory_memalloc(carpID, 23, &adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 asignada es: %d \n", adreess1);
	else
		printf("[Test ALLOC] - La Direccion 1 - Error no hay espacio de memoria \n");

	uint32_t adreess2 =  0;
	if(memory_memalloc(carpID, 7, &adreess2) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 asignada es: %d \n", adreess2);
	else
		printf("[Test ALLOC] - La Direccion 2 - Error no hay espacio de memoria \n");

	uint32_t adreess3 =  0;
	if(memory_memalloc(carpID, 14, &adreess3) == EXITO)
		printf("[Test ALLOC] - La Direccion 3 asignada es: %d \n", adreess3);
	else
		printf("[Test ALLOC] - La Direccion 3 - Error no hay espacio de memoria \n");

	uint32_t adreess4 =  0;
	if(memory_memalloc(carpID, 7, &adreess4) == EXITO)
		printf("[Test ALLOC] - La Direccion 4 asignada es: %d \n", adreess4);
	else
		printf("[Test ALLOC] - La Direccion 4 - Error no hay espacio de memoria \n");

	uint32_t adreess5 =  0;
	if(memory_memalloc(carpID, 23, &adreess5) == EXITO)
		printf("[Test ALLOC] - La Direccion 5 asignada es: %d \n", adreess5);
	else
		printf("[Test ALLOC] - La Direccion 5- Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	if(memory_memfree(carpID, adreess4) == EXITO)
		printf("[Test ALLOC] - La Direccion 4 - Se ha liberado \n");
	else
		printf("[Test ALLOC] - La Direccion 4 - Error al liberar \n");

	mostrar_allocs(carpID);

	if(memory_memfree(carpID, adreess2) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 - Se ha liberado \n");
	else
		printf("[Test ALLOC] - La Direccion 2 - Error al liberar \n");

	mostrar_allocs(carpID);

	if(memory_memfree(carpID, adreess3) == EXITO)
		printf("[Test ALLOC] - La Direccion 3 - Se ha liberado \n");
	else
		printf("[Test ALLOC] - La Direccion 3 - Error al liberar \n");

	mostrar_allocs(carpID);
}

void test_alloc_compactar_no_hay_que_compactar()
{
	//Creamos un carpincho
	int carpID = 105;

	uint32_t adreess1 =  0;
	if(memory_memalloc(carpID, 5, &adreess1) == EXITO)
		printf("[Test ALLOC] - La Direccion 1 asignada es: %d \n", adreess1);
	else
		printf("[Test ALLOC] - La Direccion 1 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	uint32_t adreess2 =  0;
	if(memory_memalloc(carpID, 20, &adreess2) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 asignada es: %d \n", adreess2);
	else
		printf("[Test ALLOC] - La Direccion 2 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	uint32_t adreess3 =  0;
	if(memory_memalloc(carpID, 5, &adreess3) == EXITO)
		printf("[Test ALLOC] - La Direccion 3 asignada es: %d \n", adreess3);
	else
		printf("[Test ALLOC] - La Direccion 3 - Error no hay espacio de memoria \n");

	mostrar_allocs(carpID);

	if(memory_memfree(carpID, adreess2) == EXITO)
		printf("[Test ALLOC] - La Direccion 2 - Se ha liberado \n");
	else
		printf("[Test ALLOC] - La Direccion 2 - Error al liberar \n");

	mostrar_allocs(carpID);
}
