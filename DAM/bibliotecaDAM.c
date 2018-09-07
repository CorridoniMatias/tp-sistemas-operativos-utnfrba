#include "bibliotecaDAM.h"

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
			"TRANSFER_SIZE"
	};

	//Creo un puntero al archivo de configuracion
	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Copio los valores de los campos leidos al struct de configuracion
	strcpy(settings->puertoEscucha, archivoConfigSacarStringDe(archivoConfig, "PUERTO_ESCUCHA"));
	strcpy(settings->ipSAFA, archivoConfigSacarStringDe(archivoConfig, "IP_SAFA"));
	strcpy(settings->puertoSAFA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_SAFA"));
	strcpy(settings->ipMDJ, archivoConfigSacarStringDe(archivoConfig, "IP_MDJ"));
	strcpy(settings->puertoMDJ, archivoConfigSacarStringDe(archivoConfig, "PUERTO_MDJ"));
	strcpy(settings->ipFM9, archivoConfigSacarStringDe(archivoConfig, "IP_FM9"));
	strcpy(settings->puertoFM9, archivoConfigSacarStringDe(archivoConfig, "PUERTO_FM9"));
	settings->transferSize = archivoConfigSacarIntDe(archivoConfig, "TRANSFER_SIZE");

	//Libero memoria, para no mandarnos cagadas;
	free(campos);
	archivoConfigDestruir(archivoConfig);

}

void inicializarVariablesGlobales()
{

	configurar();
	CommandInterpreter_Init();
	pozoDeHebras = ThreadPool_CreatePool(14, false);

}

void liberarVariablesGlobales()
{

	free(settings);
	//Hace falta liberar los campos de settings? No hice mallocs, y son arrays estaticos de caracteres
	ThreadPool_FreeGracefully(pozoDeHebras);
	//No libero el Command Interpreter, total el exit_gracefully lo hace

}

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
		cerrarSocket(socketPosta);
	}
	else if(bytesRecibidos > 0)
	{
		printf("\nA traves del socket descripto por %d llego el mensaje \"%s\" \n", socketPosta, mensaje);
		cerrarSocket(socketPosta);
		printf("Se va a cerrar el hilo... :(\n");
	}
	return;

}

void levantarServidor()
{

	SocketServer_Start("SAFA", settings->puertoEscucha);
	SocketServer_ActionsListeners acciones = INIT_ACTION_LISTENER;
	CommandInterpreter_RegisterCommand("iam", (void*)comandoIAm);

	acciones.OnConsoleInputReceived = NULL;						//El Diego no tiene consola, no es necesaria (creo)
	acciones.OnPacketArrived = (void*)llegoUnPaquete;
	acciones.OnClientConnected = (void*)clienteConectado;
	acciones.OnClientDisconnect = (void*)clienteDesconectado;
	acciones.OnReceiveError = NULL;								//A definir a futuro
	SocketServer_ListenForConnection(acciones);
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
	printf("Se conecto un cliente; hablen a traves del socket %d!", socket);
	return;

}

void clienteDesconectado(int unSocket)
{

	Logger_Log(LOG_INFO, "Desconexion con el cliente que estaba en el socket %d", socket);
	printf("Se desconecto el cliente del socket %d :(", socket);
	return;

}

void llegoUnPaquete(int socket, int tipoMensaje, void* datos)
{
	if(tipoMensaje == MESSAGETYPE_STRING)
	{
		ThreadableDoStructure* st = CommandInterpreter_MallocThreadableStructure();

		st->commandline = (char*)datos;
		st->data = (void*)socket;
		st->separator = " ";
		st->postDo = (void*)aRealizar;				//Aca si manquie :P, no seria mas facil poner un free y listo?

		ThreadPoolRunnable* ejecutable = ThreadPool_CreateRunnable();

		ejecutable->data = (void*)st;
		ejecutable->runnable = (void*)CommandInterpreter_DoThreaded;
		//Aca si necesitamos decirle al ThreadPool que libere data (o sea el st) en caso que se liberen todos los jobs
		ejecutable->free_data = (void*)CommandInterpreter_FreeThreadableDoStructure;

		ThreadPool_AddJob(pozoDeHebras, ejecutable);
	}
}

void* aRealizar(char* cmd, char* sep, void* args, bool fired)
{
	/*if(!fired)
		SocketCommons_SendMessageString((int)args, "Lo recibido no es comando!");*/

	free(cmd);
	return 0;
}
