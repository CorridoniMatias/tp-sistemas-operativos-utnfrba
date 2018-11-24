#include "headers/FM9_Server.h"

void OnPostInterpreter(char* cmd, char* sep, void* args, bool actionFired) {
	free(cmd);
}

void ProcessLineInput(char* line) {
	ThreadableDoStructure* st = CommandInterpreter_MallocThreadableStructure();

	st->commandline = line;
	st->data = NULL;
	st->separator = " ";
	st->postDo = (void*) OnPostInterpreter;

	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	run->data = (void*) st;
	run->runnable = (void*) CommandInterpreter_DoThreaded;
	run->free_data = (void*) CommandInterpreter_FreeThreadableDoStructure;

	ThreadPool_AddJob(threadPool, run);
}

void* postDo(char* cmd, char* sep, void* args, bool fired) {
	/*if(!fired)
	 SocketCommons_SendMessageString((int)args, "Lo recibido no es comando!");*/

	free(cmd);
	return 0;
}

void onPacketArrived(int socketID, int message_type, void* data) {

	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	OnArrivedData* arriveData;
	arriveData = SocketServer_CreateOnArrivedData();

	arriveData->calling_SocketID = socketID;
	arriveData->receivedData = data;

	run->data = (void*) arriveData;

	switch (message_type)
	{
		case MESSAGETYPE_CPU_ASKLINE:
			run->runnable = FM9_AskForLine;
			break;
		case MESSAGETYPE_CPU_ASIGNAR:
			run->runnable = FM9_AsignLine;
			break;
		case MESSAGETYPE_CPU_CLOSE:
			run->runnable = FM9_Close;
			break;
		case MESSAGETYPE_FM9_OPEN:
			run->runnable = FM9_Open;
			break;
		case MESSAGETYPE_FM9_FLUSH:
			run->runnable = FM9_Flush;
			break;
		default:
			free(run);
			free(arriveData);
			free(data);
			run = NULL;
			break;

	}
	//TODO verificar que onda con el free data
	if(run != NULL)
		ThreadPool_AddJob(threadPool, run);
}

void ClientConnected(int socket) {
	printf("Cliente se conecto! %d\n", socket);
}

void ClientDisconnected(int socket) {
	//RemoveCPU(socket); //Si no esta, no se va a sacar nada.
	printf("Cliente se fue! %d\n", socket);
}

void ClientError(int socketID, int errorCode) {
	printf("Cliente %d se reporto con error %s!\n", socketID,
			strerror(errorCode));
}

void StartServer() {
	CommandInterpreter_Init();
	threadPool = ThreadPool_CreatePool(10, false);

	SocketServer_Start("FM9", settings->puerto);
	Logger_Log(LOG_INFO, "Escuchando en el puerto %d", settings->puerto);
	SocketServer_ActionsListeners actions = INIT_ACTION_LISTENER;

	CommandInterpreter_RegisterCommand("dump", FM9_Dump);

	actions.OnConsoleInputReceived = (void*) ProcessLineInput;
	actions.OnPacketArrived = (void*) onPacketArrived;
	actions.OnClientConnected = (void*) ClientConnected;
	actions.OnClientDisconnect = (void*) ClientDisconnected;
	actions.OnReceiveError = (void*) ClientError;

	SocketServer_ListenForConnection(actions);

	Logger_Log(LOG_INFO, "Server Shutdown.");
}
