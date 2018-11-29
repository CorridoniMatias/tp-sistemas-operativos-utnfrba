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
		if (lineNumber == ITS_A_TRAP) {
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

	} else {
		*status = 400;
	}
	SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
			status, sizeof(uint32_t));
}

//		int virtualAddress, int dtbID, void* data)

//
//	char* buffer = malloc(tamanioLinea);
//	int result = readLine(buffer, lineNumber);
//	int size = sizeOfLine(buffer);
//	int sizeOfData = string_length((char*) data);
//	if (size + sizeOfData >= tamanioLinea)
//		return INSUFFICIENT_SPACE;
//	memcpy(buffer + size, data, sizeOfData);
//	buffer[tamanioLinea - 1] = '\n';
//	return writeLine(buffer, lineNumber);

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
		int lineNumber = memoryFunctions->virtualAddressTranslation(
				virtualAddress, dtbID);

		if (lineNumber != -1) {
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
	char* newLine = '\n';
	if (!error) {
		char* line = strtok(buffer, newLine);
		int sizeLine = strlen(line) + 1;
		code.size = sizeof(int32_t);
		code.data = status;

		SerializedPart content = { .size = sizeLine, .data = line };
		packetToCPU = Serialization_Serialize(2, code, content);

	} else {
		packetToCPU = Serialization_Serialize(1, code);
	}
	SocketCommons_SendData(arriveData->calling_SocketID,
	MESSAGETYPE_CPU_RECEIVELINE, packetToCPU->data, packetToCPU->size);
	if (freeBuffer)
		free(buffer);
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
}
//Recibir un pedazo por pedazo hace un wakemeupwhen y un if messagelength = 0 break;
void FM9_Open(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));

	if (actualData->count == 2) {
		int dtbID = *((int *) actualData->parts[0]);
		void* buffer = actualData->parts[1];
		int bufferSize = arriveData->receivedDataLength - sizeof(uint32_t);
//		void* buffer = malloc(1);
		while (1) {
			int message_type, error_status, message_length;
			void* recvData = SocketCommons_ReceiveData(
					arriveData->calling_SocketID, &message_type,
					&message_length, &error_status);

			if (message_length == 0) //recvData es NULL por definicion de las kemmens, si llegamos a length 0 es porque no hay mas nada para recibir, tenemos el archivo completo.
				break;
			bufferSize += message_length;
			buffer = realloc(buffer, bufferSize);
			memcpy(buffer + (bufferSize - message_length), recvData,
					message_length);
			free(recvData);
			break;

		}
		if (bufferSize == 0)
			free(buffer);
	}

	else
		*status = 1;
//	else
//		*status = 400;

	SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
			status, sizeof(uint32_t));
//int dtbID, void* data, int size) {
//return memoryFunctions->writeData(data, size, dtbID);
}

void FM9_Flush(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(
			arriveData->receivedData);
	if (actualData->count == 3) {
		uint32_t id = *((int*) actualData->parts[0]);
		uint32_t logicalAddress = *((int*) actualData->parts[1]);
		uint32_t transferSize = *((int*) actualData->parts[2]);
		void* buffer;
		int bufferSize = memoryFunctions->readData(buffer, id, logicalAddress);

		if (bufferSize <= 0) {
			free(buffer);
			declare_and_init(response_code, uint32_t, 2)
			SocketCommons_SendData(arriveData->calling_SocketID,
			MESSAGETYPE_INT, response_code, sizeof(uint32_t));
			return;
		}

		//Aca se copia en un nuevo buffer los datos hasta el \n, de forma que no hayan datos basura.
		void* realData = malloc(1);
		char* newLine = '\n';
		int sizeLine, realSize = 0, offset = 0;
		while (offset < bufferSize) {
			sizeLine = sizeOfLine(buffer + offset) + 1;
			realloc(realData, realSize + sizeLine);
			memcpy(realData + realSize, buffer + offset, sizeLine);
			realSize += sizeLine;
			offset += tamanioLinea;
		}
		free(buffer);
		//Falta mandar las cosas de a transfersizes
		//TODO Hay que hacer un wakemeup
		offset = 0;
		int size = 0;
		while (1) {
			if (realSize < 0)
				size = 0;
			if (realSize < transferSize)
				size = realSize;
			else
				size = transferSize;
			SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_STRING, buffer + offset, size);
			offset += size;
			realSize -= size;

		}
	} else {
		declare_and_init(response_code, uint32_t, 400)
		SocketCommons_SendData(arriveData->calling_SocketID, MESSAGETYPE_INT,
				response_code, sizeof(uint32_t));
	}
	return;
}

}

void FM9_Dump(void* data) {
//int argC, char** args, char* callingLine, void* extraData) {
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
