#ifndef DAMINTERFACE_H_
#define DAMINTERFACE_H_

#include <stdlib.h>
#include <unistd.h>
#include "Config.h"
#include "kemmens/SocketServer.h"
#include "kemmens/SocketClient.h"
#include "kemmens/Serialization.h"
#include "kemmens/SocketMessageTypes.h"
#include "kemmens/Utils.h"

void DAM_Abrir(void* arriveData);

void DAM_Flush(void* arriveData);

void DAM_Crear(void* arriveData);

void DAM_Borrar(void* arriveData);

void DAM_ErrorOperacion(uint32_t idDTB);

void* DAM_ReadFile(char* filePath, int socketMDJ, int* received_content_length);

#endif /* DAMINTERFACE_H_ */
