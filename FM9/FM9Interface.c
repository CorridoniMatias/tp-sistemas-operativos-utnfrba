#include "headers/FM9Interface.h"

//Asignar: idDTB, Direccion Logica, Datos. Codigo Operacion : 41

void FM9_AsignLine(void* data) {

	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));

	if (actualData->count == 3) {
		int dtbID = *((int *) actualData->parts[0]);
		int virtualAddress = *((int *) actualData->parts[1]);
		void* data = actualData->parts[2];
		int lineNumber = memoryFunctions->virtualAddressTranslation(
				virtualAddress, dtbID);
		if (lineNumber < 0) {
			*status = 2;
		}
		char* buffer = malloc(tamanioLinea);
		int result = readLine(buffer, lineNumber);
		if (result == INVALID_LINE_NUMBER) {
			*status = 2;
		} else {
			int size = sizeOfLine(buffer);
			int sizeOfData = string_length((char*) data);
			if (size + sizeOfData >= tamanioLinea)
				*status = 3;
			else {
				memcpy(buffer + size, data, sizeOfData);
				buffer[tamanioLinea - 1] = '\n';
				result = writeLine(buffer, lineNumber);
				if (result == INVALID_LINE_NUMBER) {
					*status = 2;
				} else {
					*status = 1;
				}
			}
		}
		free(buffer);

	} else {
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
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));
	bool error = false;
	char* buffer;
	bool freeBuffer = false;

	if (actualData->count == 2) {
		int dtbID = *((int *) actualData->parts[0]);
		int virtualAddress = *((int *) actualData->parts[1]);

		printf("\n\n\nid = %d\n\n\n", dtbID);
		printf("\n\n\nvirtualAddress = %d\n\n\n", virtualAddress);
		int lineNumber = memoryFunctions->virtualAddressTranslation(
				virtualAddress, dtbID);
		printf("\n\n\nlineNumber = %d\n\n\n", lineNumber);
		if (lineNumber >= 0) {
			buffer = malloc(tamanioLinea);
			freeBuffer = true;
			int result = readLine(buffer, lineNumber);
			if (result == INVALID_LINE_NUMBER) {
				*status = 2;
				error = true;
			} else
				*status = 1;
		} else {
			*status = 2;
			error = true;
		}
	} else {
		*status = 400;
		error = true;
	}
	SerializedPart* packetToCPU;
	SerializedPart code;
	char* newLine = "\n";

	printf("\n\n\nrompio antes de mandar mensaje\n\n\n");
	code.size = sizeof(int32_t);
	code.data = status;
	if (!error) {

		printf("\n\n\nSE METIO DONDE NO HAY ERROR\n\n\n");
		char* line = strtok(buffer, newLine);
		int sizeLine = strlen(line) + 1;


		SerializedPart content = { .size = sizeLine, .data = line };
		packetToCPU = Serialization_Serialize(2, code, content);

	} else {
		packetToCPU = Serialization_Serialize(1, code);
	}
	SocketCommons_SendData(arriveData->calling_SocketID,
	MESSAGETYPE_CPU_RECEIVELINE, packetToCPU->data, packetToCPU->size);

	printf("\n\n\nrompio despueeeeeeees de mandar mensaje\n\n\n");
	if (freeBuffer)
		free(buffer);
	SocketServer_CleanOnArrivedData(arriveData);
	Serialization_CleanupDeserializationStruct(actualData);
	free(status);
	Serialization_CleanupSerializedPacket(packetToCPU);
}

void FM9_Close(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));
	if (actualData->count == 2) {
		int dtbID = *((int *) actualData->parts[0]);
		int virtualAddress = *((int *) actualData->parts[1]);
		int result = memoryFunctions->closeFile(dtbID, virtualAddress);
		if (result == -1)
			*status = 2;
		else
			*status = 1;
	} else
		*status = 400;
	SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
			status, sizeof(uint32_t));
	SocketServer_CleanOnArrivedData(arriveData);
	Serialization_CleanupDeserializationStruct(actualData);
	free(status);
}
//Recibir un pedazo por pedazo hace un wakemeupwhen y un if messagelength = 0 break;
void FM9_Open(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	uint32_t* response_code = malloc(sizeof(uint32_t));
	int socket = arriveData->calling_SocketID;

	if (actualData->count == 3) {
		int dtbID = *((int *) actualData->parts[0]);
		int size = 0, sizeReceived = 0;
		void* buffer = malloc(1);

		while (1) {

			sizeReceived = *((int *) actualData->parts[1]);

			buffer = realloc(buffer, size + sizeReceived);
			printf("size received es = %d\n", sizeReceived);
			memcpy(buffer + size, actualData->parts[2], sizeReceived);
			size += sizeReceived;
			*response_code = 1;

			SocketCommons_SendData(arriveData->calling_SocketID,
			MESSAGETYPE_INT, response_code, sizeof(uint32_t));

			SocketServer_CleanOnArrivedData(arriveData);
			Serialization_CleanupDeserializationStruct(actualData);

			arriveData = SocketServer_WakeMeUpWhenDataIsAvailableOn(socket);
			printf("size = %d\n", size);
			printf("buffer= %s\n", (char*) buffer);
			if (arriveData->receivedDataLength == 0) //receivedDataLength es NULL por definicion de las kemmens, si llegamos a length 0 es porque no hay mas nada para recibir, tenemos el archivo completo.
				break;

			actualData = Serialization_Deserialize(arriveData->receivedData);

		}
		printf("size final es = %d\n", size);
		if (size == 0) {
			printf("se va a liberar esta poronga");
//			free(buffer);
		}

		printf("\npor hacer malloc\n");
		//Aca se copia en un nuevo buffer los datos hasta el \n pero haciendo que ocupen una linea entera, habiendo datos basura.
		void* realData = NULL;
		int sizeLine = 0, realSize = 0, offset = 0;
		printf("\npor acomodar el buffer\n");
		while (offset < size) {
			sizeLine = sizeOfLine((char*) (buffer + offset)) + 1;
			realData = realloc(realData, realSize + tamanioLinea);
			memcpy(realData + realSize, buffer + offset, sizeLine);
			realSize += tamanioLinea;
			offset += sizeLine;
		}
		free(buffer);
		int logicalAddress = memoryFunctions->writeData(realData, realSize,
				dtbID);

		if (logicalAddress == INSUFFICIENT_SPACE) {
			*response_code = 2;
			SocketCommons_SendData(socket, MESSAGETYPE_INT, response_code,
					sizeof(uint32_t));
		} else {
			declare_and_init(address, uint32_t, logicalAddress)
			printf("\n enviando direccion lógica =%d\n", logicalAddress);
			SocketCommons_SendData(socket, MESSAGETYPE_ADDRESS, address,
					sizeof(uint32_t));
			free(address);
		}

		free(realData);
	} else {
		*response_code = 400;
		SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
				response_code, sizeof(uint32_t));
	}
	SocketServer_CleanOnArrivedData(arriveData);
}

void FM9_Flush(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	if (actualData->count == 3) {
		uint32_t id = *((int*) actualData->parts[0]);
		uint32_t logicalAddress = *((int*) actualData->parts[1]);
		uint32_t transferSize = *((int*) actualData->parts[2]);
		void* buffer = NULL;
		int bufferSize = memoryFunctions->readData(buffer, id, logicalAddress);

		if (bufferSize <= 0) {
			free(buffer);
			declare_and_init(response_code, uint32_t, 2)
			SocketCommons_SendData(arriveData->calling_SocketID,
			MESSAGETYPE_INT, response_code, sizeof(uint32_t));
			free(response_code);
			SocketServer_CleanOnArrivedData(arriveData);
			Serialization_CleanupDeserializationStruct(actualData);
			return;
		}

		//Aca se copia en un nuevo buffer los datos hasta el \n, de forma que no hayan datos basura.
		void* realData = malloc(1);
		int sizeLine, realSize = 0, offset = 0;
		while (offset < bufferSize) {
			sizeLine = sizeOfLine((char*) (buffer + offset)) + 1;
			realData = realloc(realData, realSize + sizeLine);
			memcpy(realData + realSize, buffer + offset, sizeLine);
			realSize += sizeLine;
			offset += tamanioLinea;
		}
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
			SocketCommons_SendData(arriveData->calling_SocketID,
					MESSAGETYPE_STRING, buffer + offset, size);
			if (size == 0)
				break;
			offset += size;
			realSize -= size;
			OnArrivedData* wakeData =
					SocketServer_WakeMeUpWhenDataIsAvailableOn(
							arriveData->calling_SocketID);
			if (wakeData != NULL) {
				SocketServer_CleanOnArrivedData(wakeData);
			}

		}
	} else {
		declare_and_init(response_code, uint32_t, 400)
		SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
				response_code, sizeof(uint32_t));
		free(response_code);
		SocketServer_CleanOnArrivedData(arriveData);
		Serialization_CleanupDeserializationStruct(actualData);
		return;
	}
	SocketServer_CleanOnArrivedData(arriveData);
	Serialization_CleanupDeserializationStruct(actualData);
}

void FM9_Dump(int argC, char** args, char* callingLine, void* extraData) {
//switch (argC) {
//case 0:
//Logger_Log(LOG_INFO, "Tiene que indicar el id de un DTB.");
//break;
//case 1:
//Logger_Log(LOG_INFO, "DTBDID %s.",args[1]);
//memoryFunctions->dump(atoi(args[1]));
//break;
//default:
//Logger_Log(LOG_INFO, "Ingresó parámetros demas, solo debe ingresar el id del DTB");
//}
}

int sizeOfLine(char* line) {
	int i = 0;
	while (line[i] != '\n')
		i++;
	return i;
}
