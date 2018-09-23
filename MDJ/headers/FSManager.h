#ifndef FSMANAGER_H_
#define FSMANAGER_H_

#include <commons/bitarray.h>
#include <commons/config.h>
#include "kemmens/StringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

#include "Config.h"

void FIFA_Init();

void FIFA_ReadBitmap();

void FIFA_FreeBitmap();

void FIFA_SetUsedBlock(int blockNum);

void FIFA_FlushBitmap();

bool FIFA_IsBlockUsed(int blockNum);

void FSManager_WriteFile(char* path, int offset, int size, void* data);

char* FIFA_ReadFile(char* path, int offset, int size);

#endif /* FSMANAGER_H_ */
