#ifndef DAMINTERFACE_H_
#define DAMINTERFACE_H_

#include <stdlib.h>
#include <unistd.h>
#include "Config.h"
#include "kemmens/SocketServer.h"
#include "kemmens/SocketClient.h"
#include "kemmens/Serialization.h"
#include "kemmens/SocketMessageTypes.h"

void DAM_Abrir(void* arriveData);

void DAM_Flush(void* arriveData);

void DAM_Crear(void* arriveData);

#endif /* DAMINTERFACE_H_ */
