#ifndef INCS_CONSOLEHANDLER_H_
#define INCS_CONSOLEHANDLER_H_

#include "kemmens/CommandInterpreter.h"
#include "kemmens/ThreadPool.h"
#include "SharedResources.h"
#include "kemmens/SocketServer.h"


void CommandEjecutar (int argC, char** args, char* callingLine, void* extraData);

void CommandStatus (int argC, char** args, char* callingLine, void* extraData);

void CommandFinalizar (int argC, char** args, char* callingLine, void* extraData);

void CommandMetricas (int argC, char** args, char* callingLine, void* extraData);

void CommandQuit (int argC, char** args, char* callingLine, void* extraData);

void ProcessLineInput(char* line);

#endif /* INCS_CONSOLEHANDLER_H_ */
