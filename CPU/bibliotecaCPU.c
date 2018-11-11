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
			"RETARDO"
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
	char* logicDir = (char*)malloc(sizeof(dtb.parts[2]));
	int* pc = (int*)malloc(4);
	int* code = (int*)malloc(4);


	*code = 0;

	SerializedPart fieldForFM91 = {.size = 4, .data = idDtb};
	SerializedPart fieldForFM92 = {.size = sizeof(dtb.parts[2]), .data = logicDir};
	SerializedPart fieldForFM93 = {.size = 4, .data = code};

	void* packetToFM9 = Serialization_Serialize(4, fieldForFM91, fieldForFM92, fieldForFM93);

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

void* CommandConcentrar(int argC, char** args, char* callingLine, void* extraData){
	sleep(settings->retardo);
	StringUtils_FreeArray(args);
	return 0;
	}

void* CommandAsignar(){
return 0;
}

void* CommandWait(int argC, char** args, char* callingLine, void* extraData){
	if(argC == 1){

		//LE MANDO AL SAFA IDDTB|PC|QUANTUM|RECURSO|CODIGO
		SerializedPart fieldForSAFA1 = {.size = 4, .data =((Operation*)extraData)->dtb};
		SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = ((Operation*)extraData)->programCounter};
		SerializedPart fieldForSAFA3 = {.size = 4, .data = ((Operation*)extraData)->quantum};
		SerializedPart fieldForSAFA4 = {.size = sizeof(args[0]) , .data = args[0]};
		//Se define codigo nuevo - 5.Solicitud de retencion del recurso
		SerializedPart fieldForSAFA5 = {.size = 4 , .data = 5};
		void* packetToSAFA = Serialization_Serialize(4, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4, fieldForSAFA5);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_VOIDPOINTER, packetToSAFA, sizeof(packetToSAFA));

		int messageType,err;
		void* msgFromSafa = SocketCommons_ReceiveData(((Operation*)extraData)->socketSAFA,&messageType,&err);
		DeserializedData data;
		Serialization_Deserialize(msgFromSafa,&data);
		//TODO Hablar con PEPE pero suponiendo que el mensaje que me mando es solo el codigo Ej |code|
		//Siendo code = 0, pudo ser asignado y code = 1, no pudo ser asignado. Quedaria de la siguente forma
		if(*(int*)data.parts[0] == 0 ){

			return 0;
		}
	}
	SerializedPart fieldForSAFA1 = {.size = 4, .data =((Operation*)extraData)->dtb};
	SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = ((Operation*)extraData)->programCounter};
	SerializedPart fieldForSAFA3 = {.size = 4, .data = ((Operation*)extraData)->quantum};
	SerializedPart fieldForSAFA4 = {.size = sizeof(args[0]) , .data = args[0]};
	SerializedPart fieldForSAFA5 = {.size = 4 , .data = 4};
	void* packetToSAFAToBlockGDT = Serialization_Serialize(4, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4, fieldForSAFA5);

	SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_VOIDPOINTER,packetToSAFAToBlockGDT, sizeof(packetToSAFAToBlockGDT));
	return 1;
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

t_dictionary* BuildDictionary(void* flattened, int amount)
{

	t_dictionary* dict = dictionary_create();
	int i = 1;
	int offset = 0;
	int* logicalAddress;
	while(i <= amount)
	{
		char* path;
		//OJO: NO VA EL FREE DE ESTE PUNTERO ACA, SINO PIERDO LA REFERENCIA DEL ULTIMO PUT
		//EL FREE SE HACE SOLO CUANDO DESTRUYA EL DICCIONARIO Y SUS ELEMENTOS
		logicalAddress = malloc(sizeof(int));
		path = strtok((char*)(flattened + offset), ":");
		//No le sumo uno por los :, strlen me devuelve el largo + 1 por el \0 al final
		offset += (strlen(path) + 1);
		memcpy(logicalAddress, flattened + offset, sizeof(int));
		offset += (sizeof(int) + 1);
		dictionary_put(dict, path, logicalAddress);
		i++;
	}

	return dict;

}
