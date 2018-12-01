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

void executeDummy(DeserializedData* dtb, int diego, int safa){
	uint32_t idDtb = *((uint32_t*)dtb->parts[0]);
	char* path = (char*)dtb->parts[2];
	declare_and_init(id, uint32_t,idDtb);
	SerializedPart fieldForDiego1 = {.size = sizeof(uint32_t), .data = id};
	SerializedPart fieldForDiego2 = {.size = strlen(path)+1, .data = path};

	SerializedPart* packetToDiego = Serialization_Serialize(2, fieldForDiego1,fieldForDiego2);

	SocketCommons_SendData(diego, MESSAGETYPE_VOIDPOINTER, packetToDiego->data, packetToDiego->size);




	SocketCommons_SendData(safa,MESSAGETYPE_CPU_BLOCKDUMMY, id, sizeof(uint32_t));

	Serialization_CleanupSerializedPacket(packetToDiego);
	Serialization_CleanupDeserializationStruct(dtb);
	free(id);

}

char* askLineToFM9(DeserializedData* dtb, int fm9){

	uint32_t idDtb = *((uint32_t*)dtb->parts[0]);
	int32_t logicDir = *((uint32_t*)dtb->parts[3]);
	uint32_t pc = *((uint32_t*)dtb->parts[4]);

	logicDir = logicDir + pc - 1;  //Sumar 1 a logicDir para pedir la siguiente linea

	declare_and_init(id, uint32_t,idDtb);
	declare_and_init(newLogicDir, uint32_t,logicDir);

	SerializedPart fieldForFM91 = {.size = 4, .data = id};
	SerializedPart fieldForFM92 = {.size = sizeof(int32_t), .data = newLogicDir};

	SerializedPart* packetToFM9 = Serialization_Serialize(2, fieldForFM91, fieldForFM92);

	SocketCommons_SendData(fm9,MESSAGETYPE_FM9_GETLINE, packetToFM9->data, packetToFM9->size);

	// Me quedo esperando hasta que me devuelva la linea para ejecutar
	int messageType, err, msglength;

	void* msgFromFM9 = SocketCommons_ReceiveData(fm9,&messageType,&msglength,&err);

	DeserializedData* data = Serialization_Deserialize(msgFromFM9);

	if(*(int*)data->parts[0] == 1){
		free(id);
		free(newLogicDir);
		Serialization_CleanupSerializedPacket(packetToFM9);
		Serialization_CleanupDeserializationStruct(dtb);
		char* line = (char*)data->parts[1];
		Serialization_CleanupDeserializationStruct(data);
		return line;
	}
		free(id);
		free(newLogicDir);
		Serialization_CleanupSerializedPacket(packetToFM9);
		Serialization_CleanupDeserializationStruct(dtb);
		Serialization_CleanupDeserializationStruct(data);
		Logger_Log(LOG_INFO, "Error fallo de segmento/memoria en FM9");
		return "error"; //
}

void* CommandAbrir(int argC, char** args, char* callingLine, void* extraData){
	if(openFileVerificator(((Operation*)extraData)->dictionary,args[0])){
		Logger_Log(LOG_INFO, "El archivo ya se encuentra abierto");
		return 0;
	}
	else{
		int32_t idDtb = ((Operation*)extraData)->dtb;
		declare_and_init(id, int32_t,idDtb);
		int32_t pc = ((Operation*)extraData)->programCounter;
		declare_and_init(newPc,int32_t,pc);
		int32_t quantum = ((Operation*)extraData)->quantum;
		declare_and_init(newQ,int32_t,quantum);
		int32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
		declare_and_init(newNumberOfFiles,int32_t,numberOfFiles);
		char* path = (char*)args[0];

		SerializedPart fieldForDAM1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForDAM2 = {.size = strlen(path)+1, .data = path};

		SerializedPart* packetToDAM = Serialization_Serialize(2, fieldForDAM1, fieldForDAM2);

		SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_ABRIR, packetToDAM->data, packetToDAM->size);

		SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newPc};
		SerializedPart fieldForSAFA3 = {.size = sizeof(int32_t), .data = newQ};
		SerializedPart fieldForSAFA4 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
	    SerializedPart fieldForSAFA5 = {.size = strlen(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
	    SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT->data, packetToSAFAToBlockGDT->size);
	    StringUtils_FreeArray(args);
		Serialization_CleanupSerializedPacket(packetToDAM);
		Serialization_CleanupSerializedPacket(packetToSAFAToBlockGDT);
		free(id);
		free(newPc);
		free(newQ);
		free(newNumberOfFiles);
		((Operation*)extraData)->commandResult = 2; //el 2 significa hacer un break

	}
return 0;
}

void* CommandConcentrar(int argC, char** args, char* callingLine, void* extraData){
	//sleep(settings->retardo);
	StringUtils_FreeArray(args);
	((Operation*)extraData)->commandResult = 2;

	return 0;

	}

void* CommandAsignar(int argC, char** args, char* callingLine, void* extraData){
	if(argC == 3){
		if (openFileVerificator(((Operation*) extraData)->dictionary, args[0])) {

			int32_t idDtb = ((Operation*) extraData)->dtb;
			declare_and_init(id, int32_t, idDtb);
			uint32_t logicDirToWrite = *((uint32_t*) dictionary_get(((Operation*) extraData)->dictionary, args[0]));
			uint32_t lineToWrite = atoi(args[1]);
			char* dataToWrite = args[2];
			logicDirToWrite = ((Operation*) extraData)->programCounter + lineToWrite - 1;
			declare_and_init(newLogicDir, int32_t, logicDirToWrite);

			SerializedPart fieldForFM91 = { .size = sizeof(int32_t), .data = id };
			SerializedPart fieldForFM92 = { .size = sizeof(uint32_t), .data = newLogicDir };
			SerializedPart fieldForFM93 = { .size = strlen(dataToWrite) + 1, .data = dataToWrite };

			SerializedPart* packetToFM9 = Serialization_Serialize(3, fieldForFM91, fieldForFM92, fieldForFM93);

			SocketCommons_SendData(((Operation*) extraData)->socketFM9, MESSAGETYPE_FM9_ASIGN, packetToFM9->data, packetToFM9->size);

			int messageType, err, msglength;
//
//			void* msgFromFM9 = SocketCommons_ReceiveData(((Operation*)extraData)->socketFM9,&messageType,&msglength,&err);
//			DeserializedData* data = Serialization_Deserialize(msgFromFM9);

			int responseFromFM9 = *((int*) SocketCommons_ReceiveData(((Operation*) extraData)->socketFM9, &messageType, &msglength, &err));
//			if(*(int*)data->parts[0] == 1){
			switch (responseFromFM9) {
				case 1:
					free(dataToWrite);
					free(id);
					free(newLogicDir);
					((Operation*) extraData)->commandResult = 0;
					StringUtils_FreeArray(args);
					Serialization_CleanupSerializedPacket(packetToFM9);
	//				Serialization_CleanupDeserializationStruct(data);
					return 0;  // 0 SALIO to BIEN
	//			}
	//			else {
	//				if(*(int*)data->parts[0] == 2){
				case 2:
					free(dataToWrite);
					Logger_Log(LOG_INFO, "Error fallo de segmento/memoria en FM9");
//					int32_t idDtb = ((Operation*) extraData)->dtb;
//					declare_and_init(id, int32_t, idDtb)

					SocketCommons_SendData(((Operation*) extraData)->socketSAFA,
							MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));
					free(id);
					free(newLogicDir);
					StringUtils_FreeArray(args);
	//					Serialization_CleanupDeserializationStruct(data);
					Serialization_CleanupSerializedPacket(packetToFM9);
					((Operation*) extraData)->commandResult = 2;
					return 0;
	//				}
	//				else {
				case 3:
					free(dataToWrite);
					Logger_Log(LOG_INFO, "Espacio insuficiente en FM9");
//					int32_t idDtb = ((Operation*) extraData)->dtb;
//					declare_and_init(id, int32_t, idDtb)

					SocketCommons_SendData(((Operation*) extraData)->socketSAFA,
							MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));

					free(id);
					free(newLogicDir);
					StringUtils_FreeArray(args);
	//					Serialization_CleanupDeserializationStruct(data);
					Serialization_CleanupSerializedPacket(packetToFM9);
					((Operation*) extraData)->commandResult = 2;
					return 0;
			}


	}
		else {
			Logger_Log(LOG_INFO, "El archivo no se encuentra abierto");
			int32_t idDtb = ((Operation*)extraData)->dtb;
			declare_and_init(id, int32_t,idDtb);


			SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT,id, sizeof(uint32_t));
			free(id);

			((Operation*)extraData)->commandResult = 2; // hacer break
			return 0;

		}
		int32_t idDtb = ((Operation*)extraData)->dtb;
		declare_and_init(id, int32_t,idDtb);
		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT,id, sizeof(uint32_t));

		StringUtils_FreeArray(args);
		free(id);
		((Operation*)extraData)->commandResult = 2; // hacer break

	}
return 0;
}

void* CommandWait(int argC, char** args, char* callingLine, void* extraData){

	if(argC == 1){
		int32_t idDtb = ((Operation*)extraData)->dtb;
		declare_and_init(id, int32_t,idDtb);
		char* resource = args[0];

		SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForSAFA2 = {.size = strlen(args[0]) + 1 , .data = resource};

		SerializedPart* packetToSAFA = Serialization_Serialize(2, fieldForSAFA1, fieldForSAFA2);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_WAIT, packetToSAFA->data, packetToSAFA->size);

		int messageType,err,msglength;
		void* msgFromSafa = SocketCommons_ReceiveData(((Operation*)extraData)->socketSAFA,&messageType,&msglength,&err);

		DeserializedData* data = Serialization_Deserialize(msgFromSafa);

		//Siendo code = 1, pudo ser asignado y code = 0, no pudo ser asignado. Quedaria de la siguente forma
		if(*(int*)data->parts[0] == 1 ){
			StringUtils_FreeArray(args);
			free(id);
			Serialization_CleanupDeserializationStruct(data);
			Serialization_CleanupSerializedPacket(packetToSAFA);
			((Operation*)extraData)->commandResult = 0; // 0 SALIO to BIEN
			return 0;
		}
		else {
			int32_t idDtb = ((Operation*)extraData)->dtb;
			declare_and_init(id, int32_t,idDtb);
			int32_t pc = ((Operation*)extraData)->programCounter;
			declare_and_init(newPc,int32_t,pc);
			int32_t quantum = ((Operation*)extraData)->quantum;
			declare_and_init(newQ,int32_t,quantum);
			int32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
			declare_and_init(newNumberOfFiles,int32_t,numberOfFiles);

			SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
			SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newPc};
			SerializedPart fieldForSAFA3 = {.size = sizeof(int32_t), .data = newQ};
			SerializedPart fieldForSAFA4 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
			SerializedPart fieldForSAFA5 = {.size = strlen(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
			SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

			SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT->data, packetToSAFAToBlockGDT->size);
			StringUtils_FreeArray(args);
			Serialization_CleanupDeserializationStruct(data);
			Serialization_CleanupSerializedPacket(packetToSAFA);
			Serialization_CleanupSerializedPacket(packetToSAFAToBlockGDT);
			free(id);
			free(newPc);
			free(newQ);
			free(newNumberOfFiles);
			((Operation*)extraData)->commandResult = 2;
			return 0;
		}
	}
	else {
		int32_t idDtb = ((Operation*)extraData)->dtb;
		declare_and_init(id, int32_t,idDtb);
		int32_t pc = ((Operation*)extraData)->programCounter;
		declare_and_init(newPc,int32_t,pc);
		int32_t quantum = ((Operation*)extraData)->quantum;
		declare_and_init(newQ,int32_t,quantum);
		int32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
		declare_and_init(newNumberOfFiles,int32_t,numberOfFiles);

		SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newPc};
		SerializedPart fieldForSAFA3 = {.size = sizeof(int32_t), .data = newQ};
		SerializedPart fieldForSAFA4 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
		SerializedPart fieldForSAFA5 = {.size = strlen(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
		SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT->data, packetToSAFAToBlockGDT->size);
		StringUtils_FreeArray(args);
		Serialization_CleanupSerializedPacket(packetToSAFAToBlockGDT);
		free(id);
		free(newPc);
		free(newQ);
		free(newNumberOfFiles);
		((Operation*)extraData)->commandResult = 2;

		return 0;
	}

}

void* CommandSignal(int argC, char** args, char* callingLine, void* extraData){


	if(argC == 1){

		int32_t idDtb = ((Operation*)extraData)->dtb;
		declare_and_init(id, int32_t,idDtb);
		char* resource = args[0];

		SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForSAFA2 = {.size = strlen(args[0]) + 1 , .data = resource};

		SerializedPart* packetToSAFA = Serialization_Serialize(2, fieldForSAFA1, fieldForSAFA2);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_WAIT, packetToSAFA->data, packetToSAFA->size);


		int messageType,err,msglength;
		void* msgFromSafa = SocketCommons_ReceiveData(((Operation*)extraData)->socketSAFA,&messageType,&msglength,&err);
		DeserializedData* data = Serialization_Deserialize(msgFromSafa);

		if(*(int*)data->parts[0] == 1 ){
			Serialization_CleanupDeserializationStruct(data);
			Serialization_CleanupSerializedPacket(packetToSAFA);
			StringUtils_FreeArray(args);
			free(id);
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

		int32_t idDtb = ((Operation*)extraData)->dtb;
		declare_and_init(id, int32_t,idDtb);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_EOFORABORT,id, sizeof(uint32_t));

		((Operation*)extraData)->commandResult = 2;
	    StringUtils_FreeArray(args);
		Logger_Log(LOG_INFO, "El archivo no se encuentra abierto");
		return 0;
	}
	else{
		int32_t idDtb = ((Operation*)extraData)->dtb;
		declare_and_init(id, int32_t,idDtb);
		char* path = args[0];
		int32_t pc = ((Operation*)extraData)->programCounter;
		declare_and_init(newPc,int32_t,pc);
		int32_t quantum = ((Operation*)extraData)->quantum;
		declare_and_init(newQ,int32_t,quantum);
		int32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
		declare_and_init(newNumberOfFiles,int32_t,numberOfFiles);
		uint32_t virtualAddress = *((int*)dictionary_get(((Operation*)extraData)->dictionary,path));
		declare_and_init(newVirtualAddress,uint32_t,virtualAddress)

		SerializedPart fieldForDAM1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForDAM2 = {.size = sizeof(int32_t), .data =newVirtualAddress};
		SerializedPart fieldForDAM3 = {.size = strlen(path)+1, .data = path};

		SerializedPart* packetToDAM = Serialization_Serialize(3, fieldForDAM1, fieldForDAM2, fieldForDAM3);

		SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_FLUSH, packetToDAM->data, packetToDAM->size);

		SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newPc};
		SerializedPart fieldForSAFA3 = {.size = sizeof(int32_t), .data = newQ};
		SerializedPart fieldForSAFA4 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
	    SerializedPart fieldForSAFA5 = {.size = strlen(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
	    SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

		SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT->data, packetToSAFAToBlockGDT->size);
	    StringUtils_FreeArray(args);
		Serialization_CleanupSerializedPacket(packetToSAFAToBlockGDT);
		Serialization_CleanupSerializedPacket(packetToDAM);
		free(id);
		free(newPc);
		free(newQ);
		free(newNumberOfFiles);
		((Operation*)extraData)->commandResult = 2; //el 2 significa hacer un break

		//TODO me quedo esperando alguna respuesta o sigo con mi operatoria?
	}
return 0;
}

void* CommandClose(int argC, char** args, char* callingLine, void* extraData){
	if (argC == 1) {
		if (openFileVerificator(((Operation*) extraData)->dictionary, args[0])) {
			int32_t idDtb = ((Operation*) extraData)->dtb;
			declare_and_init(id, int32_t, idDtb);
			uint32_t logicDirToClose = *((uint32_t*) dictionary_get(((Operation*) extraData)->dictionary, args[0]));
			declare_and_init(newLogicDirToClose, uint32_t, logicDirToClose);

			SerializedPart fieldForFM91 ={ .size = sizeof(int32_t), .data = id };
			SerializedPart fieldForFM92 = { .size = sizeof(uint32_t), .data = newLogicDirToClose };

			SerializedPart* packetToFM9 = Serialization_Serialize(2, fieldForFM91, fieldForFM92);

			SocketCommons_SendData(((Operation*) extraData)->socketFM9, MESSAGETYPE_FM9_CLOSE, packetToFM9->data, packetToFM9->size);

			int messageType, err, msglength;
//				void* msgFromFM9 = SocketCommons_ReceiveData(((Operation*)extraData)->socketFM9,&messageType,&msglength,&err);
//				DeserializedData* data = Serialization_Deserialize(msgFromFM9);
			int responseFromFM9 = *((int*) SocketCommons_ReceiveData(((Operation*) extraData)->socketFM9, &messageType, &msglength, &err));
//				if(*(int*)data->parts[0] == 1){
			switch (responseFromFM9) {
				case 1:
					StringUtils_FreeArray(args);
	//				Serialization_CleanupDeserializationStruct(data);
					Serialization_CleanupSerializedPacket(packetToFM9);
					free(id);
					((Operation*) extraData)->commandResult = 2; //SE DESALOJA SIEMPRE QUE HAYA CLOSE
					return 0;

	//				else {
				case 2:
					Logger_Log(LOG_INFO, "Error fallo de segmento/memoria en FM9");
					int32_t idDtb = ((Operation*) extraData)->dtb;
					declare_and_init(id, int32_t, idDtb)

					SocketCommons_SendData(((Operation*) extraData)->socketSAFA,
					MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));
					StringUtils_FreeArray(args);
	//				Serialization_CleanupDeserializationStruct(data);
					Serialization_CleanupSerializedPacket(packetToFM9);

					free(id);
					return 0;
			}

		} else {
			int32_t idDtb = ((Operation*) extraData)->dtb;
			declare_and_init(id, int32_t, idDtb)
			SocketCommons_SendData(((Operation*) extraData)->socketSAFA, MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));
			((Operation*) extraData)->commandResult = 2; // 0 SALIO to MAL
			free(id);
			StringUtils_FreeArray(args);

			return 0;

		}
		int32_t idDtb = ((Operation*) extraData)->dtb;
		declare_and_init(id, int32_t, idDtb);
		SocketCommons_SendData(((Operation*) extraData)->socketSAFA,
				MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));
		((Operation*) extraData)->commandResult = 2; // 0 SALIO to MAL
		free(id);
		StringUtils_FreeArray(args);

		return 0;
	}

	return 0;
}

void* CommandCrear(int argC, char** args, char* callingLine, void* extraData){
	int32_t idDtb = ((Operation*)extraData)->dtb;
	declare_and_init(id, int32_t,idDtb);
	char* path = args[0];
	char* lines = args[1];
	int32_t pc = ((Operation*)extraData)->programCounter;
	declare_and_init(newPc,int32_t,pc);
	int32_t quantum = ((Operation*)extraData)->quantum;
	declare_and_init(newQ,int32_t,quantum);
	int32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
	declare_and_init(newNumberOfFiles,int32_t,numberOfFiles);

	SerializedPart fieldForDAM1 = {.size = sizeof(int32_t), .data =id};
	SerializedPart fieldForDAM2 = {.size = strlen(path)+1, .data = path};
	SerializedPart fieldForDAM3 = {.size = strlen(lines)+1, .data = lines};

	SerializedPart* packetToDAM = Serialization_Serialize(3, fieldForDAM1, fieldForDAM2, fieldForDAM3);

	SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_CREAR, packetToDAM->data, packetToDAM->size);

	SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
	SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newPc};
	SerializedPart fieldForSAFA3 = {.size = sizeof(int32_t), .data = newQ};
	SerializedPart fieldForSAFA4 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
    SerializedPart fieldForSAFA5 = {.size = strlen(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
    SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

	SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT->data, packetToSAFAToBlockGDT->size);
    StringUtils_FreeArray(args);
    free(id);
    free(newPc);
    free(newQ);
    free(newNumberOfFiles);
	Serialization_CleanupSerializedPacket(packetToSAFAToBlockGDT);
	Serialization_CleanupSerializedPacket(packetToDAM);

	((Operation*)extraData)->commandResult = 2; //el 2 significa hacer un break

return 0;
}

void* CommandBorrar(int argC, char** args, char* callingLine, void* extraData){
	int32_t idDtb = ((Operation*)extraData)->dtb;
	declare_and_init(id, int32_t,idDtb);
	char* path = args[0];
	int32_t pc = ((Operation*)extraData)->programCounter;
	declare_and_init(newPc,int32_t,pc);
	int32_t quantum = ((Operation*)extraData)->quantum;
	declare_and_init(newQ,int32_t,quantum);
	int32_t numberOfFiles = dictionary_size(((Operation*)extraData)->dictionary);
	declare_and_init(newNumberOfFiles,int32_t,numberOfFiles);

	SerializedPart fieldForDAM1 = {.size = sizeof(int32_t), .data =id};
	SerializedPart fieldForDAM2 = {.size = sizeof(args[0]), .data = path};

	SerializedPart* packetToDAM = Serialization_Serialize(2, fieldForDAM1, fieldForDAM2);

	SocketCommons_SendData(((Operation*)extraData)->socketDIEGO,MESSAGETYPE_CPU_BORRAR, packetToDAM->data, packetToDAM->size);

	SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
	SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newPc};
	SerializedPart fieldForSAFA3 = {.size = sizeof(int32_t), .data = newQ};
	SerializedPart fieldForSAFA4 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
    SerializedPart fieldForSAFA5 = {.size = strlen(FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)) + 1 , .data = FlattenPathsAndAddresses(((Operation*)extraData)->dictionary)};
    SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(5, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5);

	SocketCommons_SendData(((Operation*)extraData)->socketSAFA,MESSAGETYPE_CPU_BLOCKDTB,packetToSAFAToBlockGDT->data, packetToSAFAToBlockGDT->size);
    StringUtils_FreeArray(args);
	Serialization_CleanupSerializedPacket(packetToSAFAToBlockGDT);
	Serialization_CleanupSerializedPacket(packetToDAM);
	free(id);
    free(newPc);
    free(newQ);
    free(newNumberOfFiles);

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
