#ifndef FM9_INTERFACE_H_
#define  FM9_INTERFACE_H_

#include <stdbool.h>
#include "kemmens/SocketCommons.h"
#include "kemmens/SocketServer.h"
#include "kemmens/Serialization.h"
#include "kemmens/Utils.h"
#include "Storage.h"
#include "Segmentation.h"
#include "InvertedPageTable.h"
#include "FM9lib.h"

void FM9_AsignLine(void* data);

void FM9_AskForLine(void* data);

void FM9_Open(void* data);

void FM9_Close(void* data);

void FM9_Flush(void* data);

void FM9_Dump(int argC, char** args, char* callingLine, void* extraData);

void FM9_CloseDTB(void* data);

int sizeOfLine(char* line, int maxSize);

#endif /* FM9_INTERFACE_H_ */

