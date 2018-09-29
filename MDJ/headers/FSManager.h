#ifndef FSMANAGER_H_
#define FSMANAGER_H_

#include <commons/bitarray.h>
#include <commons/config.h>
#include "kemmens/StringUtils.h"
#include "kemmens/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <pthread.h>

#include "Config.h"

pthread_mutex_t bitmapLock;

void FIFA_Init();

void FIFA_ReadBitmap();

void FIFA_FreeBitmap();

void FIFA_SetUsedBlock(int blockNum);

void FIFA_FlushBitmap();

bool FIFA_IsBlockUsed(int blockNum);

void FIFA_WriteFile(char* path, int offset, int size, void* data);

char* FIFA_ReadFile(char* path, int offset, int size, int* amountCopied);

bool FIFA_IsFileValid(char* path);

bool FIFA_CreateFile(char* path, int newLines);
void FIFA_PrintBitmap();

#endif /* FSMANAGER_H_ */
