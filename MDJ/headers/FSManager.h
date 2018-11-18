#ifndef FSMANAGER_H_
#define FSMANAGER_H_

/*
 * 		FIFA - FileSystem For Academics
 */

#include <commons/bitarray.h>
#include <commons/config.h>
#include "kemmens/StringUtils.h"
#include "kemmens/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <pthread.h>
#include <dirent.h>

#include "Config.h"

//FIFA General Errors:

#define OPERATION_SUCCESSFUL	 0
#define INSUFFICIENT_SPACE		10
#define METADATA_OPEN_ERROR 	11
#define FILE_NOT_EXISTS 		12

//FIFA FileCreate Return Values:
#define EXISTING_FILE 			1
#define METADATA_CREATE_ERROR	2

pthread_mutex_t bitmapLock;

//Funcionalidades a exponer para el MDJ:

//Requerimiento: Validar Archivo
/*	Parámetros: [Path]
 *
	Descripción: Cuando el El Diego reciba la operación de abrir un archivo deberá validar que el archivo exista.
*/
bool FIFA_IsFileValid(char* path);

//Requerimiento: Crear archivo
/*
 * 	Parámetros: [Path, Cantidad de Líneas]
 *
	Descripción: Cuando el El Diego reciba la operación de crear un archivo deberá llamar a esta operación que creará
	el archivo dentro del path solicitado. El archivo creado deberá tener la cantidad de bloques necesarios para guardar
	las líneas indicadas por la operación crear con su contenido vacío.
 */
int FIFA_CreateFile(char* path, int newLines);

//Requerimiento: Obtener Datos
/*
 * 		Parámetros: [Path, Offset, Size]
 *
 * 		Descripción: Ante un pedido de datos File System devolverá del path enviado por parámetro,
 * 		la cantidad de bytes definidos por el Size a partir del offset solicitado.
 */
char* FIFA_ReadFile(char* path, int offset, int size, int* amountCopied);

//Requerimiento: Guardar Datos
/*
 * 	Parámetros: [Path, Offset, Size, Buffer]
 *
	Descripción: Ante un pedido de escritura MDJ almacenará en el path enviado por parámetro, los bytes del buffer,
	definidos por el valor del Size y a partir del offset solicitado.  En caso de que se soliciten datos o se intenten
	guardar datos en un archivo inexistente el File System deberá retornar un error de Archivo no encontrado.
 */
int FIFA_WriteFile(char* path, int offset, int size, void* data);

//Requerimiento: Borrar Archivo
/*
 * 	Parámetros: [Path]
 *
	Descripción: Ante un pedido de eliminación de un archivo, el MDJ deberá liberar los bloques asociados en el Bitmap y
	eliminar el archivo que contenga la metadata. En caso que se trate de eliminar un archivo inexistente, el File System
	deberá retornar un error de Archivo no encontrado.
 */
int FIFA_DeleteFile(char* path);


//////////////////////////////////////////////////// FUNCIONES OTRAS ////////////////////////////////////////////////////

void FIFA_Start();
void FIFA_ShutDown();
t_config* FIFA_OpenFile(char* path);

void FIFA_PrintBitmap();
void FIFA_MkDir(char* path);

#endif /* FSMANAGER_H_ */
