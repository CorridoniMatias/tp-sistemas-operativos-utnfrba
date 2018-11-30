#include "headers/Tests.h"
void testSEG() {
	createSegmentationStructures();

	char * datos = "0123456789";
	char * buffer = calloc(1, 12);
	char * datos2 = "ABCDEFGHIJKLMNO";

	int resultado;
	int dir;
	dir = writeData_SEG(datos, 10, 1);
	resultado = readData_SEG(buffer, dir, 1);
	resultado = readData_SEG(buffer, dir + 1, 1);

	dir = writeData_SEG(datos2, 15, 1);
	resultado = readData_SEG(buffer, dir, 1);
	resultado = readData_SEG(buffer, dir + 1, 1);
	free(buffer);
	freeSegmentationStructures();
}

void testPAG() {
	createPagingStructures();
	createIPTStructures();

	char * datos = "0123456789";
	char * buffer = calloc(1, 12);
	char * datos2 = "ABCDEFGHIJKLMNO";

	Logger_Log(LOG_DEBUG, "FM9 -> Por usar write frame");
	int resultado;
	int dir;

	resultado = writeFrame(datos, 6);
	resultado = readFrame(buffer, 6);

	if (resultado == INVALID_LINE_NUMBER)
		Logger_Log(LOG_DEBUG, "FM9 -> Espacio Insuficiente.");
	else
		Logger_Log(LOG_DEBUG, "FM9 -> Guardado Correctamente");

	if (resultado == INVALID_FRAME_NUMBER)
		Logger_Log(LOG_DEBUG,
				"FM9 -> No existe el espacio de memoria a leer deseado.");
	else {
		Logger_Log(LOG_DEBUG, "FM9 -> Leido Correctamente");

		*(buffer + 4) = '\0';
		Logger_Log(LOG_DEBUG, "FM9 -> Lectura = %s", buffer);
	}

	int frame = getFreeFrame();
	if (frame != NO_FRAMES_AVAILABLE) {
		Logger_Log(LOG_DEBUG, "FM9 -> Frame %d libre.", frame);
		addFreeFrame(frame);
	} else
		Logger_Log(LOG_DEBUG, "FM9 -> No hay frames libres");
	free(buffer);
	freePagingStructures();
	freeIPTStructures();
}

void testAsignar() {

	createSegmentationStructures();

	char * datos = "01\n";
	char * buffer = calloc(1, 12);
	char * datos2 = "ABCD";

	int resultado;
	int dir;
	dir = writeData_SEG(datos, 10, 1);
	OnArrivedData* data= SocketServer_CreateOnArrivedData();

	FM9_AsignLine(data);
	readLine(buffer, dir);
	free(buffer);
	freeSegmentationStructures();
}

void testDump() {
	createSegmentationStructures();

	char * d = "01\n";
	char * d1 = "ABCD";
	char * d2 = "HOLA";
	char * d3 = "DATOS#";

	Logger_Log(LOG_DEBUG, "FM9 -> Escritura en  %d", writeData_SEG(d, 3, 1));
	Logger_Log(LOG_DEBUG, "FM9 -> Escritura en  %d", writeData_SEG(d1, 4, 1));
	Logger_Log(LOG_DEBUG, "FM9 -> Escritura en  %d", writeData_SEG(d2, 4, 1));
	Logger_Log(LOG_DEBUG, "FM9 -> Escritura en  %d", writeData_SEG(d3, 5, 1));
}

void testLineLength(){

	char* line = "01245\n0123456789\n";

	int size = sizeOfLine(line);
	char* firstPart = malloc(size + 1);
	memcpy(firstPart, line, size);
	firstPart[size] = '\0';
	printf("primera parte %s\n",firstPart);
	printf("tamaño primera parte %d\n",size);
	int size1 = sizeOfLine(line+size+1);
	char* secondPart = malloc(size1 + 1);
	memcpy(secondPart, line+size+1, size1);
	secondPart[size1] = '\0';

	printf("segunda parte %s\n",secondPart);
	printf("tamaño segunda parte %d\n",size1);
}
