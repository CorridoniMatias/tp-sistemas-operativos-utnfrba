#ifndef CPUSMANAGER_H_
#define CPUSMANAGER_H_

#include "stdlib.h"
#include "commons/collections/list.h"

struct CPU_s
{
	int socket;
	bool busy;
} typedef CPU;

void InitCPUsHolder();

void AddCPU(int* socketID);

void RemoveCPU(int socketID);

int CPUsCount();

void DestroyCPUsHolder();


#endif /* CPUSMANAGER_H_ */
