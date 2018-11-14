#include "bibliotecaCPU.h"

void configurar()
{
	settings = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"IP_SAFA",
			"PUERTO_SAFA",
			"IP_DIEGO",
			"PUERTO_DIEGO",
			"IP_FM9",
			"PUERTO_FM9"
			"RETARDO",
			NULL
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	strcpy(settings->ipSAFA, archivoConfigSacarStringDe(archivoConfig, "IP_SAFA"));
	strcpy(settings->puertoSAFA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_SAFA"));
	strcpy(settings->ipDIEGO, archivoConfigSacarStringDe(archivoConfig, "IP_DIEGO"));
	strcpy(settings->puertoDIEGO, archivoConfigSacarStringDe(archivoConfig, "PUERTO_DIEGO"));
	strcpy(settings->ipFM9, archivoConfigSacarStringDe(archivoConfig, "IP_FM9"));
	strcpy(settings->puertoFM9, archivoConfigSacarStringDe(archivoConfig, "PUERTO_FM9"));
	settings->retardo = archivoConfigSacarIntDe(archivoConfig, "RETARDO");


	archivoConfigDestruir(archivoConfig);

}

int conectarAProceso(char* ip, char* puerto, char* nombreProceso)
{
	int socket;
	socket = SocketClient_ConnectToServerIP(ip, puerto);	//Me conecto al servidor que me dijeron

	if(socket == -1)
	{
		Logger_Log(LOG_ERROR, "Error al conectar el CPU al proceso %s!", nombreProceso);
		return -1;
	}

	Logger_Log(LOG_INFO, "CPU conectado al proceso %s!", nombreProceso);	//Logueo que me conecte
	SocketCommons_SendMessageString(socket, "iam CPU");

	free(nombreProceso);				//Anti-memory leak

	return socket;


}

void executeDummy(DeserializedData dtb, int diego, int safa){
	int* idDtb = (int*)malloc(4);
	char* path = (char*)malloc(sizeof(dtb.parts[1]));
	int* pc = (int*)malloc(4);
	int* quantum = (int*)malloc(4);
	int* code = (int*)malloc(4);

	SerializedPart fieldForDiego1 = {.size = 4, .data = idDtb};
	SerializedPart fieldForDiego2 = {.size = sizeof(dtb.parts[1]), .data = path};
	void* packetToDiego = Serialization_Serialize(2, fieldForDiego1, fieldForDiego2);

	SocketCommons_SendData(diego, MESSAGETYPE_VOIDPOINTER, packetToDiego, sizeof(packetToDiego));

	SerializedPart fieldForSafa1 = {.size = 4, .data = idDtb};
	SerializedPart fieldForSafa2 = {.size = 4, .data = pc};
	SerializedPart fieldForSafa3 = {.size = 4, .data = quantum};
	SerializedPart fieldForSafa4 = {.size = 4, .data = code};
	void* packetToSafa = Serialization_Serialize(4, fieldForSafa1, fieldForSafa2, fieldForSafa3, fieldForSafa4);

	SocketCommons_SendData(safa,MESSAGETYPE_VOIDPOINTER, packetToSafa, sizeof(packetToSafa));
}

char* askLineToFM9(DeserializedData dtb, int fm9){

	int* idDtb = (int*)malloc(4);
	char* path = (char*)malloc(sizeof(dtb.parts[1]));
	int* pc = (int*)malloc(4);
	int* code = (int*)malloc(4);

	*code = 0;

	SerializedPart fieldForFM91 = {.size = 4, .data = idDtb};
	SerializedPart fieldForFM92 = {.size = sizeof(dtb.parts[1]), .data = path};
	SerializedPart fieldForFM93 = {.size = 4, .data = pc};
	SerializedPart fieldForFM94 = {.size = 4, .data = code};

	void* packetToFM9 = Serialization_Serialize(4, fieldForFM91, fieldForFM92, fieldForFM93, fieldForFM94);

	SocketCommons_SendData(fm9,MESSAGETYPE_VOIDPOINTER, packetToFM9, sizeof(packetToFM9));

	// Me quedo esperando hasta que me devuelva la linea para ejecutar
	int messageType, err;

	void* msgFromFM9 = SocketCommons_ReceiveData(fm9,&messageType,&err);

	DeserializedData data;
	Serialization_Deserialize(msgFromFM9,&data);

	if(*(int*)data.parts[1] == 0){
		return (char *)data.parts[0];
	}
		return "error"; //TODO cambiar por algo para saber verdaderamente que no es una linea de codigo
}

void* CommandAbrir(){
return 0;
}

void* CommandConcentrar(){
return 0;
}

void* CommandAsignar(){
return 0;
}

void* CommandWait(){
return 0;
}

void* CommandSignal(){
return 0;
}

void* CommandFlush(){
return 0;
}

void* CommandClose(){
return 0;
}

void* CommandCrear(){
return 0;
}

void* CommandBorrar(){
return 0;
}


void waitSafaOrders(){




}
