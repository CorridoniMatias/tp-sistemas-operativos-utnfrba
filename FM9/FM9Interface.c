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

int sizeOfLine(char* line) {
	int len = 0;
	while (line[len] != '\n')
		len++;
	return len;
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
	if (!error) {
		char* line = strtok(buffer, '\n');
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

void FM9_Open(void* data) {
	OnArrivedData* arriveData = data;
	DeserializedData* actualData = Serialization_Deserialize(arriveData->receivedData);
	uint32_t* status = malloc(sizeof(uint32_t));
	if (actualData->count == 3) {
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
//int dtbID, void* data, int size) {
//return memoryFunctions->writeData(data, size, dtbID);
}

void FM9_Flush(void* data) {


//int dtbID, int virtualAddress) {

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


