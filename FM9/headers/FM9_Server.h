#ifndef FM9_SERVER_H_
#define FM9_SERVER_H_

#include <stdbool.h>
#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketServer.h"
#include "FM9lib.h"
#include "SharedResources.h"
#include "FM9Interface.h"

void OnPostInterpreter(char* cmd, char* sep, void* args, bool actionFired);

void ProcessLineInput(char* line);

void* postDo(char* cmd, char* sep, void* args, bool fired);

void onPacketArrived(int socketID, int message_type, void* data);

void ClientConnected(int socket);

void ClientDisconnected(int socket);

void ClientError(int socketID, int errorCode);


void StartServer();


#endif /* FM9_SERVER_H_ */
