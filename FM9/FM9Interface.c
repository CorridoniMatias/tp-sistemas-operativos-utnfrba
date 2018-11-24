#include "headers/FM9Interface.h"

int FM9_AsignLine(int virtualAddress, int dtbID, void* data) {
	int lineNumber = memoryFunctions->virtualAddressTranslation(virtualAddress,
			dtbID);
	if (lineNumber == -1) {
		return -1;
	}
	char* buffer = malloc(tamanioLinea);
	int result = readLine(buffer, lineNumber);
	int size = sizeOfLine(buffer);
	int sizeOfData = string_length((char*) data);
	if (size + sizeOfData >= tamanioLinea)
		return INSUFFICIENT_SPACE;
	memcpy(buffer+size, data, sizeOfData);
	buffer[tamanioLinea - 1] = '\n';
	return writeLine(buffer, lineNumber);
}

int sizeOfLine(char* line) {
	int len = 0;
	while (line[len] != '\n')
		len++;
	return len;
}

int FM9_AskForLine(int virtualAddress, int dtbID, void* buffer) {
	int lineNumber = memoryFunctions->virtualAddressTranslation(virtualAddress,
			dtbID);
	return readLine(buffer, lineNumber);
}

int FM9_Open(int dtbID, void* data, int size) {
	return memoryFunctions->writeData(data, size, dtbID);
}

int FM9_Close(int dtbID, int virtualAddress) {
	return memoryFunctions->closeFile(dtbID, virtualAddress);
}

void FM9_Dump(int argC, char** args, char* callingLine, void* extraData) {
	switch (argC) {
	case 0:
		Logger_Log(LOG_INFO, "Tiene que indicar el id de un DTB.");
		break;
	case 1:
		Logger_Log(LOG_INFO, "DTBDID %s.",args[1]);
		memoryFunctions->dump(atoi(args[1]));
		break;
	default:
		Logger_Log(LOG_INFO, "Ingresó parámetros demas, solo debe ingresar el id del DTB");
	}
}

//int FM9_Flush(int dtbID, int virtualAddress) {
//
//}
