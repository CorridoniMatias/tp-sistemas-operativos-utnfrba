#ifndef CPUSMANAGER_H_
#define CPUSMANAGER_H_

#include "stdlib.h"
#include "commons/collections/list.h"

t_list* cpus;

struct CPU_s
{
	int socket;
	bool busy;
} typedef CPU;

void InitCPUsHolder();

void AddCPU(int* socketID);

void FreeCPU(int socketID);

void RemoveCPU(int socketID);

int CPUsCount();

void DestroyCPUsHolder();

bool IsIdle(void* myCPU);

bool ExistsIdleCPU();


#endif /* CPUSMANAGER_H_ */
