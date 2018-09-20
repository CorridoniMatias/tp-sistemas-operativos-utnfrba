#ifndef SOCKETCOMMONS_H_
#define SOCKETCOMMONS_H_

#include "commons/string.h"
#include <string.h>
#include <ctype.h>
#include "logger.h"
#include "SocketMessageTypes.h"
#include <stdlib.h> // Para malloc
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo

//------ESTRUCTURAS EMPLEADAS------//

/**
 * 		Estructura que representa el encabezado de un mensaje, para ser interpretada segun el protocolo
 * 		CAMPOS:
 * 			body_length: Largo del mensaje, en bytes
 * 			message_type: Tipo de mensaje a enviar/recibir; ver en SocketMessageTypes.h
 */
typedef struct {
	uint32_t body_length;
	uint32_t message_type;
} __attribute__((packed)) ContentHeader;

/**
 *		Estructura que sirve como paquete para agrupar dos campos de datos, sus tamanios, y el tamanio total.
 *		La idea de los paquetes es guardar los datos a enviar ahi, serializarlos y enviar la cadena serializada
 *		CAMPOS:
 *			f1_length: Longitud del primer campo, en bytes (sin contar el \0 final)
 *			field1: Cadena de datos del primer campo
 *			f2_length: Longitud del segundo campo, en bytes (sin contar el \0 final)
 *			field2: Cadena de datos del segundo campo
 *			totalSize: Longitud total del paquete, en bytes (suma los largos de los campos, y el tamanio de sus lengths)
 */
typedef struct {
	uint32_t f1_length;
	char* field1;
	uint32_t f2_length;
	char* field2;
	uint32_t totalSize;
} TwoFieldedPackage;

/**
 *		Estructura que sirve como paquete para agrupar tres campos de datos, sus tamanios, y el tamanio total.
 *		La idea de los paquetes es guardar los datos a enviar ahi, serializarlos y enviar la cadena serializada
 *		CAMPOS:
 *			f1_length: Longitud del primer campo, en bytes (sin contar el \0 final)
 *			field1: Cadena de datos del primer campo
 *			f2_length: Longitud del segundo campo, en bytes (sin contar el \0 final)
 *			field2: Cadena de datos del segundo campo
 *			f3_length: Longitud del tercer campo, en bytes (sin contar el \0 final)
 *			field3: Cadena de datos del tercer campo
 *			totalSize: Longitud total del paquete, en bytes (suma los largos de los campos, y el tamanio de sus lengths)
 */
typedef struct {
	uint32_t f1_length;
	char* field1;
	uint32_t f2_length;
	char* field2;
	uint32_t f3_length;
	char* field3;
	uint32_t totalSize;
} ThreeFieldedPackage;

/**
 *		Estructura que sirve como paquete para agrupar cuatro campos de datos, sus tamanios, y el tamanio total.
 *		La idea de los paquetes es guardar los datos a enviar ahi, serializarlos y enviar la cadena serializada
 *		CAMPOS:
 *			f1_length: Longitud del primer campo, en bytes (sin contar el \0 final)
 *			field1: Cadena de datos del primer campo
 *			f2_length: Longitud del segundo campo, en bytes (sin contar el \0 final)
 *			field2: Cadena de datos del segundo campo
 *			f3_length: Longitud del tercer campo, en bytes (sin contar el \0 final)
 *			field3: Cadena de datos del tercer campo
 *			f4_length: Longitud del cuarto campo, en bytes (sin contar el \0 final)
 *			field4: Cadena de datos del cuarto campo
 *			totalSize: Longitud total del paquete, en bytes (suma los largos de los campos, y el tamanio de sus lengths)
 */
typedef struct {
	uint32_t f1_length;
	char* field1;
	uint32_t f2_length;
	char* field2;
	uint32_t f3_length;
	char* field3;
	uint32_t f4_length;
	char* field4;
	uint32_t totalSize;
} FourFieldedPackage;

//------FUNCIONES DEFINIDAS------//

/* Envia un string al socket indicado. Esta funcion ya envia el header automaticamente. Si el send del header falla devuelve -2.
 *
 *	retorna el estado del send, -2
 */
int SocketCommons_SendMessageString(int socket, char* message);

/*
 * 		Permite enviar un string serializado indicando el tipo de dato serializado en serialized_content_type
 */
int SocketCommons_SendSerializedContent(int socket, char* serialized, int serialized_content_type);

/* Recibe el header por el socket indicado.
 *
 *	retorna el socket propiamente dicho, en caso de error devuelve 0 (NULL). El free de la struct se hace solo en caso de NULL, sino hay que hacer el free.
 */
ContentHeader* SocketCommons_ReceiveHeader(int socket, int* error_status);

/*	Envia el header de datos, length es el tamaño de datos que va a recibir la otra parte, message_type es el tipo de mensaje a enviar (ver tipos en SocketMessageTypes.h)
 *
 * retorna el estado del send
 */
int SocketCommons_SendHeader(int socket, int length, int message_type);

/* Crea la estructura del header, se tiene que hacer free despues de usar.
 *
 *	retorna puntero a header.
 */
ContentHeader* SocketCommons_CreateHeader();


/*		Recibe datos por el socket indicado. El tipo recibido se almacena en message_type. Los tipos soportados se pueden encontrar en SocketMessageTypes.h
 *
 *		retorna los datos recibidos. Hacer free del buffer despues de usar. Ojo de no hacer free si devuelve NULL.
 */
void* SocketCommons_ReceiveData(int socket, int* message_type, int* error_status);


/*		Envia datos por el socket indicado. El tipo puesto en message_type debe ser un tipo conocido y definido en SocketMessageTypes.h, data contiene los datos a ser enviado y dataLength es el largo de lo contenido en la variable data.
 *
 * 		retorna el estado de envio.
 */
int SocketCommons_SendData(int socket, int message_type, void* data, int dataLength);

/*
 * 	ACCION: Deserializar los campos de la cadena serializada que recibi con el getData y guardarlos en un array
 * 			NOTA: Esta funcion debe llamarse luego de recibir el ContentHeader
 * 	PARAMETROS:
 * 		serializedContent: Cadena serializada que quiero deserializar
 * 		dataArray: Array donde ir guardando los campos que voy deserializando (sin su longitud)
 */
void SocketCommons_DeserializeContent(char* serializedContent, void** dataArray);

/*
 * 	ACCION: Cierra un socket; loguea el exito o fracaso en la clausura
 * 	PARAMETROS:
 * 		descriptorSocket: Descriptor numerico o fd del socket a cerrar
 */
void SocketCommons_CloseSocket(int descriptorSocket);

#endif /* SOCKETCOMMONS_H_ */
