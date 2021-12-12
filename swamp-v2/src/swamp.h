#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <string.h>
#include <fcntl.h> //la necesito para los parámetros del open
#include <sys/mman.h> //la necesito para los parámetros del mmap

#include "utils.h"
#include "lib.h"

#define SERVER_MENSAJE 1
#define MENSAJE 2

t_list* carpinchos;
t_list* marcos;

typedef enum
{
	LEER = 11,
	ESCRIBIR = 12,
	PUEDE_ESCRIBIR = 13,
	CASO_ASIGNACION = 14
}estado;

typedef enum
{
	FIJA = 0,
	GLOBAL = 1
}caso_asignacion;

typedef struct {
	unsigned long long timestamp;
	uint32_t id_mensaje;
	uint8_t op_code;
	uint32_t mensaje_length;
	uint32_t particion_length;
	char* mensaje;
	t_list* suscriptores;
} t_msg;

typedef struct {
	int PID;
	char* archivo;
	int cantMarcos;
} carpincho;

typedef struct {
	char* archivo;
	int marco;
	int indice;
	int PID;
} marco;


/*
     Ejemplo de mensaje a serializar y deserializar
        Vamos a enviar:
            - un nombre
            - una posicion X
            - una posicion Y
            - un id
		<!-- length se utiliza para saber el tamaño de caracteres del nombre -->
*/
typedef struct {
	uint32_t length;
	char* name;
	uint32_t posicionX;
	uint32_t posicionY;
	uint32_t idMensaje;
} __attribute__((packed)) t_msg_1; // APPEARED CATCH

typedef struct{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct
{
	uint8_t codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_paquete* paquete;
int casoAsignacion;

// Funciones
int chequearEntradaNuevosCarpinchos(int PID);
void iniciar_servidor();
void server_aceptar_clientes(int);
void* server_atender_cliente(void*);
void server_procesar_mensaje(int, int);
int server_bind_listen(char*, char*);
int obtenerCantArchivos();
t_list *crearLista();
void obtenerNombre(char*, char);
int noSuperaMaximo(int totalMemoria);
char* archivoConMasEspacio();
void crear_archivos();
int verSiHayPaginas(int cantPaginas, char* archivo);

void escribir_swap(int socket);
char* recibirPagina(int PID, estado operacion);
void leer_swap(int socket);
int hayEspacioArchivo(char* archivo);
void iniciar_marcos();
void actualizarMarcos(char* archivo, int indiceMarco, int indicePagina, int PID);
void actualizarCarpincho(estado operacion, int PID);
int conseguirMarco(int PID, int indicePagina);
int chequearAsignacion(int PID);
void iniciar_carpinchos();
void puede_escribir(int socket);

int recibir_cod_op(int socket);
void deserializar_mensaje(t_paquete* paquete, int socket);

t_msg_1* deserializar_msg_parametros_cuatro (char*);
