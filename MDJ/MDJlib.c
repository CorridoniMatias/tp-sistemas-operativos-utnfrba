#include "headers/MDJlib.h"
#include "kemmens/StringUtils.h"
#include "kemmens/SocketMessageTypes.h"
#include "kemmens/SocketServer.h"
#include "headers/MDJInterface.h"

void configurar()
{
	config = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"PUERTO",
			"PUNTO_MONTAJE",
			"RETARDO"
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	config->puertoEscucha = archivoConfigSacarIntDe(archivoConfig, "PUERTO");

	config->delay = archivoConfigSacarIntDe(archivoConfig, "RETARDO");

	char* montaje = archivoConfigSacarStringDe(archivoConfig, "PUNTO_MONTAJE");

	config->puntoMontaje = malloc(string_length(montaje) + 1);

	strcpy(config->puntoMontaje, montaje);

	archivoConfigDestruir(archivoConfig);

	char* campos2[] = {
			"CANTIDAD_BLOQUES",
			"TAMANIO_BLOQUES",
			"MAGIC_NUMBER"
	};

	char* bitmap = StringUtils_Format("%s%s", config->puntoMontaje, RUTA_BITMAP);
	config->bitmapFile = malloc(strlen(bitmap) + 1);
	strcpy(config->bitmapFile, bitmap);

	char* files = StringUtils_Format("%s%s", config->puntoMontaje, RUTA_ARCHIVOS);
	config->filesPath = malloc(strlen(files) + 1);
	strcpy(config->filesPath, files);


	char* blocks = StringUtils_Format("%s%s", config->puntoMontaje, RUTA_BLOQUES);
	config->blocksPath = malloc(strlen(blocks) + 1);
	strcpy(config->blocksPath, blocks);

	//metadata:

	char* metadata = StringUtils_Format("%s%s", config->puntoMontaje, RUTA_METADATA);
	config->metadataFile = malloc(strlen(metadata) + 1);
	strcpy(config->metadataFile, metadata);

	Logger_Log(LOG_DEBUG, "Levantando METADATA desde %s", metadata);

	archivoConfig = archivoConfigCrear(metadata, campos2);

	config->cantidadBloques = archivoConfigSacarIntDe(archivoConfig, "CANTIDAD_BLOQUES");
	config->tamanioBloque = archivoConfigSacarIntDe(archivoConfig, "TAMANIO_BLOQUES");
	archivoConfigDestruir(archivoConfig);

	Logger_Log(LOG_DEBUG, "Configuracion Leida: \n\t Puerto de escucha: %d \n\t Delay: %d \n\t Mnt: '%s' \n\t Cant. bloques: %d \n\t Tam. Bloques: %d\n",
							config->puertoEscucha, config->delay, config->puntoMontaje, config->cantidadBloques, config->tamanioBloque);


	free(metadata);
	free(bitmap);
	free(blocks);
	free(files);
}

void initGlobals()
{
	configurar();
	CommandInterpreter_Init();
}

void freeGlobals()
{
	free(config->puntoMontaje);
	free(config->bitmapFile);
	free(config->blocksPath);
	free(config->metadataFile);
	free(config->filesPath);
	free(config);
}

void onPacketArrived(int socketID, int message_type, void* data)
{

	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	OnArrivedData* arriveData;
	arriveData = SocketServer_CreateOnArrivedData();

	arriveData->calling_SocketID = socketID;
	arriveData->receivedData = data;

	run->data = (void*)arriveData;

	//TODO: al no haber un free data para arrive data (run->free_data) podrian haber memory leaks si no se ejecuta el job que se manda al pool!

	switch(message_type)
	{
		case MESSAGETYPE_MDJ_CHECKFILE:
			run->runnable = (void*)MDJ_CheckFile;
		break;
		case MESSAGETYPE_MDJ_CREATEFILE:
			run->runnable = (void*)MDJ_CreateFile;
		break;
		case MESSAGETYPE_MDJ_GETDATA:
			run->runnable = (void*)MDJ_GetData;
		break;
		case MESSAGETYPE_MDJ_PUTDATA:
			run->runnable = (void*)MDJ_PutData;
		break;
		case MESSAGETYPE_MDJ_DELETEFILE:
			run->runnable = (void*)MDJ_DeleteFile;
		break;

		default:
			free(run);
			free(arriveData);
			free(data);
			run = NULL;
			break;
	}

	if(run != NULL)
		ThreadPool_AddJob(threadPool, run);
}

void ClientConnected(int socket)
{
	printf("Cliente se conecto! %d\n", socket);
}

void ClientDisconnected(int socket)
{
	printf("Cliente se desconecto! %d\n", socket);
}

void ClientError(int socketID, int errorCode)
{
	printf("Cliente %d se reporto con error %s!\n", socketID, strerror(errorCode));
}

void OnPostInterpreter(char* cmd, char* sep, void* args, bool actionFired)
{
	free(cmd);
}

void ProcessLineInput(char* line)
{
	ThreadableDoStructure* st = CommandInterpreter_MallocThreadableStructure();

	st->commandline = line;
	st->data = NULL;
	st->separator = " ";
	st->postDo = (void*)OnPostInterpreter;

	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	run->data = (void*)st;
	run->runnable = (void*)CommandInterpreter_DoThreaded;
	run->free_data = (void*)CommandInterpreter_FreeThreadableDoStructure;

	ThreadPool_AddJob(threadPool, run);
}

void startServer()
{
	threadPool = ThreadPool_CreatePool(10, false);
	SocketServer_Start("MDJ", config->puertoEscucha);
	SocketServer_ActionsListeners actions = INIT_ACTION_LISTENER;
	CommandInterpreter_RegisterCommand("iam", (void*)CommandIAm);

	actions.OnConsoleInputReceived = (void*)ProcessLineInput;
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

