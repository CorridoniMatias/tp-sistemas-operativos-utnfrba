#ifndef FM9_INTERFACE_H_
#define  FM9_INTERFACE_H_

#include "Storage.h"
#include "Segmentation.h"
#include "InvertedPageTable.h"
#include "FM9lib.h"

int FM9_AsignLine(int virtualAddress, int dtbID, void* data);

#endif /* FM9_INTERFACE_H_ */
