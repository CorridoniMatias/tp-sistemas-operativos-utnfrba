#include "headers/bibliotecaDAM.h"

void configurar()
{

	//Pido memoria para el struct
	settings = (Configuracion*)malloc(sizeof(Configuracion));

	//Guardo los nombres literales de los campos del archivo de config en un array; asi puedo buscarlos con la funcion!
	char* campos[] = {
			"PUERTO_ESCUCHA",
			"IP_SAFA",
			"PUERTO_SAFA",
			"IP_MDJ",
			"PUERTO_MDJ",
			"IP_FM9",
			"PUERTO_FM9",
			"TRANSFER_SIZE",
			NULL
	};

	//Creo un puntero al archivo de configuracion
	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Copio los valores de los campos leidos al struct de configuracion
	settings->puertoEscucha = archivoConfigSacarIntDe(archivoConfig, "PUERTO_ESCUCHA");
	strcpy(settings->ipSAFA, archivoConfigSacarStringDe(archivoConfig, "IP_SAFA"));
	strcpy(settings->puertoSAFA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_SAFA"));
	strcpy(settings->ipMDJ, archivoConfigSacarStringDe(archivoConfig, "IP_MDJ"));
	strcpy(settings->puertoMDJ, archivoConfigSacarStringDe(archivoConfig, "PUERTO_MDJ"));
	strcpy(settings->ipFM9, archivoConfigSacarStringDe(archivoConfig, "IP_FM9"));
	strcpy(settings->puertoFM9, archivoConfigSacarStringDe(archivoConfig, "PUERTO_FM9"));
	settings->transferSize = archivoConfigSacarIntDe(archivoConfig, "TRANSFER_SIZE");

	//LoS dejo aca por si vuelvo a usarlos
	//strcpy(settings->puertoEscucha, archivoConfigSacarStringDe(archivoConfig, "PUERTO_ESCUCHA"));
	//settings->puertoSAFA = archivoConfigSacarIntDe(archivoConfig, "PUERTO_SAFA");
	//settings->puertoMDJ = archivoConfigSacarIntDe(archivoConfig, "PUERTO_MDJ");
	//settings->puertoFM9 = archivoConfigSacarIntDe(archivoConfig, "PUERTO_FM9");

	archivoConfigDestruir(archivoConfig);

}

void inicializarVariablesGlobales()
{
	configurar();
	CommandInterpreter_Init();
	threadPool = ThreadPool_CreatePool(14, false);
}

void liberarVariablesGlobales()
{
	free(settings);
	//Hace falta liberar los campos de settings? No hice mallocs, y son arrays estaticos de caracteres. No, no hace falta.
	ThreadPool_FreeGracefully(threadPool);
}
/*
int conectarAProceso(char* ip, char* puerto, char* nombreProceso)
{

	int socket;

	//printf("Voy a entrar al ConnectToServerIP\n");

	socket = SocketClient_ConnectToServerIP(ip, puerto);	//Me conecto al servidor que me dijeron

	if(socket == -1)
	{
		Logger_Log(LOG_ERROR, "Error al conectar el DAM al proceso %s!", nombreProceso);
		return -1;
	}

	Logger_Log(LOG_INFO, "DAM conectado al proceso %s!", nombreProceso);	//Logueo que me conecte

	SocketCommons_SendMessageString(socket, "iam ElDiego");

	return socket;

}
*/
/* Version con IP por parametro; por ahora no la usamos, la dejo aca
 *
int conectarAProceso(char* ip, char* puerto, char* nombreProceso)
{

	int socket;
	socket = SocketClient_ConnectToServerIP(ip, puerto);	//Me conecto al servidor que me dijeron

	if(socket == -1)
	{
		Logger_Log(LOG_ERROR, "Error al conectar el DAM al proceso %s!", nombreProceso);
		return -1;
	}

	Logger_Log(LOG_INFO, "DAM conectado al proceso %s!", nombreProceso);	//Logueo que me conecte
	SocketCommons_SendMessageString(socket, "iam ElDiego");

	free(nombreProceso);				//Anti-memory leak

	return socket;

}
*/
/*
void esperarRespuesta(void* socket)
{

	int socketPosta = (int) socket;			//Lo casteo para no tener warnings; el hilo me lo mando como un void*
	int bytesRecibidos;						//Retorno para verificar
	char mensaje[256];						//Hardcodeado de momento, es solo para la prueba

	//Aca me bloqueo esperando a que me contesten algo
	bytesRecibidos = recv(socketPosta, (void*) mensaje, 256, 0);
	if(bytesRecibidos < 0)
	{
		Logger_Log(LOG_ERROR, "Error en la recepcion de mensajes a traves del socket %d", socketPosta);
	}
	else if(bytesRecibidos == 0)
	{
		SocketCommons_CloseSocket(socketPosta);
	}
	else if(bytesRecibidos > 0)
	{
		printf("\nA traves del socket descripto por %d llego el mensaje \"%s\" \n", socketPosta, mensaje);
		SocketCommons_CloseSocket(socketPosta);
		printf("Se va a cerrar el hilo... :(\n");
	}
	return;

}
*/


void levantarServidor()
{

	SocketServer_Start("DAM", settings->puertoEscucha);
	SocketServer_ActionsListeners acciones = INIT_ACTION_LISTENER;
	CommandInterpreter_RegisterCommand("iam", (void*)comandoIAm);

	acciones.OnConsoleInputReceived = NULL;						//El Diego no tiene consola, no es necesaria (creo)
	acciones.OnPacketArrived = (void*)llegoUnPaquete;
	acciones.OnClientConnected = (void*)clienteConectado;
	acciones.OnClientDisconnect = (void*)clienteDesconectado;
	acciones.OnReceiveError = NULL;								//A definir a futuro

	settings->socketSAFA = SocketClient_ConnectToServerIP(settings->ipSAFA, settings->puertoSAFA);

	if(settings->socketSAFA != -1)
	{
		SocketCommons_SendMessageString(settings->socketSAFA, "iam dam"); //handshake con DAM
		SocketServer_ListenForConnection(acciones);
	}
	else
		Logger_Log(LOG_ERROR, "Error al conectar al SAFA. Cancelando ejecucion...");

	Logger_Log(LOG_INFO, "Se cierra el servidor");

}

void* comandoIAm (int argc, char** args, char* comando, void* datos)
{
	if(argc == 1)
	{
		if(string_equals_ignore_case(args[0], "cpu"))
		{
			list_add(cpus, datos);
		}
		else
		{
			printf("Proceso no-CPU intento conectarse; denegado\n");
		}
	}

	CommandInterpreter_FreeArguments(args);
	return 0;
}

void clienteConectado(int socket)
{

	Logger_Log(LOG_INFO, "Nuevo cliente conectado. Comunicacion a traves del socket %d", socket);
	return;

}

void clienteDesconectado(int unSocket)
{

	Logger_Log(LOG_INFO, "Desconexion con el cliente que estaba en el socket %d", unSocket);
	return;

}

void llegoUnPaquete(int socketID, int message_code, void* datos, int message_length)
{
	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	OnArrivedData* arriveData;
	arriveData = SocketServer_CreateOnArrivedData();

	arriveData->calling_SocketID = socketID;
	arriveData->receivedData = datos;
	arriveData->receivedDataLength = message_length;
	arriveData->message_type = message_code;

	run->data = (void*)arriveData;

	//TODO: al no haber un free data para arrive data (run->free_data) podrian haber memory leaks si no se ejecuta el job que se manda al pool!

	switch(message_code)
	{
		case MESSAGETYPE_CPU_FLUSH:
			run->runnable = (void*)DAM_Flush;
		break;
		case MESSAGETYPE_CPU_CREAR:
			run->runnable = (void*)DAM_Crear;
		break;
		case MESSAGETYPE_CPU_BORRAR:
			run->runnable = (void*)DAM_Borrar;
		break;
		case MESSAGETYPE_CPU_EXECDUMMY:
		case MESSAGETYPE_CPU_ABRIR:
			run->runnable = (void*)DAM_Abrir;
		break;

		default:
			free(run);
			free(arriveData);
			free(datos);
			run = NULL;
			break;
	}

	if(run != NULL)
		ThreadPool_AddJob(threadPool, run);
}

void* aRealizar(char* cmd, char* sep, void* args, bool fired)
{
	/*if(!fired)
		SocketCommons_SendMessageString((int)args, "Lo recibido no es comando!");*/

	free(cmd);
	return 0;
}
