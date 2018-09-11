#include "MDJlib.h"

void configurar()
{

	config = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"IP_DIEGO",
			"PUERTO_DIEGO",
			"DELAY"
	};


	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	strcpy(config->ipDiego, archivoConfigSacarStringDe(archivoConfig, "IP_DIEGO"));
	strcpy(config->puertoDiego, archivoConfigSacarStringDe(archivoConfig, "PUERTO_DIEGO"));
	strcpy(config->delay, archivoConfigSacarIntDe(archivoConfig, "DELAY"));

	archivoConfigDestruir(archivoConfig);

}

void initGlobals()
{
	configurar();
	CommandInterpreter_Init();
}

void freeGlobals()
{
	free(config);
}

void startServer()
{

	SocketServer_Start("SAFA", config->puertoEscucha);
	SocketServer_ActionsListeners actions = INIT_ACTION_LISTENER;
	CommandInterpreter_RegisterCommand("iam", (void*)CommandIAm());

	actions.OnConsoleInputReceived = (void*)NULL;
	actions.OnPacketArrived = (void*)onPacketArrived;
	actions.OnClientConnected = (void*)ClientConnected;
	actions.OnClientDisconnect = (void*)ClientDisconnected;
	actions.OnReceiveError = (void*)ClientError;
	SocketServer_ListenForConnection(actions);
	Logger_Log(LOG_INFO, "Se cierra el server");

}

void *CommandIAm (int argC, char** args, char* callingLine, void* extraData)
{
	CommandInterpreter_FreeArguments(args);
	return 0;
}

void* postDo(char* cmd, char* sep, void* args, bool fired)
{
	/*if(!fired)
		SocketCommons_SendMessageString((int)args, "Lo recibido no es comando!");*/

	free(cmd);
	return 0;
}


void onPacketArrived(int socketID, int message_type, void* data)
{
}

void ClientConnected(int socket)
{
	printf("Cliente se conecto! %d\n", socket);
}

void ClientDisconnected(int socket)
{
	RemoveCPU(socket);
	printf("Cliente se desconecto! %d\n", socket);
}

void ClientError(int socketID, int errorCode)
{
	printf("Cliente %d se reporto con error %s!\n", socketID, strerror(errorCode));
}
