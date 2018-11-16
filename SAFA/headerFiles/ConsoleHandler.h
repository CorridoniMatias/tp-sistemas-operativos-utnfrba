#ifndef HEADERFILES_CONSOLEHANDLER_H_
#define HEADERFILES_CONSOLEHANDLER_H_

#include "kemmens/CommandInterpreter.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/SocketServer.h"

#include "../headerFiles/Scheduling.h"
#include "../headerFiles/SharedResources.h"

//Variables externas, declaradas primero en Scheduling.h
extern int PLPtask;
extern int PCPtask;
extern sem_t workPLP;
extern CreatableGDT* toBeCreated;
extern DeassignmentInfo* toBeMoved;
extern t_queue* scriptsQueue;

void CommandEjecutar (int argC, char** args, char* callingLine, void* extraData);

void CommandStatus (int argC, char** args, char* callingLine, void* extraData);

void CommandFinalizar (int argC, char** args, char* callingLine, void* extraData);

void CommandMetricas (int argC, char** args, char* callingLine, void* extraData);

void CommandQuit (int argC, char** args, char* callingLine, void* extraData);

void ProcessLineInput(char* line);

#endif /* HEADERFILES_CONSOLEHANDLER_H_ */
