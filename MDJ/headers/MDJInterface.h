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
 */
void MDJ_CheckFile(void* arriveData);

void MDJ_CreateFile(void* arriveData);

void MDJ_GetData(void* arriveData);

void MDJ_PutData(void* arriveData);

void MDJ_DeleteFile(void* arriveData);

#endif /* MDJINTERFACE_H_ */
