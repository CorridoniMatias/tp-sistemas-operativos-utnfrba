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
			"PUERTO_FM9",
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


	return socket;


}

void executeDummy(DeserializedData dtb, int diego, int safa){
	uint32_t* idDtb = (uint32_t*)malloc(4);
	*idDtb = (uint32_t)dtb.parts[0];
	char* path = (char*)malloc(sizeof(dtb.parts[1]));
	strcpy(path,(char*)dtb.parts[1]);
	int* pc = (int*)malloc(4);
	*pc = (int)dtb.parts[3];
	int* quantum = (int*)malloc(4);
	*quantum = (int)dtb.parts[4];

	SerializedPart fieldForDiego1 = {.size = 4, .data = idDtb};
	SerializedPart fieldForDiego2 = {.size = sizeof(dtb.parts[1]), .data = &path};
	void* packetToDiego = Serialization_Serialize(2, fieldForDiego1, fieldForDiego2);

	SocketCommons_SendData(diego, MESSAGETYPE_VOIDPOINTER, packetToDiego, sizeof(packetToDiego));

	SerializedPart fieldForSafa1 = {.size = 4, .data = &idDtb};


	void* packetToSafa = Serialization_Serialize(1,fieldForSafa1);

	SocketCommons_SendData(safa,MESSAGETYPE_CPU_BLOCKDUMMY, packetToSafa, sizeof(packetToSafa));
	free(idDtb);
	free(path);
	free(pc);
	free(quantum);

}

char* askLineToFM9(DeserializedData dtb, int fm9){

	uint32_t* idDtb = (uint32_t*)malloc(4);
	*idDtb = (uint32_t)dtb.parts[0];
	uint32_t* logicDir = (uint32_t*)malloc(sizeof(dtb.parts[2]));
	*logicDir = (uint32_t)dtb.parts[2];
	int*pc = (int*)malloc(4);
	*pc = (int)dtb.parts[3];

	*logicDir = *logicDir + *pc - 1;  //Sumar 1 a logicDir para pedir la siguiente linea


	SerializedPart fieldForFM91 = {.size = 4, .data = idDtb};
	SerializedPart fieldForFM92 = {.size = sizeof(dtb.parts[2]), .data = logicDir};

	void* packetToFM9 = Serialization_Serialize(2, fieldForFM91, fieldForFM92);

	SocketCommons_SendData(fm9,MESSAGETYPE_CPU_ASKLINE, packetToFM9, sizeof(packetToFM9));

	// Me quedo esperando hasta que me devuelva la linea para ejecutar
	int messageType, err, msglength;

	void* msgFromFM9 = SocketCommons_ReceiveData(fm9,&messageType,&msglength,&err);

	DeserializedData data;
	Serialization_Deserialize(msgFromFM9,&data);

	if(*(int*)data.parts[0] == 1){
		free(idDtb);
		free(logicDir);
		return (char *)data.parts[1];
	}
		free(idDtb);
		free(logicDir);
		Logger_Log(LOG_INFO, "Error fallo de segmento/memoria en FM9");
		return "error"; //
}

void* CommandAbrir(int argC, char** args, char* callingLine, void* extraData){
	if(openFileVerificator(((Operation*)extraData)->dictionary,args[0])){
		Logger_Log(LOG_INFO, "El archivo ya se encuentra abierto");
		return 0;
	}
	else{
		SerializedPart fieldForDAM1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		SerializedPart fieldForDAM2 = {.size = sizeof(args[0]), .data = &args[0]};

		void* packetToDAM = Serialization_Serialize(2, fieldForDAM1, fieldForDAM2);

		SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_ABRIR, packetToDAM, sizeof(packetToDAM));

		SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = &(((Operation*)extraData)->programCounter)};
		SerializedPart fieldForSAFA3 = {.size = 4, .data = &(((Operation*)extraData)->quantum)};
		int numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
		SerializedPart fieldForSAFA4 = {.size = 4 , .data = &numberOfFiles};
	    SerializedPart fieldForSAFA5 = {.size = sizeof(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
		void* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT, sizeof(packetToSAFAToBlockGDT));
	    StringUtils_FreeArray(args);

		((Operation*)extraData)->commandResult = 2; //el 2 significa hacer un break

		//TODO me quedo esperando alguna respuesta o sigo con mi operatoria?


	}
return 0;
}

void* CommandConcentrar(int argC, char** args, char* callingLine, void* extraData){
	sleep(settings->retardo);
	StringUtils_FreeArray(args);
	return 0;
	}

void* CommandAsignar(int argC, char** args, char* callingLine, void* extraData){
	if(argC == 3){
		if(openFileVerificator(((Operation*)extraData)->dictionary,args[0])){

			uint32_t logicDirToWrite = *((uint32_t*)dictionary_get(((Operation*)extraData)->dictionary,args[0]));
			uint32_t lineToWrite = atoi(args[1]);
			char* dataToWrite = args[2];
			logicDirToWrite = ((Operation*)extraData)->programCounter + lineToWrite - 1;
			SerializedPart fieldForFM91 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
			SerializedPart fieldForFM92 = {.size = 4, .data =&logicDirToWrite};
			SerializedPart fieldForFM93 = {.size = sizeof(dataToWrite), .data = &dataToWrite};


			void* packetToFM9 = Serialization_Serialize(3, fieldForFM91, fieldForFM92, fieldForFM93);


			SocketCommons_SendData(((Operation*)extraData)->socketFM9,MESSAGETYPE_CPU_ASIGNAR, packetToFM9, sizeof(packetToFM9));

			int messageType, err, msglength;

			void* msgFromFM9 = SocketCommons_ReceiveData(((Operation*)extraData)->socketFM9,&messageType,&msglength,&err);

			DeserializedData data;
			Serialization_Deserialize(msgFromFM9,&data);
			if(*(int*)data.parts[0] == 1){
				free(dataToWrite);
				((Operation*)extraData)->commandResult = 0; // 0 SALIO to BIEN
			}
			else {
				Logger_Log(LOG_INFO, "El archivo no se encuentra abierto");
				if(*(int*)data.parts[0] == 2){
					free(dataToWrite);
					Logger_Log(LOG_INFO, "Error fallo de segmento/memoria en FM9");

					SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};


					void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);
					SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));
					StringUtils_FreeArray(args);
					((Operation*)extraData)->commandResult = 2;
				}
				else {
					free(dataToWrite);
					Logger_Log(LOG_INFO, "Espacio insuficiente en FM9");

					SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};


					void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);
					SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));

					StringUtils_FreeArray(args);
					((Operation*)extraData)->commandResult = 2;
				}
			}

	}
		else {
			SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};


			void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);
			SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));
			((Operation*)extraData)->commandResult = 2; // hacer break

		}

		SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};


		void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);
		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));
		((Operation*)extraData)->commandResult = 2; // hacer break

	}
StringUtils_FreeArray(args);

return 0;
}

void* CommandWait(int argC, char** args, char* callingLine, void* extraData){

	if(argC == 1){
		SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		SerializedPart fieldForSAFA2 = {.size = sizeof(args[0]) , .data = &args[0]};

		void* packetToSAFA = Serialization_Serialize(2, fieldForSAFA1, fieldForSAFA2);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_WAIT, packetToSAFA, sizeof(packetToSAFA));

		int messageType,err,msglength;
		void* msgFromSafa = SocketCommons_ReceiveData(((Operation*)extraData)->socketSAFA,&messageType,&msglength,&err);
		DeserializedData data;
		Serialization_Deserialize(msgFromSafa,&data);

		//Siendo code = 1, pudo ser asignado y code = 0, no pudo ser asignado. Quedaria de la siguente forma
		if(*(int*)data.parts[0] == 1 ){
			StringUtils_FreeArray(args);
			((Operation*)extraData)->commandResult = 0; // 0 SALIO to BIEN
			return 0;
		}
		else {
			SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
			SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = &(((Operation*)extraData)->programCounter)};
			SerializedPart fieldForSAFA3 = {.size = 4, .data = &(((Operation*)extraData)->quantum)};
			uint32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
			SerializedPart fieldForSAFA4 = {.size = 4 , .data = &numberOfFiles};
			SerializedPart fieldForSAFA5 = {.size = sizeof(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
 			void* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

			SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT, sizeof(packetToSAFAToBlockGDT));
			StringUtils_FreeArray(args);
			((Operation*)extraData)->commandResult = 2;
			return 0;
		}
	}
	else {
		SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = &(((Operation*)extraData)->programCounter)};
		SerializedPart fieldForSAFA3 = {.size = 4, .data = &(((Operation*)extraData)->quantum)};
		uint32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
		SerializedPart fieldForSAFA4 = {.size = 4 , .data = &numberOfFiles};
		SerializedPart fieldForSAFA5 = {.size = sizeof(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
			void* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4, fieldForSAFA5);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT, sizeof(packetToSAFAToBlockGDT));
		StringUtils_FreeArray(args);
		((Operation*)extraData)->commandResult = 2;
		return 0;
	}

}

void* CommandSignal(int argC, char** args, char* callingLine, void* extraData){


	if(argC == 1){

		SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		SerializedPart fieldForSAFA2 = {.size = sizeof(args[0]) , .data = &args[0]};

		void* packetToSAFA = Serialization_Serialize(2, fieldForSAFA1, fieldForSAFA2);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_SIGNAL, packetToSAFA, sizeof(packetToSAFA));

		int messageType,err,msglength;
		void* msgFromSafa = SocketCommons_ReceiveData(((Operation*)extraData)->socketSAFA,&messageType,&msglength,&err);
		DeserializedData data;
		Serialization_Deserialize(msgFromSafa,&data);

		if(*(int*)data.parts[0] == 1 ){
			StringUtils_FreeArray(args);
			((Operation*)extraData)->commandResult = 0;
			return 0;
		}


	}
	StringUtils_FreeArray(args);
	((Operation*)extraData)->commandResult = 1;
	return 0;


}


void* CommandFlush(int argC, char** args, char* callingLine, void* extraData){
	if(!(openFileVerificator(((Operation*)extraData)->dictionary,args[0]))){

		SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));

		((Operation*)extraData)->commandResult = 2;
	    StringUtils_FreeArray(args);
		Logger_Log(LOG_INFO, "El archivo no se encuentra abierto");
		return 0;
	}
	else{
		SerializedPart fieldForDAM1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		SerializedPart fieldForDAM2 = {.size = sizeof(args[0]), .data = &args[0]};

		void* packetToDAM = Serialization_Serialize(2, fieldForDAM1, fieldForDAM2);

		SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_FLUSH, packetToDAM, sizeof(packetToDAM));

		SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
		SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = &(((Operation*)extraData)->programCounter)};
		SerializedPart fieldForSAFA3 = {.size = 4, .data = &(((Operation*)extraData)->quantum)};
		uint32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
		SerializedPart fieldForSAFA4 = {.size = 4 , .data = &numberOfFiles};
	    SerializedPart fieldForSAFA5 = {.size = sizeof(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
		void* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT, sizeof(packetToSAFAToBlockGDT));
	    StringUtils_FreeArray(args);

		((Operation*)extraData)->commandResult = 2; //el 2 significa hacer un break

		//TODO me quedo esperando alguna respuesta o sigo con mi operatoria?
	}
return 0;
}

void* CommandClose(int argC, char** args, char* callingLine, void* extraData){
	if(argC == 1){
			if(openFileVerificator(((Operation*)extraData)->dictionary,args[0])){

				uint32_t logicDirToClose = *((uint32_t*)dictionary_get(((Operation*)extraData)->dictionary,args[0]));

				SerializedPart fieldForFM91 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
				SerializedPart fieldForFM92 = {.size = 4, .data =&logicDirToClose};


				void* packetToFM9 = Serialization_Serialize(2, fieldForFM91, fieldForFM92);

				SocketCommons_SendData(((Operation*)extraData)->socketFM9,MESSAGETYPE_CPU_CLOSE, packetToFM9, sizeof(packetToFM9));

				int messageType, err, msglength;

				void* msgFromFM9 = SocketCommons_ReceiveData(((Operation*)extraData)->socketFM9,&messageType,&msglength,&err);

				DeserializedData data;
				Serialization_Deserialize(msgFromFM9,&data);
				if(*(int*)data.parts[0] == 1){
					((Operation*)extraData)->commandResult = 2; //SE DESALOJA SIEMPRE QUE HAYA CLOSE
				}
				else {
						Logger_Log(LOG_INFO, "Error fallo de segmento/memoria en FM9");
						SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};


						void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);
						SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));
				}

		}
			else {
				SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};


				void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);
				SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));
				((Operation*)extraData)->commandResult = 2; // 0 SALIO to MAL

			}

			SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};


			void* packetToSAFA = Serialization_Serialize(1, fieldForSAFA1);
			SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT, packetToSAFA, sizeof(packetToSAFA));
			((Operation*)extraData)->commandResult = 2; // 0 SALIO to MAL

		}
	StringUtils_FreeArray(args);

	return 0;
}

void* CommandCrear(int argC, char** args, char* callingLine, void* extraData){
	SerializedPart fieldForDAM1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
	SerializedPart fieldForDAM2 = {.size = sizeof(args[0]), .data = &args[0]};
	SerializedPart fieldForDAM3 = {.size = sizeof(args[0]), .data = &args[1]};

	void* packetToDAM = Serialization_Serialize(3, fieldForDAM1, fieldForDAM2, fieldForDAM3);

	SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_CREAR, packetToDAM, sizeof(packetToDAM));

	SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
	SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = &(((Operation*)extraData)->programCounter)};
	SerializedPart fieldForSAFA3 = {.size = 4, .data = &(((Operation*)extraData)->quantum)};
	uint32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
	SerializedPart fieldForSAFA4 = {.size = 4 , .data = &numberOfFiles};
    SerializedPart fieldForSAFA5 = {.size = sizeof(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
	void* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

	SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT, sizeof(packetToSAFAToBlockGDT));
    StringUtils_FreeArray(args);

	((Operation*)extraData)->commandResult = 2; //el 2 significa hacer un break

return 0;
}

void* CommandBorrar(int argC, char** args, char* callingLine, void* extraData){
	SerializedPart fieldForDAM1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
	SerializedPart fieldForDAM2 = {.size = sizeof(args[0]), .data = &args[0]};

	void* packetToDAM = Serialization_Serialize(2, fieldForDAM1, fieldForDAM2);

	SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_BORRAR, packetToDAM, sizeof(packetToDAM));

	SerializedPart fieldForSAFA1 = {.size = 4, .data =&(((Operation*)extraData)->dtb)};
	SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)extraData)->programCounter), .data = &(((Operation*)extraData)->programCounter)};
	SerializedPart fieldForSAFA3 = {.size = 4, .data = &(((Operation*)extraData)->quantum)};
	uint32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
	SerializedPart fieldForSAFA4 = {.size = 4 , .data = &numberOfFiles};
    SerializedPart fieldForSAFA5 = {.size = sizeof(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
	void* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

	SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT, sizeof(packetToSAFAToBlockGDT));
    StringUtils_FreeArray(args);

	((Operation*)extraData)->commandResult = 2; //el 2 significa hacer un break

return 0;
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
bool openFileVerificator(t_dictionary* dictionary,char* path){
	if(dictionary_get(dictionary,path) != NULL){
		return true;
	}
	return false;
}
void* FlattenPathsAndAddresses(t_dictionary* openFilesTable)
{

	void* result = malloc(1);
	int offset = 0, totalSize = 0;
	int nextSize;

	printf("Reserve el espacio\n");

	void CopyPath(char* path, void* address)
	{
		nextSize = strlen(path);							//Obtengo el largo del path
		totalSize += (nextSize + 2 + sizeof(int));			//Sumo a totalSize, y sumo 2 mas por los : y la ,
		result = realloc(result, totalSize);				//Realloco memoria
		memcpy(result + offset, path, nextSize);			//Copio el path y muevo el offset
		offset += nextSize;
		memcpy(result + offset, ":", 1);					//Copio el : (separa path de DL) y muevo el offset
		offset++;
		memcpy(result + offset, address, sizeof(int));		//Copio la DL y muevo el offset
		offset += sizeof(int);
		memcpy(result + offset, ",", 1);					//Copio la , (separa registros) y muevo el offset
		offset++;
	}

	dictionary_iterator(openFilesTable, CopyPath);			//Llamo al closure de arriba para hacerlo con todos los registros

	memcpy(result + offset - 1, ";", 1);					//Pongo el ; al final de la cadena

	return result;											//Queda : "arch1:d1,arch2:d2,...,archN:dN;"

}

