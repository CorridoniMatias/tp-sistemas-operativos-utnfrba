#include "headers/FM9Interface.h"

int FM9_AsignLine(int virtualAddress, int dtbID, void* data) {
	int lineNumber = memoryFunctions->virtualAddressTranslation(virtualAddress, dtbID);
	char* buffer = malloc(tamanioLinea);
	int result = readLine(buffer, lineNumber);
	int size = sizeOfLine(buffer);
	int sizeOfData = string_length((char*) data);
	if (size + sizeOfData >= tamanioLinea)
		return INSUFFICIENT_SPACE;
	memcpy(buffer[size], data, sizeOfData);
	buffer[tamanioLinea-1] = '\n';
	return 1;
}

int sizeOfLine(char* line) {
	int len = 0;
	while (line[len] != '\n')
		len++;
	return len;
}
