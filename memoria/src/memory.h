#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <pthread.h>
#include "lib.h"
#include "client.h"

int tlb_hits_totales;
int tlb_miss_totales;

char* CACHE;
int cantidad_maxima_marcos;
t_list* marcos_list;
t_list* alloc_list;
t_list* paginas_en_cache_list;
t_list* carpinchos_list;

int contador_carpinchos;
int contador_timestamp;
int contador_timestamp_tlb;

pthread_mutex_t mutex_lectura_escritura;
pthread_mutex_t mutex_memalloc;

// simula al puntero de los ejercicios dentro del stack
int algoritm_stack_pointer;
int tlb_fifo_pointer;

// TLB
t_list* tlb_list;
t_list* mensajes_dump_list;
t_list* paginas_en_tlb_list;

typedef enum {
	EXITO=1,
	SIN_ESPACIO=2,
	DIRECCION_INVALIDA=3,
} status_memory_op;

typedef enum {
	ALLOC_HEAP=1,
	ALLOC_PAGINA=2,
	ALLOC_SOLICITAR_PAGINA=3,
	ALLOC_SIN_ESPACIO=4,
} check_alloc_respuesta;

typedef struct
{
	int carpincho_pid;
	int entrada;
	int estado;
	int pagina;
	int marco;
} t_mensaje_dump;

typedef struct
{
	bool resuelta;
	uint32_t direccion;
} t_resolver_direccion_logica;

typedef struct
{
	uint32_t prevAlloc;
	uint32_t nextAlloc;
	uint8_t isFree;
} __attribute__((packed)) t_heap_metadata;


typedef struct
{
	int pid;
	int tlb_hits;
	int tlb_miss;
	int algoritm_stack_pointer;
	u_int32_t last_address;
	t_list* paginas;
	t_list* allocs;
	int numero;
} t_carpincho;

typedef struct
{
	int marco;
	int carpincho_pid;
	int pagina;
	int bit_presencia;
	int bit_de_uso;
	int bit_de_modificacion;
	int carp_num;
	unsigned long long timestamp;
} t_pagina;

typedef struct
{
	int marco;
	int isFree;
	int pagina;
	int pid;
} t_marco;

typedef struct {
	int entrada;
	int pid;
	int pagina;
	int marco;
	unsigned long long timestamp;
} t_tlb;

typedef struct
{
	uint32_t direccion;
	int isFree;
	int tamanio_data;
} t_alloc;

typedef struct
{
	uint32_t pagina;
	t_list* direcciones;
} t_pagina_accion;

//memoria
void inicializarMemoria();
void close_memory();

//carpinchos
t_carpincho* get_carpincho(int pid);
t_carpincho* create_carpincho(int pid);
t_carpincho* create_carpincho_without_add_list(int pid);
void close_carpincho(t_carpincho* carpincho);
void close_carpinchos();
void limpiar_conteos_tlb_de_carpincho(t_carpincho* carpincho);

//Cache
int escribir_en_cache(int marco, char* buffer, int size, uint32_t offset);
int escribir_en_cache_string(int marco, char* buffer, uint32_t offset);
char* busquedaParticionLibre(uint32_t);
char* retornar_contenido(int marco, uint32_t offset, int tamanio);
char * obtenerPosicionLibreEnCache(uint32_t);
void cache_imprimir();
void eliminarParticionLRU();

//Marcos
void create_marcos();
void close_marcos();
void imprimir_marcos();
void imprimir_lista_de_marcos(t_list* marcos);
void vaciar_marco_en_cache(int marco);
uint32_t get_marco_ptr(int);
bool any_marco_free();
bool is_marco_in_tlb(int marco);
t_marco* get_marco(int carpincho, int pagina);
t_marco* get_marco_by_marco_number(int marco_number);
t_marco* get_free_marco();
int get_cant_marcos_libres();
int obtener_marco_para_pagina(t_pagina* pagina);
int obtener_marco_de_cache(int carpincho, int pagina);
int get_marco_de_tlb(t_pagina* pagina);
int amount_frame_used_by_carpincho(t_carpincho* carpincho);
int amount_frame_used_by_carpincho_pid(int carpincho);

//LRU
t_pagina* get_proxima_a_swampear_by_lru(t_list* paginas);

//M-CLOCK
t_pagina* get_proxima_a_swampear_by_m_clock(t_list* paginas);
void print_m_clock_gant_status_cache();

//SEMAFOROS
void luz_roja();
void luz_verde();

//paginas
t_list* get_all_pages_in_cache();
t_pagina* create_pagina();
bool pagina_is_in_tlb(t_pagina* pagina);
bool pagina_is_in_cache(t_pagina* pagina);
char* leer_en_pagina(t_pagina* pagina, int tamanio, uint32_t offset);
int escribir_en_pagina(t_pagina* pagina, char* buffer, int size, uint32_t offset);
int escribir_en_pagina_string(t_pagina* pagina, char* buffer, uint32_t offset);
int vaciar_pagina(t_pagina* pagina);
int swampear_pagina(int carpincho, int pagina);
int swampear_pagina_when_create_page(int carpincho_pid, int pagina);
void free_pages_by_carpincho();
t_pagina* get_page_by_ints(int carpincho, int pagina);
t_pagina* get_page(t_carpincho* carpincho, int pagina);

//TLB functions
t_list* create_tlb();
void free_tlb();
void actualizar_tlb(t_pagina* pagina, int marco);
void tlb_hit(t_pagina* pagina);
void tlb_miss(t_pagina* pagina);
void print_tlb_status();
void vaciar_tlb();
t_tlb* get_tlb_item(t_pagina* pagina);
t_tlb* get_next_free_tlb_item();
t_tlb* proxima_pagina_a_sacar_tlb_by_lru();
t_tlb* proxima_pagina_a_sacar_tlb_by_fifo();
t_tlb* get_tlb_item_by_marco_number(int marco);
bool any_tlb_item_is_free();
void cargar_pagina_pagina_tlb(t_pagina* pagina, int marco);
void vaciar_pagina_tlb(t_tlb* tlb_item);

//TLB dump funtions
t_mensaje_dump* create_mensaje_dump(int,int,int,int);
void free_mensaje_dump(t_mensaje_dump* mensaje_dump);
void clear_mensajes_dump();
void close_mensajes_dump();

//Utils functions
unsigned long long obtenerTimeStamp();
unsigned long long obtenerTimeStampTLB();

//mensajes
void* memory_init(uint32_t memsize, uint32_t pagesize);
int memory_memalloc(int carpinchoId, uint32_t alloc_size, uint32_t* direccion);
int memory_memwrite(int carpinchoId, uint32_t pos_log_abs , uint32_t size, char* valor);
int memory_memread(int carpinchoId, uint32_t pos , uint32_t size, char* buffer);
int memory_memfree(int carpinchoId, uint32_t pos);
int memory_memfree_all(int carpinchoId);

//allocs
check_alloc_respuesta check_alloc(t_carpincho* carpincho, int size);
void free_allocs_by_carpincho(t_carpincho* carpincho);
void mostrar_allocs(int pid);

uint32_t memory_getCantPag(uint32_t memsize, uint32_t pagesize);
uint32_t memory_getAdressFrame(uint32_t frame, uint32_t pagesize);
void memory_add_carpincho (t_list* t_carpincho, int carpId);
uint32_t memory_crearHeap(void* memoria, uint32_t alloc_size);
uint32_t memory_getNextAllocForSize(uint32_t currentPos, uint32_t alloc_size);


#endif /* MEMORY_H_ */
