#include "headers/MDJlib.h"
#include "kemmens/StringUtils.h"
#include "kemmens/SocketMessageTypes.h"
#include "kemmens/SocketServer.h"
#include "headers/MDJInterface.h"
#include "kemmens/Utils.h"
#include <openssl/md5.h>

char* CONTEXT_CURRENT_PATH_LINUX;
char* CONTEXT_CURRENT_PATH;

void configurar()
{
	config = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"PUERTO",
			"PUNTO_MONTAJE",
			"RETARDO",
			NULL
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
			"MAGIC_NUMBER",
			NULL
	};

	char* bitmap = StringUtils_Format("%s%s", config->puntoMontaje, RUTA_BITMAP);
	config->bitmapFile = malloc(strlen(bitmap) + 1);
	strcpy(config->bitmapFile, bitmap);

	char* files = StringUtils_Format("%s%s", config->puntoMontaje, RUTA_ARCHIVOS);
	config->filesPath = malloc(strlen(files) + 1);
	strcpy(config->filesPath, files);

	CONTEXT_CURRENT_PATH_LINUX = string_duplicate(config->filesPath);


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


	CONTEXT_CURRENT_PATH = (char*)malloc(2);
	CONTEXT_CURRENT_PATH[0] = '/';
	CONTEXT_CURRENT_PATH[1] = '\0';

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
	free(CONTEXT_CURRENT_PATH);
	free(CONTEXT_CURRENT_PATH_LINUX);
	free(config);
}

void onPacketArrived(int socketID, int message_type, void* data, int message_length)
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
	if(!actionFired)
		printf("Comando desconocido.\n");

	free(cmd);
}

void ProcessLineInput(char* line)
{
	//Mandamos el procesamiento de comandos al ThreadPool usando la estructura de las Kemmens ThreadableDoStructure
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

	//comando de identificacion de las partes del sistema
	CommandInterpreter_RegisterCommand("iam", (void*)CommandIAm);

	CommandInterpreter_RegisterCommand("quit", (void*)CommandQuit);

	//comandos de consola:
	CommandInterpreter_RegisterCommand("ls", (void*)Command_ls);
	CommandInterpreter_RegisterCommand("cd", (void*)Command_cd);
	CommandInterpreter_RegisterCommand("md5", (void*)Command_md5);
	CommandInterpreter_RegisterCommand("cat", (void*)Command_cat);

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

void *CommandQuit (int argC, char** args, char* callingLine, void* extraData)
{
	SocketServer_Stop();
	CommandInterpreter_FreeArguments(args);
	return 0;
}

void md5(void* content, unsigned char* digest)
{
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, content, strlen(content));
	MD5_Final(digest, &context);
}

//Comandos de consola
void *Command_ls (int argC, char** args, char* callingLine, void* extraData)
{
	char* parampath = string_new();

	for(int i = 1; i <= argC;i++)
	{
		string_append(&parampath, args[i]);
	}

	void freeall(char* ppath, char** aargs)
	{
		free(ppath);
		CommandInterpreter_FreeArguments(aargs);
	}

	char* FSpath = BuildPath(parampath, false);
	free(parampath);

	char* path = StringUtils_Format("%s%s", config->filesPath, FSpath);
	printf("ls: %s\n", FSpath);
	free(FSpath);

	struct dirent *de;

	DIR *dr = opendir(path);

	if (dr == NULL)
	{
		Logger_Log(LOG_ERROR, "Comando 'ls' -> imposible listar archivos y directorios opendir retorno NULL.");
		freeall(path, args);
		return 0;
	}

	while ((de = readdir(dr)) != NULL)
		printf("Nombre: %s \t Tipo: %s \n", de->d_name, ((de->d_type == 8) ? "FILE" : "DIR"));

	printf("\n");

	closedir(dr);

	freeall(path, args);
	return 0;
}


void *Command_cd (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC >= 1)
	{
		char* path = string_new();

		for(int i = 1; i <= argC;i++)
		{
			string_append(&path, args[i]);
		}

		char* finalPath = BuildPath(path, false);

		if(finalPath == NULL)
		{
			printf("cd: %s no es un directorio valido\n", path);
		} else
		{
			free(CONTEXT_CURRENT_PATH);
			CONTEXT_CURRENT_PATH = finalPath;
			free(CONTEXT_CURRENT_PATH_LINUX);
			CONTEXT_CURRENT_PATH_LINUX = StringUtils_Format("%s%s", config->filesPath, CONTEXT_CURRENT_PATH);
			printf("cd: Working Directory %s\n", CONTEXT_CURRENT_PATH);
		}

		free(path);
	}

	CommandInterpreter_FreeArguments(args);
	return 0;
}

void *Command_md5 (int argC, char** args, char* callingLine, void* extraData)
{
	if(argC > 0)
	{
		char* passedPath = string_new();

		for(int i = 1; i <= argC;i++)
		{
			string_append(&passedPath, args[i]);
		}

		char* path = BuildPath(passedPath, true);
		free(passedPath);

		if(path == NULL)
		{
			printf("md5: El path ingresado no es valido.\n");
			return 0;
		}

		int cop = 0;

		t_config* metadata = FIFA_OpenFile(path);

		if(metadata == NULL)
		{
			printf("El path ingresado no es valido.\n");
			free(path);
			CommandInterpreter_FreeArguments(args);
			return 0;
		}

		int fileSize = config_get_int_value(metadata, "TAMANIO");

		config_destroy(metadata);

		char* cont = FIFA_ReadFile(path, 0, fileSize, &cop);

		cont = realloc(cont, cop + 1);

		cont[cop] = '\0';

		unsigned char digest[MD5_DIGEST_LENGTH];

		md5(cont, digest);

		printf("md5 %s = ", path);

		for(int x = 0; x < MD5_DIGEST_LENGTH; x++)
		        printf("%02x", digest[x]);

		printf("\n");

		//free(digest);
		free(cont);
		free(path);
	}

	CommandInterpreter_FreeArguments(args);
	return 0;
}

void *Command_cat (int argC, char** args, char* callingLine, void* extraData)
{
	if(argC > 0)
	{
		char* passedPath = string_new();

		for(int i = 1; i <= argC;i++)
		{
			string_append(&passedPath, args[i]);
		}

		char* path = BuildPath(passedPath, true);
		free(passedPath);

		if(path == NULL)
		{
			printf("cat: El path ingresado no es valido.\n");
			return 0;
		}

		int cop = 0;

		t_config* metadata = FIFA_OpenFile(path);

		if(metadata == NULL)
		{
			printf("El path ingresado no es valido.\n");
			free(path);
			CommandInterpreter_FreeArguments(args);
			return 0;
		}

		int fileSize = config_get_int_value(metadata, "TAMANIO");

		config_destroy(metadata);

		char* cont = FIFA_ReadFile(path, 0, fileSize, &cop);

		cont = realloc(cont, cop + 1);

		cont[cop] = '\0';

		printf("Cat %s (%d bytes):\n%s\n", path, cop, cont);

		free(cont);
		free(path);
	}

	CommandInterpreter_FreeArguments(args);
	return 0;
}

//Constructores de paths dinamicos
char* BuildPath(char* path, bool hasFileAtEnd)
{
	char* tmp;
	int error = 0;

	if(strcmp(path, "/") == 0) //queremos ir a root (caso particular)
	{
		tmp = string_duplicate("/");
	} else
	{
		char** parts = string_split(path, "/");

		if(string_starts_with(path, "/")) //es un path absoluto
		{
			tmp = string_duplicate("/");
		} else //es un path relativo
		{
			tmp = string_duplicate(CONTEXT_CURRENT_PATH);
		}
		//Ejemplo ruta actual: /hola/como/te/va
		//el escaneo recursivo permite acceder por ejemplo haciendo cd ./../va/.. -> terminaria en /hola/como/te/
		int count = StringUtils_ArraySize(parts);
		int max = ((hasFileAtEnd) ? count-1 : count);

		for(int i = 0; i < max; i++) {

			if(cd(parts[i], &tmp) == 0)
			{
				free(tmp);
				error = 1;
				break;
			}
		}

		if(error == 0 && hasFileAtEnd)
		{
			string_append(&tmp, parts[count-1]);
		}

		StringUtils_FreeArray(parts);
	}

	if(error == 1)
		return NULL;

	return tmp;
}

int cd(char* path, char** tmp)
{
	if(strcmp(path, "..") == 0)
	{
		if(StringUtils_CountOccurrences(*tmp, "/") > 1) //si no estamos en /
		{
			char* temp = string_substring(*tmp, 0, StringUtils_LastIndexOf(*tmp, '/'));
			char* posta = string_substring(temp, 0, StringUtils_LastIndexOf(temp, '/') + 1);
			free(temp);
			free(*tmp);
			*tmp = posta;
			return 1;
		}

		return 0;
	} else if(strcmp(path, ".") == 0)
	{
		return 1;
	} else
	{
		char* temp = StringUtils_Format("%s%s", *tmp, path);
		char* linuxFS = StringUtils_Format("%s%s", config->filesPath, temp);

		DIR* dir = opendir(linuxFS);
		free(linuxFS);
		if (dir)
		{
			string_append(&temp, "/");
			free(*tmp);
			*tmp = temp;
		    closedir(dir);
		    return 1;
		}

		free(temp);
		return 0;
	}
}


