#ifndef FM9_INTERFACE_H_
#define  FM9_INTERFACE_H_

#include "Storage.h"
#include "Segmentation.h"
#include "InvertedPageTable.h"
#include "FM9lib.h"

int FM9_AsignLine(int virtualAddress, int dtbID, void* data);

int FM9_AskForLine(int virtualAddress, int dtbID, void* buffer);

int FM9_Open(int dtbID, void* data, int size);

int FM9_Close(int dtbID, int virtualAddress);

void FM9_Dump(int argC, char** args, char* callingLine, void* extraData);

int FM9_Flush(int dtbID, int virtualAddress);

int sizeOfLine(char* line);
#endif /* FM9_INTERFACE_H_ */
