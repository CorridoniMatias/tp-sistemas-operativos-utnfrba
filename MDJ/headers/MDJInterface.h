#ifndef MDJINTERFACE_H_
#define MDJINTERFACE_H_

#include <stdlib.h>
#include <unistd.h>
#include "Config.h"
#include "kemmens/SocketServer.h"
#include "FSManager.h"
#include "kemmens/Serialization.h"

/*
 * 		Verificar si un archivo existe:
 *
 * 			Para solicitar este recurso usar SocketCommons_SendStringAsContent(int socket, "/path_to_file", MESSAGETYPE_MDJ_CHECKFILE);
 *
 * 			Esta funcion devuelve un 1 o un 0 del tamaño uint32_t a traves del socket solicitante. Para recibir la respuesta usar SocketCommons_ReceiveDataWithoutHeader.
 *
 * 			Si devuelve 1 quiere decir que el archivo existe, si devuelve 0 no existe.
 */
void MDJ_CheckFile(void* arriveData);


/*		Crea un archivo en el FS:
 *
 *			Para solicitar este recurso usar SocketCommons_SendData(int socket, MESSAGETYPE_MDJ_CREATEFILE, <serialized_data>, <serialized_data.length>);
 *
 *				* <serialized_data> es un paquete serializado que debe cumplir el siquiente orden: |    pathToFile    |    newLineCount    |
 *					pathToFile: Ruta del archivo a crear.
 *					newLineCount: Cantidad de \n que se deben crear por defecto.
 *
 *			Esta funcion devuelve un numero del tamaño uint32_t a traves del socket solicitante. Para recibir la respuesta usar SocketCommons_ReceiveDataWithoutHeader.
 *
 *			Valores de retorno:
 *				numero distinto de 400: ver FSManager.h para la interpretacion del valor de retorno.
 *				400: la cantidad de argumentos no coincide con la interfaz.
 *
 */
void MDJ_CreateFile(void* arriveData);

/*		Leer datos desde un archivo:
 *
 *			Para solicitar este recurso usar SocketCommons_SendData(int socket, MESSAGETYPE_MDJ_GETDATA, <serialized_data>, <serialized_data.length>);
 *
 *				* <serialized_data> es un paquete serializado que debe cumplir el siquiente orden: |  pathToFile  |  offset  |  tamañoALeer  |
 *					pathToFile: Ruta del archivo a crear.
 *					offset: Cantidad bytes a desplazarse para empezar a leer.
 *					tamañoALeer: Cantidad de bytes a leer
 *
 *			Esta funcion devuelve un string o un int a traves del socket solicitante.
 *			Para recibir la respuesta usar SocketCommons_ReceiveData(int socket, int* message_type, int* error_status) -> se tiene que usar esta porque no se sabe el length porque puede ser que se soliciten mas bytes que los que existen.
 *
 *				message_type tomara valores de MESSAGETYPE_STRING o MESSAGETYPE_INT.
 *				En caso de ser MESSAGETYPE_INT el valor recibido es un error (404 = el archivo solicitado no existe. 400 = cantidad de argumentos no coincide con la interfaz)
 *				En caso de ser MESSAGETYPE_STRING el valor recibido es el contenido del archivo.
 *
 */
void MDJ_GetData(void* arriveData);

/*		Escribe datos en un archivo:
 *
 *			Para solicitar este recurso usar SocketCommons_SendData(int socket, MESSAGETYPE_MDJ_PUTDATA, <serialized_data>, <serialized_data.length>);
 *
 *				* <serialized_data> es un paquete serializado que debe cumplir el siquiente orden: |  pathToFile  |  offset  |  tamañoDelBuffer  |  buffer  |
 *					pathToFile: Ruta del archivo a crear.
 *					offset: Cantidad bytes a desplazarse para empezar a leer.
 *					tamañoDelBuffer: cantidad de bytes que tiene el buffer
 *					buffer: contenido a escribir
 *
 *			Esta funcion devuelve un numero del tamaño uint32_t a traves del socket solicitante. Para recibir la respuesta usar SocketCommons_ReceiveDataWithoutHeader.
 *
 *			Valores de retorno:
 *				numero distinto de 400: ver FSManager.h para la interpretacion del valor de retorno.
 *				400: la cantidad de argumentos no coincide con la interfaz.
 *
 */
void MDJ_PutData(void* arriveData);

/*
 * 		Elimina un archivo del FS:
 *
 * 			Para solicitar este recurso usar SocketCommons_SendStringAsContent(int socket, "/path_to_file", MESSAGETYPE_MDJ_DELETEFILE);
 *
 * 			Esta funcion devuelve un numero del tamaño uint32_t a traves del socket solicitante. Para recibir la respuesta usar SocketCommons_ReceiveDataWithoutHeader.
 *
 * 			ver FSManager.h para la interpretacion del valor de retorno.
 */
void MDJ_DeleteFile(void* arriveData);

#endif /* MDJINTERFACE_H_ */
