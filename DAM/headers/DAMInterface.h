#ifndef DAMINTERFACE_H_
#define DAMINTERFACE_H_

#include <stdlib.h>
#include <unistd.h>
#include "Config.h"
#include "kemmens/SocketServer.h"
#include "kemmens/Serialization.h"

void DAM_Abrir(void* arriveData);

void DAM_Flush(void* arriveData);

#endif /* DAMINTERFACE_H_ */
