#include "headers/FM9Interface.h"

//Asignar: idDTB, Direccion Logica, Datos. Codigo Operacion : 41

void FM9_AsignLine(void* data) {

	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));
	if (actualData->count == 3) {
		int dtbID = *((int *) actualData->parts[0]);
		int logicalAddress = *((int *) actualData->parts[1]);
		char* line = actualData->parts[2];

//		printf("\n\n\nasignar linea id dtb %d\n\n\n",dtbID);
//		printf("\nnumero de linea recibida = %d.\n", logicalAddress);
//		printf("\nlinea recibida = %s.\n", line);

		char* buffer = calloc(1,tamanioLinea + 1);
		memset(buffer,0,tamanioLinea + 1);
		int result;
//		int result = readLine(buffer, lineNumber);
//		if (result == INVALID_LINE_NUMBER) {
//			*status = 2;
//		} else {
//		int size = sizeOfLine(buffer, tamanioLinea);
		int sizeOfData = string_length(line);
//		if (size + sizeOfData >= tamanioLinea)
		if (sizeOfData >= tamanioLinea){
			Logger_Log(LOG_ERROR, "FM9::Asignar -> Espacio insuficiente!");
			*status = 3;
		}
		else {
			Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", logicalAddress);
			int lineNumber = memoryFunctions->logicalAddressTranslation(logicalAddress, dtbID);
			if (lineNumber < 0) {
				Logger_Log(LOG_ERROR, "FM9::Asignar -> Número de linea invalido!");
				*status = 2;
			} else {
				memcpy(buffer, line, sizeOfData);
//			memcpy(buffer + size, line, sizeOfData);
				buffer[sizeOfData] = '\n';
//				printf("\nlinea a ser escritaaaa = \"%s\".\n", buffer);
				result = writeLine(buffer, lineNumber);
				if (result == INVALID_LINE_NUMBER) {
					Logger_Log(LOG_ERROR, "FM9::Asignar -> Número de linea invalido!");
					*status = 2;
				} else {
					Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", lineNumber);
					buffer[sizeOfData] = 0;
					Logger_Log(LOG_INFO, "FM9::Asignar -> Se asigno \"%s\" a la linea %d.", buffer, lineNumber);
					*status = 1;
				}
			}
		}
		free(buffer);

	} else {
		Logger_Log(LOG_ERROR, "FM9::Asignar -> Cantidad de argumentos insuficiente!");
		*status = 400;
	}
	SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
			status, sizeof(uint32_t));

	SocketServer_CleanOnArrivedData(arriveData);
	Serialization_CleanupDeserializationStruct(actualData);
	free(status);

}

void FM9_AskForLine(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));
	bool error = false;
	char* buffer = calloc(1,tamanioLinea);
	int lineNumber = 0;
	memset(buffer,0,tamanioLinea);
	if (actualData->count == 2) {
		int dtbID = *((int *) actualData->parts[0]);
		int logicalAddress = *((int *) actualData->parts[1]);

//		printf("\n\n\nid = %d\n\n\n", dtbID);
//		printf("\n\n\nvirtualAddress = %d\n\n\n", logicalAddress);
		Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", logicalAddress);
		lineNumber = memoryFunctions->logicalAddressTranslation(logicalAddress, dtbID);
//		printf("\n\n\nlineNumber = %d\n\n\n", lineNumber);
		if (lineNumber >= 0) {
			int result = readLine(buffer, lineNumber);
			if (result == INVALID_LINE_NUMBER) {
				Logger_Log(LOG_ERROR, "FM9::Pedir linea -> Fallo de memoria! Número de linea invalido!");
				*status = 2;
				error = true;
			} else{
				Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", lineNumber);
				*status = 1;
			}
		} else {
			Logger_Log(LOG_ERROR, "FM9::Pedir linea -> Número de linea invalido!");
			*status = 2;
			error = true;
		}
	} else {
		Logger_Log(LOG_ERROR, "FM9::Pedir linea -> Cantidad de argumentos insuficiente!");
		*status = 400;
		error = true;
	}
	SerializedPart* packetToCPU;
	SerializedPart code;
//	char* newLine = "\n";
	code.size = sizeof(int32_t);
	code.data = status;
	if (!error) {
//		printf("\n\n\nnewLine=\"%s\"",newLine);
//		printf("\n\n\nbuffer=\"%s\"",buffer);
//		char* line = strtok(buffer, newLine);
//		int sizeLine = strlen(line) + 1;
		int sizeLine = sizeOfLine(buffer, tamanioLinea);
		buffer[sizeLine] = 0;
		SerializedPart content = { .size = sizeLine + 1, .data = buffer};
		packetToCPU = Serialization_Serialize(2, code, content);
		Logger_Log(LOG_INFO, "FM9::Pedir linea -> Linea %d que contenia \"%s\" devuelta correctamente!.", lineNumber, buffer);

	} else {
		packetToCPU = Serialization_Serialize(1, code);
	}
	SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_CPU_RECEIVELINE, packetToCPU->data, packetToCPU->size);
	free(buffer);
	SocketServer_CleanOnArrivedData(arriveData);
	Serialization_CleanupDeserializationStruct(actualData);
	free(status);
	Serialization_CleanupSerializedPacket(packetToCPU);
}

void FM9_Close(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));
	if (actualData->count == 2) {
		uint32_t dtbID = *((uint32_t *) actualData->parts[0]);
		uint32_t virtualAddress = *((uint32_t *) actualData->parts[1]);
		Logger_Log(LOG_INFO, "FM9::Cerrar -> Realizando close del archivo del G.DT de id %d con dirección lógica %d", dtbID, virtualAddress);
		int result = memoryFunctions->closeFile(dtbID, virtualAddress);
		if (result <= 0) {
			Logger_Log(LOG_ERROR, "FM9::Cerrar -> Fallo de memoria! No pudo cerrarse el archivo");
			*status = 2;
		} else {
			Logger_Log(LOG_INFO, "FM9::Cerrar -> Pudo cerrarse correctamente el archivo!");
			*status = 1;
		}
	} else {
		Logger_Log(LOG_ERROR, "FM9::Cerrar -> Cantidad de argumentos insuficiente!");
		*status = 400;
	}
	SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT, status, sizeof(uint32_t));
	SocketServer_CleanOnArrivedData(arriveData);
	Serialization_CleanupDeserializationStruct(actualData);
	free(status);
}
//Recibir un pedazo por pedazo hace un wakemeupwhen y un if messagelength = 0 break;
void FM9_Open(void* data) {
//	printf("\n\n\n\n\nEjecutando FM9 OPEN\n\n");
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	uint32_t* response_code = malloc(sizeof(uint32_t));
	int socket = arriveData->calling_SocketID;

	if (actualData->count == 3) {
		int dtbID = *((uint32_t *) actualData->parts[0]);

//		printf("\niddd = %d\n", dtbID);
		int size = 0, sizeReceived = 0;
		void* buffer = malloc(1);
		*response_code = 1;
//		printf("\nsocket = %d\n", socket);

		while (1) {

			sizeReceived = *((uint32_t *) actualData->parts[1]);

			buffer = realloc(buffer, size + sizeReceived);

			memset(buffer+size,0,sizeReceived);
//			printf("size received es = %d\n", sizeReceived);
			memcpy(buffer + size, actualData->parts[2], sizeReceived);
			size += sizeReceived;

			SocketServer_CleanOnArrivedData(arriveData);
			Serialization_CleanupDeserializationStruct(actualData);

			bool found = SocketServer_ReserveSocket(socket);
			if (!found) {
				return;
			}
			SocketCommons_SendData(socket, MESSAGETYPE_INT, response_code, sizeof(uint32_t));
			arriveData = SocketServer_WakeMeUpWhenDataIsAvailableOn(socket);
//			printf("\nsize = %d\n", size);
//			printf("\nbuffer= %s\n", (char*) buffer);


//			printf("\n\n\nwakemeuppppp\n\n\n");

			if (arriveData->receivedDataLength == 0) //receivedDataLength es NULL por definicion de las kemmens, si llegamos a length 0 es porque no hay mas nada para recibir, tenemos el archivo completo.
				break;

			actualData = Serialization_Deserialize(arriveData->receivedData);

		}
//		printf("\n\nsize final es = %d\n\n", size);
//		printf("\n\nbuffer final=%s\n\n",(char*)buffer);
		buffer = realloc(buffer, size + 1);
		((char*)buffer)[size] = 0;
		Logger_Log(LOG_INFO, "FM9::Abrir -> Tamaño real del archivo son %d bytes. El contenido es:\n%s", size, buffer);
		if (size == 0) {
//			printf("se va a liberar esta poronga");
//			free(buffer);
		}
		//Aca se copia en un nuevo buffer los datos hasta el \n pero haciendo que ocupen una linea entera, habiendo datos basura.
		void* realData = NULL;
		int sizeLine = 0, realSize = 0, offset = 0;
//		printf("\npor acomodar el buffer\n");
		while (offset < size) {
//			printf("\n\noffset=%d\n\n",offset);
			sizeLine = sizeOfLine((char*) (buffer + offset), size-offset) + 1;
			realData = realloc(realData, realSize + tamanioLinea);
			memcpy(realData + realSize, buffer + offset, sizeLine);
			realSize += tamanioLinea;
			offset += sizeLine;
		}
		free(buffer);
		int logicalAddress = memoryFunctions->writeData(realData, realSize, dtbID);

		if (logicalAddress == INSUFFICIENT_SPACE) {
			Logger_Log(LOG_ERROR, "FM9::Abrir -> Espacio insuficiente en FM9! No se pudo abrir el archivo para el G.DT de id %d",dtbID);
			*response_code = 2;
			SocketCommons_SendData(socket, MESSAGETYPE_INT, response_code, sizeof(uint32_t));
		} else if (logicalAddress == ITS_A_TRAP) {
			Logger_Log(LOG_ERROR, "FM9::Abrir -> Fallo de memoria! No se pudo abrir el archivo para el G.DT de id %d",dtbID);
			*response_code = 500;
			SocketCommons_SendData(socket, MESSAGETYPE_INT, response_code, sizeof(uint32_t));
		} else {
			Logger_Log(LOG_INFO, "FM9::Abrir -> Se abrio de manera exitosa el archivo para el G.DT de id %d en la dirección lógica %d",dtbID, logicalAddress);
			realData = realloc(realData, realSize + 1);
			((char*)realData)[realSize] = 0;
//			Logger_Log(LOG_INFO, "FM9::Abrir -> Tamaño del archivo en memoria son %d bytes. El contenido es:\n%s",realSize, realData);
			Logger_Log(LOG_INFO, "FM9::Abrir -> Tamaño del archivo en memoria son %d bytes.",realSize);
			declare_and_init(address, uint32_t, logicalAddress)
//			printf("\n enviando direccion lógica =%d\n", logicalAddress);
			SocketCommons_SendData(socket, MESSAGETYPE_ADDRESS, address, sizeof(uint32_t));
			free(address);
		}

		free(realData);
	} else {
		Logger_Log(LOG_ERROR, "FM9::Abrir -> Cantidad de argumentos insuficiente!");
		*response_code = 400;
		SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
				response_code, sizeof(uint32_t));
	}
	free(response_code);
	SocketServer_CleanOnArrivedData(arriveData);
}

void FM9_Flush(void* data) {
	OnArrivedData* arriveData = data;
//	printf("\n\n\n\n\nEjecutando FM9 FLUSHHH\n\n");
	DeserializedData* actualData;
	int socket = arriveData->calling_SocketID;
	if (arriveData->receivedData == NULL) {
//		printf("\n\n\n\nhay un null en el receivedData que onda?\n\n\n\n");
		return;
	}
	actualData = Serialization_Deserialize(arriveData->receivedData);
	if (actualData->count == 3) {
		uint32_t id = *((int*) actualData->parts[0]);
		uint32_t logicalAddress = *((int*) actualData->parts[1]);
		uint32_t transferSize = *((int*) actualData->parts[2]);
//		printf("\n\nid=%d\n\n",id);
//		printf("\n\nlogicalAddress=%d\n\n",logicalAddress);
		void* buffer = NULL;
		int bufferSize = memoryFunctions->readData(&buffer, logicalAddress, id);

		if (bufferSize == ITS_A_TRAP) {
			Logger_Log(LOG_ERROR, "FM9::Flush -> Fallo de memoria!");
			free(buffer);
			declare_and_init(response_code, uint32_t, 2)
//			printf("\n\npor enviar errorrrrrrrrrrrrrr\n\n\n\n\n\n\n");
			SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT, response_code, sizeof(uint32_t));
			free(response_code);
			SocketServer_CleanOnArrivedData(arriveData);
			Serialization_CleanupDeserializationStruct(actualData);
			return;
		}
		buffer = realloc(buffer, bufferSize + 1);
		((char*)buffer)[bufferSize] = 0;
//		Logger_Log(LOG_INFO, "FM9::Flush -> Tamaño del archivo en memoria son %d bytes. El contenido es:\n%s",bufferSize, buffer);
		Logger_Log(LOG_INFO, "FM9::Flush -> Tamaño del archivo en memoria son %d bytes.",bufferSize);
		//Aca se copia en un nuevo buffer los datos hasta el \n, de forma que no hayan datos basura.
		void* realData = malloc(1);
		int sizeLine, realSize = 0, offset = 0;
		while (offset < bufferSize) {
			if(buffer==NULL){
//				printf("\n\n\n\nporque poronga buffer es null\n\n\n\n");
				break;
			}
//			printf("\n\nbuffer =\"%s\"\n\n",(char*)buffer);
//			printf("\n\nrealdata =\"%s\"\n\n",(char*)realData);
			sizeLine = sizeOfLine((char*)buffer + offset, bufferSize - offset) + 1;
			realData = realloc(realData, realSize + sizeLine);
			memcpy(realData + realSize, buffer + offset, sizeLine);
			realSize += sizeLine;
			offset += tamanioLinea;
		}
		Logger_Log(LOG_INFO, "FM9::Flush -> Tamaño real del archivo son %d bytes. El contenido es:\n%s", realSize, realData);
		free(buffer);
		//Falta mandar las cosas de a transfersizes
		offset = 0;
		int size = 0;
		while (1) {
			if (realSize < 0)
				size = 0;
			if (realSize < transferSize)
				size = realSize;
			else
				size = transferSize;

			SocketServer_CleanOnArrivedData(arriveData);
//			printf("\n\npor enviar ack\n\n");
			bool found = SocketServer_ReserveSocket(socket);
			if(!found){
				Logger_Log(LOG_ERROR, "FM9::Flush -> Fallo de memoria!");
				free(realData);
				return;
			}
//			printf("\ntamaño = %d\n",size);
			SocketCommons_SendData(socket, MESSAGETYPE_STRING, realData + offset, size);
			if (size == 0)
				break;
			char* linea = NULL;
			linea = calloc(1,size);
			memcpy(linea, realData + offset, size);
			linea = realloc(linea, size + 1);
			memcpy(linea + size, "\0", 1);
//			memcpy(linea + tamanioLinea, 0, 1);
//			printf("\ndatos enviados =\"%s\"\n",linea);
			free(linea);
			arriveData = SocketServer_WakeMeUpWhenDataIsAvailableOn(socket);
			offset += size;
			realSize -= size;

		}
		Logger_Log(LOG_INFO, "FM9::Flush -> Se flusheo correctamente el archivo con dirección lógica %d del GDT %d", logicalAddress, id);
		free(realData);
	} else {
		Logger_Log(LOG_ERROR, "FM9::Flush -> Cantidad de argumentos insuficiente!");
		declare_and_init(response_code, uint32_t, 400)
		SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT, response_code, sizeof(uint32_t));
		free(response_code);
		SocketServer_CleanOnArrivedData(arriveData);
		Serialization_CleanupDeserializationStruct(actualData);
		return;
	}
	Serialization_CleanupDeserializationStruct(actualData);
}

void FM9_Dump(int argC, char** args, char* callingLine, void* extraData) {
	switch (argC) {
		case 0:
			Logger_Log(LOG_ERROR, "FM9::Dump -> Tiene que indicar el id de un DTB.");
			break;
		case 1:
			Logger_Log(LOG_INFO, "FM9::Dump -> GDT id %s.", args[1]);
			if(memoryFunctions->dump(atoi(args[1])) == ITS_A_TRAP){
				Logger_Log(LOG_INFO, "FM9::Dump -> No habían archivos para hacer dump!");
			}
			break;
		default:
			Logger_Log(LOG_ERROR,"FM9::Dump -> Ingresó parámetros demas, solo debe ingresar el id del DTB");
	}
}


void FM9_CloseDTB(void* data){
	OnArrivedData* arrivedData = data;
	uint32_t dtbID = *((uint32_t*)arrivedData->receivedData);
	int result = memoryFunctions->closeDTBFiles(dtbID);
	if(result == ITS_A_TRAP)
		Logger_Log(LOG_ERROR, "FM9::Cerrar archivos -> No se pudieron cerrar los archivos del G.DT de id %d",dtbID);
	else
		Logger_Log(LOG_INFO, "FM9::Cerrar archivos -> Se cerraron correctamente los archivos del G.DT de id %d",dtbID);
	SocketServer_CleanOnArrivedData(arrivedData);
}

int sizeOfLine(char* line, int maxSize) {
	int i = 0;
//	printf("\n\nanalizando linea=\"%s\"\n\n", line);
	while (line[i] != '\n') {
//		printf(" i=%d ", i);
		i++;
		if (i >= maxSize)
			return 0;
	}
	return i;
}
